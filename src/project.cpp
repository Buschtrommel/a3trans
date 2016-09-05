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

#include "project.h"
#include "package.h"
#include "translation.h"
#ifdef QT_DEBUG
#include <QDebug>
#endif



/*!
 * \class Project
 * \brief Contains information about a project.
 *
 * The project class is an object representation of the stringtable XML project node.
 * As the project node is the root node in the XML file, this object is the root object
 * for all other objects.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */




/*!
 * \brief Constructs a new empty project, identified by name.
 * \since 1.0.0
 * \param name      The name of the project. Can be returned by objectName().
 * \param parent    The parent object.
 */
Project::Project(const QString &name, QObject *parent) : QObject(parent)
{
    setObjectName(name);

#ifdef QT_DEBUG
    qDebug() << "Creating new project" << name;
#endif
}





/*!
 * \brief Sets a new translation belonging to this project.
 *
 * If the child objects Package, Container, Key and Translation are not available, they will be created.
 *
 * \since 1.0.0
 *
 * \param pacakge       The package this translation belongs to, identified by its name.
 * \param container     The container this translation belongs to, identified by its name.
 * \param key           The key the translation belongs to, identified by its id.
 * \param lang          The language of the translation.
 * \param translation   The translation string.
 */
void Project::setTranslation(const QString &package, const QString &container, const QString &key, const QString &lang, const QString &translation)
{
    Package *p = findChild<Package *>(package, Qt::FindDirectChildrenOnly);

    if (!p) {
        p = new Package(package, this);
    }

    p->setTranslation(container, key, lang, translation);
}





/*!
 * \brief Returns a pointer to the translation identified by package, container, key and language.
 * \since 1.0.0
 * \param package   The package this translation belongs to, idenfied by its name.
 * \param container The container this translation belongs to, identified by its name
 * \param key       The key the translation belongs to, identified by its id.
 * \param lang      The language for the translation.
 * \return          Pointer to a Translation object.
 */
Translation *Project::getTranslation(const QString &package, const QString &container, const QString &key, const QString &lang)
{
    Package *p = findChild<Package *>(package, Qt::FindDirectChildrenOnly);

    if (!p) {
        return nullptr;
    }

    return p->getTranslation(container, key, lang);
}





/*!
 * \brief Returns a list of all translations associated with the given package, container and key.
 * \since 1.0.0
 * \param package   The package the translations belong to, identified by its name.
 * \param container The container the translations belong to, identified by its name.
 * \param key       The key the translations belong to, identified by its id.
 * \return          List of pointers to Translations.
 */
QList<Translation *> Project::getAllTranslations(const QString &package, const QString &container, const QString &key) const
{
    Package *p = findChild<Package *>(package, Qt::FindDirectChildrenOnly);

    if (!p) {
        return QList<Translation *>();
    }

    return p->getAllTranslations(container, key);
}





/*!
 * \brief Converts this object into an XML entity.
 *
 * When converting to XML, all children will be converted to XML too and will be child nodes of this node.
 *
 * \since 1.0.0
 * \return XML document
 */
QDomDocument Project::toXml() const
{
    QDomDocument xml;

    QList<Package*> ps = findChildren<Package*>(QString(), Qt::FindDirectChildrenOnly);

    if (ps.isEmpty()) {
        return xml;
    }

    QDomProcessingInstruction pi = xml.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"utf-8\" "));
    xml.appendChild(pi);

    QDomElement e = xml.createElement(QStringLiteral("Project"));
    e.setAttribute(QStringLiteral("name"), objectName());
    xml.appendChild(e);

    for (int i = 0; i < ps.size(); ++i) {
        QDomDocument p = ps.at(i)->toXml();
        if (p.hasChildNodes()) {
            e.appendChild(p);
        }
    }

    return xml;
}





