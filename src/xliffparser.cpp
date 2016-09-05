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

#include "xliffparser.h"
#include "project.h"
#include "package.h"
#include "container.h"
#include "key.h"
#include "translation.h"
#include <QDomDocument>



/*!
 * \class XliffParser
 * \brief Provides methods and functions to extract translation strings from XLIFF files.
 *
 * Results are saved into the Project object pointed to when constructing a new XliffParser object.
 * Currently XLIFF 1.0, 1.1, 1.2 and 2.0 are supported.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */




/*!
 * \brief Constructs a new XliffParser object.
 *
 * The XLIFF files are expected in a subdirectory of workingDir name l10n.
 *
 * \since 1.0.0
 * \param workingDir    The current working directory.
 * \param prj           Pointer to a Project object to store the extracted strings in.
 * \param parent        Pointer to a parent object.
 */
XliffParser::XliffParser(const QDir &workingDir, Project *prj, QObject *parent) : QObject(parent), m_wd(workingDir), m_prj(prj)
{
    m_supportedLangs = QStringList({QStringLiteral("en"), QStringLiteral("cz"), QStringLiteral("fr"), QStringLiteral("es"), QStringLiteral("it"), QStringLiteral("pl"), QStringLiteral("pt"), QStringLiteral("ru"), QStringLiteral("de"), QStringLiteral("ko"), QStringLiteral("ja")});
}




/*!
 * \brief Starts the parsing
 * \since 1.0.0
 */
void XliffParser::parse()
{
    if (!m_prj) {
        qFatal("We have no valid Project object!");
        return;
    }

    QDir l10nDir(m_wd);

    if (!l10nDir.exists(QStringLiteral("l10n"))) {
        qCritical("%s", qUtf8Printable(tr("Can not find the l10n directory inside the working directory.")));
        return;
    }

    l10nDir.cd(QStringLiteral("l10n"));

    for (int i = 0; i < m_supportedLangs.size(); ++i) {
        QString fn = QStringLiteral("strings_");
        fn.append(m_supportedLangs.at(i));
        fn.append(QLatin1String(".xlf"));

        if (l10nDir.exists(fn)) {
            qInfo("%s", qUtf8Printable(tr("Parsing file: %1").arg(fn)));
            extract(l10nDir.absoluteFilePath(fn));
        }
    }
}



/*!
 * \brief Extracts the translation strings from a single XLIFF file.
 * \since 1.0.0
 * \param filePath  Full path to the XLIFF file.
 */
void XliffParser::extract(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    QFile f(filePath);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("%s", qUtf8Printable(tr("Failed to open file. Aborting.")));
        return;
    }

    QDomDocument xliff(QStringLiteral("xliffdata"));

    if (!xliff.setContent(&f)) {
        qWarning("%s", qUtf8Printable(tr("Failed to parse XLIFF XML data. Aborting.")));
        f.close();
        return;
    }

    f.close();

    QDomElement x = xliff.documentElement();

    if (x.isNull() || !x.hasChildNodes() || x.tagName() != QLatin1String("xliff")) {
        qWarning("%s", qUtf8Printable(tr("Can not find XLIFF root node. Aborting.")));
        return;
    }

    QString version = x.attribute(QStringLiteral("version"));

    if (version.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("Can not determine XLIFF version. Aborting.")));
        return;
    }

    if (version.startsWith(QChar('1')) || version.startsWith(QChar('2'))) {

        qInfo("%s", qUtf8Printable(tr("Detected XLIFF version: %1").arg(version)));

        if (version.startsWith(QChar('2'))) {
            extractV2(x, x.attribute(QStringLiteral("trgLang")), x.attribute(QStringLiteral("srcLang")));
        } else {
            extractV1(x);
        }

    } else {
        qWarning("%s", qUtf8Printable(tr("XLIFF version %1 is not supported. Aborting.").arg(version)));
    }
}




/*!
 * \brief Extracts translation strings from XLIFF 1.x files.
 * \since 1.0.0
 * \param e XML root element (xliff element).
 */
