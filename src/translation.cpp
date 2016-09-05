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

#include "translation.h"
#include <QDomElement>
#ifdef QT_DEBUG
#include <QDebug>
#endif



/*!
 * \class Translation
 * \brief Contains information about a translation.
 *
 * The translation class is an object representation of the stringtable XML language node.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */



/*!
 * \brief Constructs a new translation, identified by language
 * \since 1.0.0
 * \param lang      The language of this translation. Can be returned by objectName().
 * \param string    The translated string.
 * \param parent    The parent object.
 */
Translation::Translation(const QString &lang, const QString &string, QObject *parent) : QObject(parent), m_string(string)
{
    setObjectName(lang);

#ifdef QT_DEBUG
    qDebug() << "Creating new translation for language " << lang << ":" << string;
#endif
}






/*!
 * \brief Sets the string for this translation.
 * \param string    Translation string.
 */
void Translation::setString(const QString &string)
{
    m_string = string;
}





/*!
 * \brief Returns the string of this translation.
 * \return  Translation string.
 */
QString Translation::string() const
{
    return m_string;
}




/*!
 * \brief Converts this object into an XML entity.
 *
 * \since 1.0.0
 * \return XML document
 */
QDomDocument Translation::toXml() const
{
    QDomDocument xml;

    QDomElement e = xml.createElement(objectName());

    xml.appendChild(e);
    e.appendChild(xml.createTextNode(string()));

    return xml;
}


/*!
 * \brief Converts this object into an XLIFF compatible xml entity.
 *
 * \since 1.0.0
 * \return              XML document
 */
QDomDocument Translation::toXliff() const
{
    QDomDocument xml;

    QDomElement e;

    if (objectName() == QLatin1String("Original")) {
        e = xml.createElement(QStringLiteral("source"));
    } else {
        e = xml.createElement(QStringLiteral("target"));
    }

    xml.appendChild(e);
    e.appendChild(xml.createTextNode(string()));

    return xml;
}
