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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QFile>

#include "scriptparser.h"
#include "stringtableparser.h"
#include "project.h"
#include "filewriter.h"
#include "xliffparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("a3trans"));
    a.setOrganizationDomain(QStringLiteral("buschmann23.de"));
    a.setOrganizationName(QStringLiteral("Buschtrommel"));
    a.setApplicationVersion(QStringLiteral("0.0.1"));

    QString desc(QCoreApplication::translate("main", "a3trans is a translation string extractor for ArmA 3 script files."));
    desc.append(QLatin1String("\n"));
    desc.append(QLatin1String("\nCopyright (c) 2016 Buschtrommel/Matthias Fehring (https://www.buschmann23.de)\n"));
    desc.append(QCoreApplication::translate("main","This program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\nGNU General Public License for more details."));
    desc.append(QLatin1String("\nhttp://www.gnu.org/licenses/gpl-3.0.html"));

    QString dirPath = QDir::currentPath();
    QString projectName;
    QString srcLng = QStringLiteral("en");
    QStringList supportedLangs({QStringLiteral("en"), QStringLiteral("cz"), QStringLiteral("fr"), QStringLiteral("es"), QStringLiteral("it"), QStringLiteral("pl"), QStringLiteral("pt"), QStringLiteral("ru"), QStringLiteral("de"), QStringLiteral("ko"), QStringLiteral("ja")});
    int convertToXliff = 0;
    bool createBackup = false;
    bool sourceLangOnly = false;
    bool x2s = false;
    bool extract = false;

    QCommandLineParser clparser;
    clparser.setApplicationDescription(desc);
    clparser.addHelpOption();
    clparser.addVersionOption();

    QCommandLineOption extractOption(QStringList() << QStringLiteral("e") << QStringLiteral("extract"), QCoreApplication::translate("main", "Start extracting translation strings from script files found in the workin directory an the subdirectories."));
    clparser.addOption(extractOption);

    QCommandLineOption directoryOption(QStringList() << QStringLiteral("d") << QStringLiteral("directory"), QCoreApplication::translate("main", "Sets the working directory. If omitted, the current directoy will be used."));
    clparser.addOption(directoryOption);

    QCommandLineOption srcLngOption(QStringList() << QStringLiteral("s") << QStringLiteral("sourceLang"), QCoreApplication::translate("main", "Sets the source language by ISO 639-1 code. Default: en. Supported languages: English (en), Czech (cz), French (fr), Spanish (es), Italian (it), Polish (pl), Portuguese (pt), Russian (ru), German (de), Korean (ko), Japanese (ja)"));
    clparser.addOption(srcLngOption);

    QCommandLineOption backupOption(QStringList() << QStringLiteral("b") << QStringLiteral("backup"), QCoreApplication::translate("main", "Create a backup of the stringtable.xml file before writing the new file."));
    clparser.addOption(backupOption);

    QCommandLineOption xliffOption(QStringList() << QStringLiteral("1") << QStringLiteral("xliff"), QCoreApplication::translate("main", "Converts the stringtable.xml file in the working directory into XLIFF 1.2 files for each language. The XLIFF 1.2 files will be stored in a subdirectory called l10n that will be created in the working directory if it not exists already."));
    clparser.addOption(xliffOption);

    QCommandLineOption xliff2Option(QStringList() << QStringLiteral("2") << QStringLiteral("xliff2"), QCoreApplication::translate("main", "Converts the stringtable.xml file in the working directory into XLIFF 2.0 files for each language. The XLIFF 2.0 files will be stored in a subdirectory called l10n that will be created in the working directory if it not exists already."));
    clparser.addOption(xliff2Option);

    QCommandLineOption srcLngOnlyOption(QStringList() << QStringLiteral("sourceLangOnly"), QCoreApplication::translate("main", "When converting to XLIFF, only a language indipendent source file will be created."));
    clparser.addOption(srcLngOnlyOption);

    QCommandLineOption xliff2xmlOption(QStringList() << QStringLiteral("x2s") << QStringLiteral("xliff2stringtable"), QCoreApplication::translate("main", "Converts language specific XLIFF files into a single stringtable.xml file. Expects the XLIFF file to be in a l10n subdirectory of the working directory."));
    clparser.addOption(xliff2xmlOption);

    clparser.process(a);

    if (argc > 1) {

        if (clparser.isSet(directoryOption)) {
            dirPath = clparser.value(directoryOption);
        }

        if (clparser.isSet(srcLngOption)) {
            QString lng = clparser.value(srcLngOption);
            if (supportedLangs.contains(lng)) {
                srcLng = lng;
            } else {
                qDebug("%s",qUtf8Printable(QCoreApplication::translate("main", "The language code %1 is not supported. Using default language English.").arg(lng)));
            }
        }

        if (clparser.isSet(xliffOption)) {
            convertToXliff = 1;
        } else if (clparser.isSet(xliff2Option)) {
            convertToXliff = 2;
        }

        createBackup = clparser.isSet(backupOption);

        sourceLangOnly = clparser.isSet(srcLngOnlyOption);

        x2s = clparser.isSet(xliff2xmlOption);

        extract = clparser.isSet(extractOption);

    } else {

        clparser.showHelp();

    }

    QDir dir(dirPath);

    qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Working directory: %1").arg(dirPath)));

    Project *stringTableProject = nullptr;

    if (!x2s) {

        qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Start parsing stringtable.xml file.")));

        StringtableParser stp(dir.absoluteFilePath(QStringLiteral("stringtable.xml")));
        stringTableProject = stp.parse();

    }

    if (convertToXliff > 0) {

        qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Start converting stringtable.xml into XLIFF files.")));

        FileWriter fw(dir, stringTableProject);

        if (sourceLangOnly) {
            fw.writeXliff(QStringList(), srcLng, convertToXliff > 1);
        } else {
            fw.writeXliff(supportedLangs, srcLng, convertToXliff > 1);
        }

    } else if (x2s || extract) {

        if (projectName.isEmpty() && stringTableProject) {
            projectName = stringTableProject->objectName();
        }

        if (projectName.isEmpty()) {
            projectName = QStringLiteral("My Project");
        }

        Project *currentProject = new Project(projectName);

        if (x2s) {

            qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Start converting XLIFF files into stringtable.xml.")));

            XliffParser xp(dir, currentProject);
            xp.parse();


        } else if (extract) {

            qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Start parsing script files.")));


            QDirIterator it(dirPath, QStringList() << QStringLiteral("*.sqf") << QStringLiteral("*.SQF") << QStringLiteral("description.ext") << QStringLiteral("mission.sqm"), QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString fp = it.next();
                QString fn = fp;
                fn.remove(dirPath);
                fn.remove(0, 1);
                qInfo("%s", qUtf8Printable(QCoreApplication::translate("main", "Parsing file: %1").arg(fn)));
                ScriptParser sp(fp, stringTableProject, currentProject);
                sp.parse();
            }
        }

        FileWriter fw(dir, currentProject);
        fw.writeStringTable(createBackup);
    }

    return 0;
}
