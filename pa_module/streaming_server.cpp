#include "streaming_server.h"

#include <functional>

#include <QDebug>
#include <QDir>
#include <QFile>

#include <Tufao/HttpFileServer>
#include <Tufao/NotFoundHandler>

#include "constants.h"

#define HLS_SERVER_PORT 51348

StreamingServer::StreamingServer() {
    // Creates the master playlist.
    createMasterPlaylist();

    // Starts the streaming server.

    // FIXME: temporary fix for Tufao::HttpFileServer.
    QString out_path =  QString(OUT_PATH)
            .remove(QRegularExpression(QString(QDir::separator()) + '$'));

    m_router.reset(new Tufao::HttpServerRequestRouter({
        {QRegularExpression(""), Tufao::HttpFileServer::handler(out_path)},
        {QRegularExpression(""), Tufao::NotFoundHandler::handler()}
    }));

    QObject::connect(&m_http_server, &Tufao::HttpServer::requestReady,
                     m_router.data(),
                     &Tufao::HttpServerRequestRouter::handleRequest);

    qDebug() << "Starting streaming server.";
    bool ok = m_http_server.listen(QHostAddress::Any, HLS_SERVER_PORT);
    Q_ASSERT(ok && "Could not open the streaming server socket.");
}

StreamingServer::~StreamingServer() {
    // Removes the master playlist and the out-directory (if present)
    QFile().remove(OUT_PATH MASTER_PLAYLIST_FILENAME);
    QDir(OUT_PATH).rmpath(OUT_PATH);
}

void StreamingServer::createMasterPlaylist() {
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
