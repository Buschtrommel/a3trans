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

#include "scriptparser.h"
#include "project.h"
#include "package.h"
#include "container.h"
#include "key.h"
#include "translation.h"
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QFileInfo>



/*!
 * \class ScriptParser
 * \brief Provides methods and functions to extract translation strings from scripts.
 *
 * This class can extract translation strings from SQF, EXT and SQM files.
 *
 * \since 1.0.0
 * \version 1.0.0
 * \date 2016-09-05
 * \author Buschmann
 * \copyright GNU GENERAL PUBLIC LICENSE Version 3
 */



/*!
 * \brief Constructs a new ScriptParser object.
 * \since 1.0.0
 * \param scriptFile            Full path to the file to parse.
 * \param stringTableProject    Pointer to a Project object that contains the data of the current stringtable.xml file.
 * \param scriptProject         Pointer to a Project object that will contain the extracted data.
 * \param parent                Pointer to the parent object.
 */
ScriptParser::ScriptParser(const QString &scriptFile, Project *stringTableProject, Project *scriptProject, QObject *parent) : QObject(parent), m_st(stringTableProject), m_sp(scriptProject)
{
    m_file.setFileName(scriptFile);
    m_fileBaseName = QFileInfo(m_file).baseName();
}


/*!
 * \brief Starts the parsing process.
 *
 * The extracted data will be part of the Project object that has been set via the scriptProject parameter
 * when creating the object.
 *
 * \since 1.0.0
 */
void ScriptParser::parse()
{
    if (!m_sp) {
        qFatal("We have no valid Project object.");
        return;
    }

    if (!m_file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qCritical("%s", qUtf8Printable(tr("Can not open script file.")));
        return;
    }

    QTextStream in(&m_file);
    in.setCodec("UTF-8");

    QRegularExpression singleLine(QStringLiteral("//\\s*TR\\s+([a-zA-z0-9_\\*]+)\\s+([a-zA-z0-9_\\*]+)\\s+([a-zA-z0-9_]+)\\s+\"([^\"]*)\""));

    QRegularExpression multiLineStart(QStringLiteral("/\\*\\s*TR"));
    QRegularExpression multiLineEnd(QStringLiteral("\\s*\\*/"));
    QRegularExpression multiLineMeta(QStringLiteral("/\\*\\s*TR\\s+([a-zA-z0-9_\\*]+)\\s+([a-zA-z0-9_\\*]+)\\s+([a-zA-z0-9_\\*]+)"));
    bool multiLineStarted = false;

    QString multiLineHeader;    // stores the start line of a multiline translation comment
    QString multiLine;          // stores the content string of a multiline translation comment

    // start searching for translation comments
    while (!in.atEnd()) {
        QString line = in.readLine();

        QRegularExpressionMatch singleLineMatch = singleLine.match(line);

        if (singleLineMatch.hasMatch() && !multiLineStarted) {

            QString package = singleLineMatch.captured(1);
            if (package == QLatin1String("*")) {
                package = QStringLiteral("Main");
            } else {
                package.replace(QChar('_'), QLatin1String(" "));
            }

            QString container = singleLineMatch.captured(2);
            if (container == QLatin1String("*")) {
                container = m_fileBaseName;
            } else {
                container.replace(QChar('_'), QLatin1String(" "));
            }

            QString key = singleLineMatch.captured(3);
            QString text = singleLineMatch.captured(4);

            saveTranslation(package, container, key, text);

        } else {


            if (line.contains(multiLineStart)) {
                multiLineStarted = true;
            }

            if (multiLineStarted) {
                if (multiLineHeader.isEmpty()) {
                    multiLineHeader = line;
                } else {
                    multiLine.append(line).append(QLatin1String(" "));
                }
            }

            if (multiLineStarted && line.contains(multiLineEnd)) {
                multiLineStarted = false;
                multiLine.replace(QLatin1String("<"), QLatin1String("&lt;"));
                multiLine.replace(QLatin1String(">"), QLatin1String("&gt;"));
                multiLine.remove(multiLineEnd);
                multiLine = multiLine.simplified();
                multiLineHeader = multiLineHeader.simplified();
                QRegularExpressionMatch match = multiLineMeta.match(multiLineHeader);

                if (match.hasMatch()) {
                    QString package = match.captured(1);
                    if (package == QLatin1String("*")) {
                        package = QStringLiteral("Main");
                    } else {
                        package.replace(QChar('_'), QLatin1String(" "));
                    }

                    QString container = match.captured(2);
                    if (container == QLatin1String("*")) {
                        container = m_fileBaseName;
                    } else {
                        container.replace(QChar('_'), QLatin1String(" "));
                    }

                    QString key = match.captured(3);

                    saveTranslation(package, container, key, multiLine);
                }

                multiLine.clear();
                multiLineHeader.clear();
            }
        }
    }

    if (!in.seek(0)) {
        qDebug("%s", qUtf8Printable(tr("Failed to reset position in file to the start.")));
        return;
    }

    QRegularExpression locString(QStringLiteral("(str_[a-zA-Z0-9_]+)"), QRegularExpression::CaseInsensitiveOption);

    // start searching for localization strings without localization
    uint lineNumber = 0;
    while(!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();

        QRegularExpressionMatchIterator i = locString.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            findLonelyTranslation(match.captured(1), lineNumber);
        }
    }
}




