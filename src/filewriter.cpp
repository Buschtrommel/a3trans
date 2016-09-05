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

#include "filewriter.h"
#include "project.h"
#include <QFile>
#include <QDomDocument>
#include <QStringList>
#include <QDateTime>




/*!
 * \class FileWriter
 * \brief Provides functions to write the generated content to files.
 * \since 1.0.0
 */



/*!
 * \brief Constructs a new FileWrite object.
 * \since 1.0.0
 * \param workingDir    The working directory.
 * \param project       Pointer to the project data.
 * \param parent        Parent object.
 */
FileWriter::FileWriter(const QDir &workingDir, Project *project, QObject *parent) : QObject(parent), m_wd(workingDir), m_prj(project)
{

}


/*!
 * \brief Writes a new stringtable.xml file in the working directory.
 * \since 1.0.0
 * \param backup    Set to true if the current stringtable.xml file should be copied to backup file.
 */
void FileWriter::writeStringTable(bool backup)
{
    if (!m_prj) {
        qFatal("No valid project object.");
        return;
    }

    QFile stf(m_wd.absoluteFilePath(QStringLiteral("stringtable.xml")));

    if (stf.exists() && backup) {
        if (!stf.copy(m_wd.absoluteFilePath(QStringLiteral("stringtable_")).append(QString::number(QDateTime::currentDateTimeUtc().toTime_t())).append(QLatin1String(".xml.bak")))) {
            qCritical("%s", qUtf8Printable(tr("Failed to create stringtable.xml backup file.")));
            return;
        }
    }

    if (stf.exists()) {
        if (!stf.remove()) {
            qCritical("%s", qUtf8Printable(tr("Failed to remove current stringtable.xml file.")));
            return;
        }
    }

    writeToFile(stf.fileName(), QStringLiteral("stringtable.xml"), m_prj->toXml().toByteArray(8));

}




/*!
 * \brief Writes XLIFF CAT files to the l10n directory inside the current working directory.
 * \since 1.0.0
 * \param trgLangs  List of target languages. If empty, only a source translation file will be created.
 * \param srcLang   ISO 639-1 language code of the source language.
 * \param version2  Set this to true to write XLIFF 2.0 compatible files.
 */
void FileWriter::writeXliff(const QStringList &trgLangs, const QString &srcLang, bool version2)
{
    if (!m_prj) {
        qFatal("No valid project object.");
        return;
    }

    QDir l10nDir(m_wd);

    if (!l10nDir.exists(QStringLiteral("l10n"))) {
        l10nDir.mkdir(QStringLiteral("l10n"));
    }

    l10nDir.cd(QStringLiteral("l10n"));

    QString fullFilePath = l10nDir.absoluteFilePath(QStringLiteral("strings")).append(QLatin1String(".xlf"));
    QString filePath = fullFilePath;
    filePath.remove(m_wd.absolutePath());

    writeToFile(fullFilePath, filePath, m_prj->toXliff(QString(), srcLang, version2).toByteArray(8));

    if (!trgLangs.isEmpty()) {

        for (int i = 0; i < trgLangs.size(); ++i) {

            QString l = trgLangs.at(i).toLower();

            fullFilePath = l10nDir.absoluteFilePath(QStringLiteral("strings_")).append(l).append(QLatin1String(".xlf"));
            filePath = fullFilePath;
            filePath.remove(m_wd.absolutePath());

            writeToFile(fullFilePath, filePath, m_prj->toXliff(l, srcLang, version2).toByteArray(8));
        }

    }
}



/*!
 * \brief Writes data to a file.
 * \since 1.0.0
 * \param fullFilePath  Full path to the file.
 * \param filePath      Relative path of the file. Used to display status messages.
 * \param data          The data to write into the file.
 * \return              Returns true on success.
 */
bool FileWriter::writeToFile(const QString &fullFilePath, const QString &filePath, const QByteArray &data) const
{
    QFile f(fullFilePath);
    if (f.exists()) {
        if (!f.remove()) {
            qWarning("%s", qUtf8Printable(tr("Failed to remove file: %1").arg(filePath)));
            return false;
        }
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("%s", qUtf8Printable(tr("Failed to open file for writing: %1").arg(filePath)));
        return false;
    }

    if (f.write(data) < 0) {
        qWarning("%s", qUtf8Printable(tr("Failed to write data to file: %1").arg(filePath)));
        f.close();
        return false;
    }

    f.close();

    return true;
}
