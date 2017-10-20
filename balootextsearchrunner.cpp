/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// Modified by Juca Crispim to include search for source code
// indexed by baloo. In case of problems with that
// modifications, contact juca@poraodojuca.net


#include "balootextsearchrunner.h"

#include <QtGlobal>
#include <QAction>
#include <QIcon>
#include <QDir>
#include <KRun>
#include <KRunner/QueryMatch>
#include <KLocalizedString>
#include <QMimeDatabase>
#include <QTimer>
#include <QMimeData>
#include <QMimeType>

#include <Baloo/Query>

#include <KIO/OpenFileManagerWindowJob>


const char* const s_sourceCodeMimeTypes[] = {
    "text/css",
    "text/x-c++src",
    "text/x-c++hdr",
    "text/x-csrc",
    "text/x-chdr", // c header files
    "text/x-python",
    "text/x-assembly",
    "text/x-java",
    "text/x-objsrc",
    "text/x-ruby",
    "text/x-scheme",
    "text/x-pascal",
    "text/x-yacc",
    "text/x-sed",
    "text/x-haskell",
    "text/asp",
    "application/x-awk",
    "application/x-cgi",
    "application/x-csh",
    "application/x-ipynb+json",
    "application/x-java",
    "application/x-javascript",
    "application/javascript",
    "application/x-perl",
    "application/x-php",
    "application/x-python",
    "application/x-sh",

    // end of list
    nullptr
};


TextSearchRunner::TextSearchRunner(QObject* parent, const QVariantList& args)
  : Plasma::AbstractRunner(parent, args)
{
}

TextSearchRunner::TextSearchRunner(QObject* parent, const QString& serviceId)
  : Plasma::AbstractRunner(parent, serviceId)
{
}

void TextSearchRunner::init()
{
  Plasma::RunnerSyntax syntax(QStringLiteral(":q"), i18n("Search through source code files"));
}

TextSearchRunner::~TextSearchRunner()
{
}


QStringList TextSearchRunner::categories() const
{
  QStringList list;
  list << i18n("Source Code");

  return list;
}

QIcon TextSearchRunner::categoryIcon(const QString& category) const
{

    return Plasma::AbstractRunner::categoryIcon(category);
}

QStringList TextSearchRunner::sourceCodeMimeTypes()
{
  QStringList l;

  for (int i = 0; s_sourceCodeMimeTypes[i]; ++i)
    l << QLatin1String(s_sourceCodeMimeTypes[i]);
  return l;
}

QList<Plasma::QueryMatch> TextSearchRunner::match(Plasma::RunnerContext& context, const QString& type,
                                              const QString& category)
{
    if (!context.isValid())
        return QList<Plasma::QueryMatch>();

    const QStringList categories = context.enabledCategories();
    if (!categories.isEmpty() && !categories.contains(category))
        return QList<Plasma::QueryMatch>();

    Baloo::Query query;
    QString s_base_query;
    QString s_query = context.query();
    for (const auto& i: TextSearchRunner::sourceCodeMimeTypes()){
      s_base_query += "mimetype:" + i + " OR ";
    }
    s_base_query.remove(QRegExp("OR\\s*$"));

    s_query = s_base_query + s_query;
    query.setSearchString(s_query);
    // only to avoid warnings...
    query.setType(type);
    query.setType("");
    query.setLimit(10);

    Baloo::ResultIterator it = query.exec();

    QList<Plasma::QueryMatch> matches;

    QMimeDatabase mimeDb;

    // KRunner is absolutely retarded and allows plugins to set the global
    // relevance levels. so Baloo should not set the relevance of results too
    // high because then Applications will often appear after if the application
    // runner has not a higher relevance. So stupid.
    // Each runner plugin should not have to know about the others.
    // Anyway, that's why we're starting with .75
    QString mimes;
    float relevance = .75;
    while (context.isValid() && it.next()) {
        Plasma::QueryMatch match(this);
        QString localUrl = it.filePath();
        const QUrl url = QUrl::fromLocalFile(localUrl);

	QMimeType mimeType = mimeDb.mimeTypeForFile(localUrl);
        QString iconName = mimeType.iconName();
        match.setIconName(iconName);
        match.setId(it.filePath());
        match.setText(url.fileName());
        match.setData(url);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setMatchCategory(category);
        match.setRelevance(relevance);
        relevance -= 0.05;

	QString folderPath = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();
        if (folderPath.startsWith(QDir::homePath())) {
	  folderPath.replace(0, QDir::homePath().length(), QStringLiteral("~"));
        }
        match.setSubtext(folderPath);

        matches << match;
    }

    return matches;
}

void TextSearchRunner::match(Plasma::RunnerContext& context)
{
    const QString text = context.query();
    //
    // Baloo (as of 2014-11-20) is single threaded. It has an internal mutex which results in
    // queries being queued one after another. Also, Baloo is really really slow for small queries
    // For example - on my SSD, it takes about 1.4 seconds for 'f' with an SSD.
    // When searching for "fire", it results in "f", "fi", "fir" and then "fire" being searched
    // We're therefore hacking around this by having a small delay for very short queries so that
    // they do not get queued internally in Baloo
    //
    if (text.length() <= 3) {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.setSingleShot(true);
        timer.start(100);
        loop.exec();

        if (!context.isValid()) {
            return;
        }
    }

    QList<Plasma::QueryMatch> matches;
    matches << match(context, QStringLiteral("Text"), i18n("Source Code"));

    context.addMatches(matches);
}

void TextSearchRunner::run(const Plasma::RunnerContext&, const Plasma::QueryMatch& match)
{
    const QUrl url = match.data().toUrl();

    if (match.selectedAction() && match.selectedAction()->data().toString() == QLatin1String("openParentDir")) {
        KIO::highlightInFileManager({url});
        return;
    }

    new KRun(url, 0);
}


QMimeData *TextSearchRunner::mimeDataForMatch(const Plasma::QueryMatch &match)
{
    QMimeData *result = new QMimeData();
    result->setUrls({match.data().toUrl()});
    return result;
}

K_EXPORT_PLASMA_RUNNER(balootextsearchrunner, TextSearchRunner)

#include "balootextsearchrunner.moc"
