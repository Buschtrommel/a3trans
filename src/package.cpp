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

#include "package.h"
#include "container.h"
#include "translation.h"
#ifdef QT_DEBUG
#include <QDebug>
#endif




/*!
 * \class Package
 * \brief Contains information about a package.
 *
 * The package class is an object representation of the stringtable XML package node.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */




/*!
 * \brief Constructs a new empty package, identified by name.
 * \since 1.0.0
 * \param name      The name of the package. Can be returned by objectName().
 * \param parent    The parent object.
 */
Package::Package(const QString &name, QObject *parent) : QObject(parent)
{
    setObjectName(name);

#ifdef QT_DEBUG
    qDebug() << "Creating new package" << name;
#endif
}






/*!
 * \brief Sets a new translation belonging to this package.
 *
 * If the child objects Container, Key and Translation are not available, they will be created.
 *
 * \since 1.0.0
 *
 * \param container     The container the translation belongs to, identified by its name.
 * \param key           The key the translation belongs to, identified by its id.
 * \param lang          The language of the translation.
 * \param translation   The translation string.
 */
void Package::setTranslation(const QString &container, const QString &key, const QString &lang, const QString &translation)
{
    Container *c = findChild<Container *>(container, Qt::FindDirectChildrenOnly);

    if (!c) {
        c = new Container(container, this);
    }

    c->setTranslation(key, lang, translation);
}






/*!
 * \brief Returns a pointer to the translation identified by container, key and language.
 * \since 1.0.0
 * \param container The container the translation belongs to, identified by its name.
 * \param key       The key the translation belongs to, identified by its id.
 * \param lang      The language for the translation.
 * \return          Pointer to a Translation object.
 */
Translation *Package::getTranslation(const QString &container, const QString &key, const QString &lang)
{
    Container *c = findChild<Container *>(container, Qt::FindDirectChildrenOnly);

    if (!c) {
        return nullptr;
    }

    return c->getTranslation(key, lang);
}




/*!
 * \brief Returns a list of all translations associated with the given container and key.
 * \since 1.0.0
 * \param container The container the translations belong to, identified by its name.
 * \param key       The key the translations belong to, identified by its id.
 * \return          List of pointers to Translations.
 */
QList<Translation *> Package::getAllTranslations(const QString &container, const QString &key) const
{
    Container *c = findChild<Container *>(container, Qt::FindDirectChildrenOnly);

    if (!c) {
        return QList<Translation *>();
    }

    return c->getAllTranslations(key);
}




/*!
 * \brief Converts this object into an XML entity.
 *
 * When converting to XML, all children will be converted to XML too and will be child nodes of this node.
 *
 * \since 1.0.0
 * \return XML document
 */
QDomDocument Package::toXml() const
{
    QDomDocument xml;

    QList<Container*> cs = findChildren<Container*>(QString(), Qt::FindDirectChildrenOnly);

    if (cs.isEmpty()) {
        return xml;
    }

    QDomElement e = xml.createElement(QStringLiteral("Package"));
    e.setAttribute(QStringLiteral("name"), objectName());
    xml.appendChild(e);

    for (int i = 0; i < cs.size(); ++i) {
        QDomDocument c = cs.at(i)->toXml();
        if (c.hasChildNodes()) {
            e.appendChild(c);
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
QDomDocument Package::toXliff(const QString &lang, bool version2) const
{
    QDomDocument xml;

    QList<Container*> cs = findChildren<Container*>(QString(), Qt::FindDirectChildrenOnly);

    if (cs.isEmpty()) {
        return xml;
    }

    QDomElement e = xml.createElement(QStringLiteral("group"));

    QString id = objectName().simplified();
    id.replace(QChar(' '), QLatin1String("_"));

    e.setAttribute(QStringLiteral("id"), id);
    xml.appendChild(e);

    for (int i = 0; i < cs.size(); ++i) {
        QDomDocument c = cs.at(i)->toXliff(lang, version2);
        if (c.hasChildNodes()) {
            e.appendChild(c);
        }
    }

    return xml;
}