void XliffParser::extractV1(const QDomElement &e)
{
    QDomElement file = e.firstChildElement(QStringLiteral("file"));

    QString trgLang = file.attribute(QStringLiteral("target-language"));
    if (trgLang.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No target language set. Aborting.")));
        return;
    }

    if (!m_supportedLangs.contains(trgLang, Qt::CaseInsensitive)) {
        qWarning("%s", qUtf8Printable(tr("Language code %1 is not supported. Aborting.").arg(trgLang)));
        return;
    }

    QString projectName = file.attribute(QStringLiteral("original"));
    if (projectName.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No project name / original set. Using default: %1.").arg(QStringLiteral("My Project"))));
        projectName = QStringLiteral("My Project");
    } else {
        projectName.replace(QChar('_'), QLatin1String(" "));
    }

    QString srcLang = file.attribute(QStringLiteral("source-language"));
    if (srcLang.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No source language set. Using default: English.")));
        srcLang = QStringLiteral("en");
    }

    QDomElement package = file.firstChildElement(QStringLiteral("body")).firstChildElement(QStringLiteral("group"));

    if (package.isNull()) {
        qWarning("%s", qUtf8Printable(tr("Can not find any package group elements. Aborting.")));
        return;
    }

    m_prj->setObjectName(projectName);


    while (!package.isNull()) {
        QString packageName = package.attribute(QStringLiteral("id"));
        packageName.replace(QChar('_'), QLatin1String(" "));

        QDomNodeList containers = package.elementsByTagName(QStringLiteral("group"));

        if (!containers.isEmpty()) {
            for (int i = 0; i < containers.size(); ++i) {
                QDomElement container = containers.at(i).toElement();
                QString containerName = container.attribute(QStringLiteral("id"));
                containerName.replace(QChar('_'), QLatin1String(" "));

                QDomNodeList keys = container.elementsByTagName(QStringLiteral("trans-unit"));

                if (!keys.isEmpty()) {
                    for (int j = 0; j < keys.size(); ++j) {
                        QDomElement key = keys.at(j).toElement();
                        QString keyId = key.attribute(QStringLiteral("id"));

                        QString source = key.firstChildElement(QStringLiteral("source")).text();
                        QString target = key.firstChildElement(QStringLiteral("target")).text();

                        m_prj->setTranslation(packageName, containerName, keyId, QStringLiteral("Original"), source);
                        m_prj->setTranslation(packageName, containerName, keyId, m_prj->langCodeToString(trgLang), target);
                    }
                }
            }
        }

        package = package.nextSiblingElement(QStringLiteral("group"));
    }
}


/*!
 * \brief Extracts translation strings from XLIFF 2.0 files.
 * \since 1.0.0
 * \param e         XML root element (xliff element).
 * \param trgLang   Target language tag (ISO 639-1).
 * \param srcLang   Source language tag (ISO 639-1).
 */
void XliffParser::extractV2(const QDomElement &e, const QString &trgLang, const QString &srcLang)
{
    if (trgLang.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No target language set. Aborting.")));
        return;
    }

    if (!m_supportedLangs.contains(trgLang, Qt::CaseInsensitive)) {
        qWarning("%s", qUtf8Printable(tr("Language code %1 is not supported. Aborting.").arg(trgLang)));
        return;
    }

    QString _srcLang = srcLang;
    if (_srcLang.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No source language set. Using default: English.")));
        _srcLang = QStringLiteral("en");
    }

    QDomElement file = e.firstChildElement(QStringLiteral("file"));

    QString projectName = file.attribute(QStringLiteral("id"));

    if (projectName.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("No project name / original set. Using default: %1.").arg(QStringLiteral("My Project"))));
        projectName = QStringLiteral("My Project");
    } else {
        projectName.replace(QChar('_'), QLatin1String(" "));
    }


    QDomNodeList packages = file.elementsByTagName(QStringLiteral("group"));

    if (packages.isEmpty()) {
        qWarning("%s", qUtf8Printable(tr("Can not find any package group elements. Aborting.")));
        return;
    }

    m_prj->setObjectName(projectName);

    for (int i = 0; i < packages.size(); ++i) {
        QDomElement package = packages.at(i).toElement();
        QString packageName = package.attribute(QStringLiteral("id"));
        packageName.replace(QChar('_'), QLatin1String(" "));


        QDomNodeList containers = package.elementsByTagName(QStringLiteral("unit"));

        if (!containers.isEmpty()) {
            for (int j = 0; j < containers.size(); ++j) {
                QDomElement container = containers.at(j).toElement();
                QString containerName = container.attribute(QStringLiteral("id"));
                containerName.replace(QChar('_'), QLatin1String(" "));

                QDomNodeList keys = container.elementsByTagName(QStringLiteral("segment"));

                if (!keys.isEmpty()) {
                    for (int k = 0; k < keys.size(); ++k) {
                        QDomElement key = keys.at(k).toElement();
                        QString keyId = key.attribute(QStringLiteral("id"));

                        QString source = key.firstChildElement(QStringLiteral("source")).text();
                        QString target = key.firstChildElement(QStringLiteral("target")).text();

                        m_prj->setTranslation(packageName, containerName, keyId, QStringLiteral("Original"), source);
                        m_prj->setTranslation(packageName, containerName, keyId, m_prj->langCodeToString(trgLang), target);
                    }
                }
            }
        }
    }
}