/*!
 * \brief Converts this object into an XLIFF compatible xml entity.
 *
 * When converting to XLIFF, all children will be converted to XLIFF too and will be child nodes of this node.
 *
 * \since 1.0.0
 *
 * \param lang          The target language of the XLIFF document.
 * \param srcLng        The source language of this project. Default: en
 * \param version2      Set to true if the output should be XLIFF 2.0 compatible. Default: false
 * \return              XML document
 */
QDomDocument Project::toXliff(const QString &lang, const QString &srcLng, bool version2) const
{
    QDomDocument xml;

    QList<Package*> ps = findChildren<Package*>(QString(), Qt::FindDirectChildrenOnly);

    if (ps.isEmpty()) {
        return xml;
    }

    QDomProcessingInstruction pi = xml.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"utf-8\" "));
    xml.appendChild(pi);

    QDomElement xliff;

    if (!version2) {
        xliff = xml.createElementNS(QStringLiteral("urn:oasis:names:tc:xliff:document:1.2"), QStringLiteral("xliff"));
        xliff.setAttribute(QStringLiteral("version"), QStringLiteral("1.2"));
    } else {
        xliff = xml.createElementNS(QStringLiteral("urn:oasis:names:tc:xliff:document:2.0"), QStringLiteral("xliff"));
        xliff.setAttribute(QStringLiteral("version"), QStringLiteral("2.0"));
        xliff.setAttribute(QStringLiteral("srcLang"), srcLng);
        xliff.setAttribute(QStringLiteral("trgLang"), lang);
    }

    xml.appendChild(xliff);

    QDomElement file = xml.createElement(QStringLiteral("file"));

    QString id = objectName().simplified();
    id.replace(QChar(' '), QLatin1String("_"));

    if (!version2) {
        file.setAttribute(QStringLiteral("original"), id);
        file.setAttribute(QStringLiteral("source-language"), srcLng);
        if (!lang.isEmpty()) {
            file.setAttribute(QStringLiteral("target-language"), lang);
        }
        file.setAttribute(QStringLiteral("datatype"), QStringLiteral("plaintext"));
    } else {
        file.setAttribute(QStringLiteral("id"), id);
    }

    xliff.appendChild(file);

    if (!version2) {

        QDomElement body = xml.createElement(QStringLiteral("body"));
        file.appendChild(body);

        for (int i = 0; i < ps.size(); ++i) {
            QDomDocument p = ps.at(i)->toXliff(langCodeToString(lang), version2);
            if (p.hasChildNodes()) {
                body.appendChild(p);
            }
        }

    } else {

        for (int i = 0; i < ps.size(); ++i) {
            QDomDocument p = ps.at(i)->toXliff(langCodeToString(lang), version2);
            if (p.hasChildNodes()) {
                file.appendChild(p);
            }
        }

    }

    return xml;
}


/*!
 * \brief Returns the language name used by ArmA that is associated to the ISO 639-1 code.
 * \param code  ISO 639-1 language code.
 * \return      ArmA language name.
 */
QString Project::langCodeToString(const QString &code) const
{
    QString c = code.toLower();
    if (c == QLatin1String("en")) {
        return QStringLiteral("English");
    } else if (c == QLatin1String("cz")) {
        return QStringLiteral("Czech");
    } else if (c == QLatin1String("fr")) {
        return QStringLiteral("French");
    } else if (c == QLatin1String("es")) {
        return QStringLiteral("Spanish");
    } else if (c == QLatin1String("it")) {
        return QStringLiteral("Italian");
    } else if (c == QLatin1String("pl")) {
        return QStringLiteral("Polish");
    } else if (c == QLatin1String("pt")) {
        return QStringLiteral("Portuguese");
    } else if (c == QLatin1String("ru")) {
        return QStringLiteral("Russian");
    } else if (c == QLatin1String("de")) {
        return QStringLiteral("German");
    } else if (c == QLatin1String("ko")) {
        return QStringLiteral("Korean");
    } else if (c == QLatin1String("ja")) {
        return QStringLiteral("Japanese");
    } else {
        return QString();
    }
}
