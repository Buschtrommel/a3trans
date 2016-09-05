/*
    a3trans - A translation string extractor and convertor for ArmA 3 script files.
    Copyright (C) 2016 Buschtrommel/Matthias Fehring (https://www.buschmann23.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stringtableparser.h"
#include "project.h"
#include "package.h"
#include "container.h"
#include "key.h"
#include "translation.h"
#include <QDomDocument>
#include <QDomNodeList>
#include <QDomElement>
#include <QTextStream>


/*!
 * \class StringtableParser
 * \brief Provides methods and functions to extract translation strings from a stringtable.xml file.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */


/*!
 * \brief Constructs a new parser object for the stringtable.xml file identified by \c stringTable.
 * \param stringTable   The full path to the stringtable.xml file.
 * \param parent        Pointer to the parent object.
 */
StringtableParser::StringtableParser(const QString &stringTable, QObject *parent) : QObject(parent)
{
    m_stringtable.setFileName(stringTable);
}


/*!
 * \brief Starts the parsing process and returns a pointer to a Project object.
 * \return Project object containing the extracted data.
 */
Project *StringtableParser::parse()
{
    if (!m_stringtable.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qCritical("%s", qUtf8Printable(tr("Failed to open file.")));
        return nullptr;
    }

    QDomDocument st(QStringLiteral("stringtable"));

    if (!st.setContent(&m_stringtable)) {
        qCritical("%s", qUtf8Printable(tr("Failed to parse XML data.")));
        m_stringtable.close();
        return nullptr;
    }
    
    m_stringtable.close();

    QDomElement project = st.documentElement();

    if (project.isNull() || !project.hasChildNodes() || project.tagName() != QLatin1String("Project")) {
        qCritical("%s", qUtf8Printable(tr("Failed to parse XML data.")));
        return nullptr;
    }

    QString projectName = project.attribute(QStringLiteral("name"), QStringLiteral("My Project"));

    Project *proj = new Project(projectName);


    QDomNodeList packages = project.elementsByTagName(QStringLiteral("Package"));

    if (packages.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("Can not find Package node elements.")));
        delete proj;
        return nullptr;
    }

    for (int i = 0; i < packages.size(); ++i) {
        QDomElement package = packages.at(i).toElement();
        if (!package.isNull() && package.hasChildNodes()) {

            Package *pac = new Package(package.attribute(QStringLiteral("name")), proj);

            QDomNodeList containers = package.elementsByTagName(QStringLiteral("Container"));

            if (containers.isEmpty()) {
                delete pac;
            } else {

                for (int j = 0; j < containers.size(); ++j) {
                    QDomElement container = containers.at(j).toElement();

                    if (!container.isNull() && container.hasChildNodes()) {

                        Container *cont = new Container(container.attribute(QStringLiteral("name")), pac);

                        QDomNodeList keys = container.elementsByTagName(QStringLiteral("Key"));

                        if (keys.isEmpty()) {
                            delete cont;
                        } else {

                            for (int k = 0; k < keys.size(); ++k) {
                                QDomElement currentKey = keys.at(k).toElement();
                                QString id = currentKey.attribute(QStringLiteral("ID"));
                                if (!id.isEmpty() && currentKey.hasChildNodes()) {

                                    Key *key = new Key(id, cont);

                                    QDomNodeList translations = currentKey.childNodes();

                                    if (translations.isEmpty()) {
                                        delete key;
                                    } else {

                                        for (int l = 0; l < translations.size(); ++l) {
                                            QDomElement translation = translations.at(l).toElement();

                                            Translation *trans = new Translation(translation.tagName(), translation.text(), key);
                                            Q_UNUSED(trans)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (proj->children().size() < 1) {
        qWarning("%s", qUtf8Printable(tr("Can not find any nodes.")));
        delete proj;
        return nullptr;
    }

    return proj;
}
