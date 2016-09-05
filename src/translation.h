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

#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QObject>
#include <QDomDocument>

class Translation : public QObject
{
    Q_OBJECT
public:
    explicit Translation(const QString &lang, const QString &string = QString(), QObject *parent = nullptr);

    void setString(const QString &string);

    QString string() const;

    QDomDocument toXml() const;

    QDomDocument toXliff() const;

private:
    Q_DISABLE_COPY(Translation)

    QString m_string;
};

#endif // TRANSLATION_H