/*!
 * \brief Saves extracted translation strings.
 *
 * The extracted strings will be saved to the Project object pointed to by the scriptProject parameter
 * that has been set when creating this object.
 *
 * \since 1.0.0
 * \param package
 * \param container
 * \param key
 * \param text
 */
void ScriptParser::saveTranslation(const QString &package, const QString &container, const QString &key, const QString &text)
{
    m_sp->setTranslation(package, container, key, QStringLiteral("Original"), text);

    Translation *savedOriginalTranslation = m_st->getTranslation(package, container, key, QStringLiteral("Original"));

    if (savedOriginalTranslation) {

        if (QString::compare(savedOriginalTranslation->string(), text, Qt::CaseInsensitive) == 0) {

            QList<Translation *> tl = m_st->getAllTranslations(package, container, key);

            if (!tl.isEmpty()) {

                Key *k = m_sp->findChild<Package *>(package, Qt::FindDirectChildrenOnly)->findChild<Container *>(container, Qt::FindDirectChildrenOnly)->findChild<Key *>(key, Qt::FindDirectChildrenOnly);

                for (int i = 0; i < tl.size(); ++i) {
                    Translation *t = tl.at(i);
                    if (t->objectName() != QLatin1String("Original")) {

                        Translation *trans = new Translation(t->objectName(), t->string(), k);
                        Q_UNUSED(trans)

                    }
                }
            }
        }
    }
}


/*!
 * \brief Searches for translations in the stringtable.xml file and outputs strings without translation.
 * \since 1.0.0
 * \param key           ID of the translation key.
 * \param lineNumber    Current line number in the parsed file, for status messages.
 */
void ScriptParser::findLonelyTranslation(const QString &key, uint lineNumber)
{
    // try to find the key in the current project object hierarchy
    QList<Key*> spKeys = m_sp->findChildren<Key*>(QRegularExpression(key, QRegularExpression::CaseInsensitiveOption), Qt::FindChildrenRecursively);

    if (spKeys.isEmpty()) {

        // try to find the key in the stringtable object hierarchy
        QList<Key*> stKeys = m_st->findChildren<Key*>(QRegularExpression(key, QRegularExpression::CaseInsensitiveOption), Qt::FindChildrenRecursively);

        if (!stKeys.isEmpty()) {
            Key *k = stKeys.first();
            QList<Translation*> translations = k->findChildren<Translation*>(QString(), Qt::FindDirectChildrenOnly);
            Container *c = qobject_cast<Container*>(k->parent());
            Package *p = qobject_cast<Package*>(c->parent());

            if (!translations.isEmpty() && c && p) {
                for (int i = 0; i < translations.size(); ++i) {
                    m_sp->setTranslation(p->objectName(), c->objectName(), k->objectName(), translations.at(i)->objectName(), translations.at(i)->string());
                }
            }
        } else {
            qDebug("%s", qUtf8Printable(tr("ID without localization at line %1: %2").arg(QString::number(lineNumber), key)));
        }
    }
}
