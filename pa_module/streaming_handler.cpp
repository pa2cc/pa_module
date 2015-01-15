#include "streaming_handler.h"

#include <functional>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>

#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/NotFoundHandler>

#include "constants.h"

StreamingHandler::StreamingHandler()
    : m_file_server(new Tufao::HttpFileServer(OUT_PATH))
{
    // Creates the master playlist.
    createMasterPlaylist();
}

StreamingHandler::~StreamingHandler() {
    // Removes the master playlist and the out-directory (if present)
    QFile().remove(OUT_PATH MASTER_PLAYLIST_FILENAME);
    QDir(OUT_PATH).rmpath(OUT_PATH);
}

void StreamingHandler::createMasterPlaylist() {
    // Creates the output path if it does not exist yet.
    Q_ASSERT(QDir().mkpath(OUT_PATH) &&
             "Could not create the output directory.");

    // Opens the master playlist file.
    QFile f(OUT_PATH MASTER_PLAYLIST_FILENAME);
    bool ok = f.open(QIODevice::WriteOnly);
    Q_ASSERT(ok && "Could not open the master playlist file");

    // Writes the contents.
    QString contents = QString(
                "#EXTM3U\n"
                "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=%1,CODECS=\"mp4a.40.2\"\n"
                "%2")
            .arg(BIT_RATE_BPS)
            .arg(PLAYLIST_FILENAME);
    f.write(contents.toUtf8());

    // Closes the file.
    f.close();
}

Tufao::HttpServerRequestRouter::Handler StreamingHandler::handler() {
    return  [this](Tufao::HttpServerRequest &request,
                   Tufao::HttpServerResponse &response)
    {
        // Adds the CORS header.
        response.headers().insert("Access-Control-Allow-Origin", "*");

        // Removes the prefix from the request URL.
        QString path = request.url().path(QUrl::FullyDecoded);
        if (path.startsWith(HTTP_STREAM_PREFIX)) {
            path.remove(0, QString(HTTP_STREAM_PREFIX).size());

            QUrl stripped_url = request.url();
            stripped_url.setPath(path);
            request.setUrl(stripped_url);
        }

        // Forwards the request to the file server.
        return m_file_server->handleRequest(request, response);
    };
}
