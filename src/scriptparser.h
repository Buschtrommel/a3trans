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

#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include <QObject>
#include <QFile>

class Project;

class ScriptParser : public QObject
{
    Q_OBJECT
public:
    explicit ScriptParser(const QString &scriptFile, Project *stringTableProject, Project *scriptProject, QObject *parent = nullptr);

    void parse();

private:
    QFile m_file;
    Project *m_st;
    Project *m_sp;
    QString m_fileBaseName;

    void saveTranslation(const QString &package, const QString &container, const QString &key, const QString &text);
    void findLonelyTranslation(const QString &key, uint lineNumber);
};

#endif // SCRIPTPARSER_H
