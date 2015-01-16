#include "streaming_server.h"

#include <functional>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QUrl>

#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/NotFoundHandler>

#include "constants.h"

#define DEFAULT_SECRET_LENGTH_B 32

StreamingServer::StreamingServer(const QString &stream_secret)
    : m_stream_secret(stream_secret)
    , m_file_server(new Tufao::HttpFileServer(OUT_PATH))
{
    // Creates the master playlist.
    createMasterPlaylist();

    // Sets up the request router.
    m_router.map({QRegularExpression(""), handler()});
    m_router.map({QRegularExpression(""), Tufao::NotFoundHandler::handler()});

    QObject::connect(&m_http_server, &Tufao::HttpServer::requestReady,
                     &m_router, &Tufao::HttpServerRequestRouter::handleRequest);

    // Starts the HTTP server.
    bool ok = m_http_server.listen(QHostAddress::Any, STREAM_SERVER_PORT);
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

Tufao::HttpServerRequestRouter::Handler StreamingServer::handler() {
    return  [this](Tufao::HttpServerRequest &request,
                   Tufao::HttpServerResponse &response)
    {
        // TODO: Check if fileserver will handle this request.

        // Only adds CORS headers if the request contains an Origin header.
        bool do_add_cors_header = request.headers().contains("Origin");
        auto add_cors_header = [&response, do_add_cors_header]
                (const QByteArray &key, const QByteArray &value) {
            response.headers().insert(key, value);
        };

        // Adds the global CORS headers.
        const QByteArray &origin = request.headers().value("Origin");
        add_cors_header("Access-Control-Allow-Origin", origin);
        //add_cors_header("Access-Control-Allow-Credentials", "true");
        add_cors_header("Vary", "Origin");


        // Handles the actual request.
        if ("OPTIONS" == request.method()) {
            // This is a CORS preflight request.

            // Adds the Allow-Headers header if Request-Headers was set.
            const QStringList &request_headers = QString::fromUtf8(
                        request.headers().value(
                            "Access-Control-Request-Headers"))
                    .split(", ");
            if (request_headers.contains("authorization")) {
                add_cors_header("Access-Control-Allow-Headers", "authorization");
            }

            // Adds the Allow-Methods header if Request-Method was set.
            if (request.headers().contains("Access-Control-Request-Method")) {
                add_cors_header("Access-Control-Allow-Methods", "GET");
            }

            response.writeHead(Tufao::HttpResponseStatus::OK);
            response.end();
        } else if ("HEAD" == request.method() || "GET" == request.method()) {
            // This is the actual request for a file.

            // Checks if the authorization header is correct.
            const QString &secret =
                    QString::fromUtf8(request.headers().value("Authorization"));
            if (secret != m_stream_secret) {
                response.writeHead(Tufao::HttpResponseStatus::UNAUTHORIZED);
                response.end();
                return true;
            }

            // Forwards the request to the file server.
            bool handled = m_file_server->handleRequest(request, response);
            Q_ASSERT(handled); // We checked it at the beginning.
        } else {
            // Unknown method.
            response.writeHead(Tufao::HttpResponseStatus::METHOD_NOT_ALLOWED);
            response.headers().insert("Allow", "GET, HEAD, OPTIONS");
            response.end();
        }

        return true;
    };
}

QString StreamingServer::generateStreamSecret() {
    // Sets a seed for the random function.
    static bool qrand_initialized = false;
    if (!qrand_initialized) {
        qsrand(QTime::currentTime().msec());
        qrand_initialized = true;
    }

    // Generates the secret.
    const QString chars(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    QString secret;
    for(int i = 0; i < DEFAULT_SECRET_LENGTH_B; ++i) {
        int index = qrand() % chars.length();
        secret.append(chars[index]);
    }

    return secret;
}
