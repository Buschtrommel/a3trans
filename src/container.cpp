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

#include "container.h"
#include "translation.h"
#include "key.h"
#include <QRegularExpression>
#ifdef QT_DEBUG
#include <QDebug>
#endif


/*!
 * \class Container
 * \brief Contains information about a container.
 *
 * The container class is an object representation of the stringtable XML container node.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */


/*!
 * \brief Constructs a new empty container, identified by name.
 * \since 1.0.0
 * \param name      The name of the container. Can be returned by objectName().
 * \param parent    The parent object.
 */
Container::Container(const QString &name, QObject *parent) : QObject(parent)
{
    setObjectName(name);

#ifdef QT_DEBUG
    qDebug() << "Creating new container" << name;
#endif
}



/*!
 * \brief Sets a new translation belonging to this container.
 *
 * If the child objects Key and Translation are not available, they will be created.
 *
 * \since 1.0.0
 *
 * \param key           The key the translation belongs to, identified by its id.
 * \param lang          The language of the translation.
 * \param translation   The translation string.
 */
void Container::setTranslation(const QString &key, const QString &lang, const QString &translation)
{
    QList<Key*> ks = findChildren<Key *>(QRegularExpression(key, QRegularExpression::CaseInsensitiveOption), Qt::FindDirectChildrenOnly);

    Key *k = nullptr;

    if (ks.isEmpty()) {
        k = new Key(key, this);
    } else {
        k = ks.first();
    }

    if (k) {
        k->setTranslation(lang, translation);
    }
}


/*!
 * \brief Returns a pointer to the translation identified by key and language.
 * \since 1.0.0
 * \param key   The key the translation belongs to, identified by its id.
 * \param lang  The language for the translation.
 * \return      Pointer to a Translation object.
 */
Translation *Container::getTranslation(const QString &key, const QString &lang)
{
    QList<Key*> ks = findChildren<Key *>(QRegularExpression(key, QRegularExpression::CaseInsensitiveOption), Qt::FindDirectChildrenOnly);

    if (ks.isEmpty()) {
        return nullptr;
    }

    return ks.first()->getTranslation(lang);
}



/*!
 * \brief Returns a list of all translations associated with the given key.
 * \since 1.0.0
 * \param key   The key the translations belong to.
 * \return      List of pointers to Translations.
 */
QList<Translation *> Container::getAllTranslations(const QString &key) const
{
    QList<Key*> ks = findChildren<Key *>(QRegularExpression(key, QRegularExpression::CaseInsensitiveOption), Qt::FindDirectChildrenOnly);

    if (ks.isEmpty()) {
        return QList<Translation*>();
    }

    return ks.first()->getAllTranslations();
}


/*!
 * \brief Converts this object into an XML entity.
 *
 * When converting to XML, all children will be converted to XML too and will be child nodes of this node.
 *
 * \since 1.0.0
 * \return XML document
 */
QDomDocument Container::toXml() const
{
    QList<Key *> ks = findChildren<Key *>(QString(), Qt::FindDirectChildrenOnly);

    QDomDocument xml;

    if (ks.isEmpty()) {
        return xml;
    }

    QDomElement e = xml.createElement(QStringLiteral("Container"));
    e.setAttribute(QStringLiteral("name"), objectName());
    xml.appendChild(e);

    for (int i = 0; i < ks.size(); ++i) {
        QDomDocument k = ks.at(i)->toXml();
        if (k.hasChildNodes()) {
            e.appendChild(k);
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
QDomDocument Container::toXliff(const QString &lang, bool version2) const
{
    QList<Key *> ks = findChildren<Key*>(QString(), Qt::FindDirectChildrenOnly);

    QDomDocument xml;

    if (ks.isEmpty()) {
        return xml;
    }

    QDomElement e;

    if (!version2) {
        e = xml.createElement(QStringLiteral("group"));
    } else {
        e = xml.createElement(QStringLiteral("unit"));
    }


    QString id = objectName().simplified();
    id.replace(QChar(' '), QLatin1String("_"));

    e.setAttribute(QStringLiteral("id"), id);
    xml.appendChild(e);

    for (int i = 0; i < ks.size(); ++i) {
        QDomDocument k = ks.at(i)->toXliff(lang, version2);
        if (k.hasChildNodes()) {
            e.appendChild(k);
        }
    }

    return xml;
}
