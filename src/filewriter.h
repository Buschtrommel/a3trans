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

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QObject>
#include <QDir>

class Project;

class FileWriter : public QObject
{
    Q_OBJECT
public:
    explicit FileWriter(const QDir &workingDir, Project *project, QObject *parent = nullptr);


    void writeStringTable(bool backup = false);

    void writeXliff(const QStringList &trgLangs, const QString &srcLang = QStringLiteral("en"), bool version2 = false);

private:
    QDir m_wd;
    Project *m_prj;

    bool writeToFile(const QString &fullFilePath, const QString &filePath, const QByteArray &data) const;
};

#endif // FILEWRITER_H
