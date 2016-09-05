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

#include "key.h"
#include "translation.h"
#include <QDomElement>
#ifdef QT_DEBUG
#include <QDebug>
#endif


/*!
 * \class Key
 * \brief Contains information about a key.
 *
 * The key class is an object representation of the stringtable XML key node.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */



/*!
 * \brief Constructs a new empty key, identified by id.
 * \since 1.0.0
 * \param id        The id of the key. Can be returned by objectName().
 * \param parent    The parent object.
 */
Key::Key(const QString &id, QObject *parent) : QObject(parent)
{
    setObjectName(id);

#ifdef QT_DEBUG
    qDebug() << "Creating new key with id" << id;
#endif
}






/*!
 * \brief Sets a new translation belonging to this key.
 *
 * If the child objects Translation are not available, they will be created.
 *
 * \since 1.0.0
 *
 * \param lang          The language of the translation.
 * \param translation   The translation string.
 */
void Key::setTranslation(const QString &lang, const QString &string)
{
    Translation *t = findChild<Translation *>(lang, Qt::FindDirectChildrenOnly);

    if (t && !string.isEmpty()) {
        t->setString(string);
    } else if (!t && !string.isEmpty()) {
        t = new Translation(lang, string, this);
    } else if (t && string.isEmpty()) {
        delete t;
    }
}






/*!
 * \brief Returns a pointer to the translation identified by language.
 * \since 1.0.0
 * \param lang  The language for the translation.
 * \return      Pointer to a Translation object.
 */
Translation *Key::getTranslation(const QString &lang) const
{
    return findChild<Translation *>(lang, Qt::FindDirectChildrenOnly);
}






/*!
 * \brief Returns a list of all translations associated with tis key.
 * \since 1.0.0
 * \return      List of pointers to Translations.
 */
QList<Translation *> Key::getAllTranslations() const
{
    return findChildren<Translation *>(QString(), Qt::FindDirectChildrenOnly);
}






/*!
 * \brief Converts this object into an XML entity.
 *
 * When converting to XML, all children will be converted to XML too and will be child nodes of this node.
 *
 * \since 1.0.0
 * \return XML document
 */
QDomDocument Key::toXml() const
{
    QList<Translation*> ts = getAllTranslations();

    QDomDocument xml;

    if (ts.isEmpty()) {
        return xml;
    }

    QDomElement e = xml.createElement(QStringLiteral("Key"));
    e.setAttribute(QStringLiteral("ID"), objectName());
    xml.appendChild(e);

    for (int i = 0; i < ts.size(); ++i) {
        QDomDocument t = ts.at(i)->toXml();
        if (t.hasChildNodes()) {
            e.appendChild(t);
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
 * \param version2      Set to true if the output should be XLIFF 2.0 compatible.
 * \return              XML document
 */
QDomDocument Key::toXliff(const QString &lang, bool version2) const
{
    QDomDocument xml;

    Translation *o = getTranslation(QStringLiteral("Original"));

    if (!o) {
        return xml;
    }

    QDomElement e;

    if (!version2) {
        e = xml.createElement(QStringLiteral("trans-unit"));
    } else {
        e = xml.createElement(QStringLiteral("segment"));
    }

    QString id = objectName().simplified();
    id.replace(QChar(' '), QLatin1String("_"));

    e.setAttribute(QStringLiteral("id"), id);
    xml.appendChild(e);

    e.appendChild(o->toXliff());

    if (!lang.isEmpty()) {
        Translation *t = getTranslation(lang);

        if (t) {
            e.appendChild(t->toXliff());
        }
    }

    return xml;
}
