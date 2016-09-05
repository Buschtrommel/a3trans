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

#ifndef XLIFFPARDER_H
#define XLIFFPARDER_H

#include <QObject>
#include <QDir>

class Project;
class QDomElement;

class XliffParser : public QObject
{
    Q_OBJECT
public:
    explicit XliffParser(const QDir &workingDir, Project *prj, QObject *parent = nullptr);

    void parse();

private:
    Q_DISABLE_COPY(XliffParser)

    QDir m_wd;
    Project *m_prj;
    QStringList m_supportedLangs;

    void extract(const QString &filePath);
    void extractV1(const QDomElement &e);
    void extractV2(const QDomElement &e, const QString &trgLang, const QString &srcLang);
};

#endif // XLIFFPARDER_H
