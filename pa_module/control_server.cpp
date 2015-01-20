#include "control_server.h"

#include <algorithm>

#include <QByteArray>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QUrlQuery>
#include <QTimer>

#include "Tufao/Headers"
#include "Tufao/HttpServerRequest"
#include "Tufao/HttpServerResponse"
#include "Tufao/NotFoundHandler"

#include "constants.h"
#include "change_notifier.h"

#define STREAM_INFO_PATH "/streamInfo"
#define VOLUME_INFO_PATH "/volumeInfo"

#define VOLUME_MAX_WAIT_MS 50000

ControlServer::ControlServer(const QString &stream_secret,
                             ChangeNotifier<int> *volume_notifier)
    : m_stream_secret(stream_secret)
    , m_volume_notifier(volume_notifier)
{
    // Sets up the request router.
    m_router.map({QRegularExpression("^" STREAM_INFO_PATH "(/|$)"), "GET",
                  streamInfoHandler()});
    m_router.map({QRegularExpression("^" VOLUME_INFO_PATH "(/|$)"), "GET",
                  volumeInfoHandler()});
    m_router.map({QRegularExpression(""), Tufao::NotFoundHandler::handler()});

    bool (ControlServer:: *request_handler)(
                Tufao::HttpServerRequest &request,
                Tufao::HttpServerResponse &response) =
            &ControlServer::handleRequest;
    QObject::connect(&m_http_server, &Tufao::HttpServer::requestReady,
                     this, request_handler);

    // Starts the HTTP server.
    bool ok = m_http_server.listen(QHostAddress::Any, CONTROL_SERVER_PORT);
    Q_ASSERT(ok && "Could not open the control server socket.");
}

ControlServer::~ControlServer() {
}

bool ControlServer::handleRequest(Tufao::HttpServerRequest &request,
                                  Tufao::HttpServerResponse &response) {
    // We only accept requests from localhost.
    if (!request.socket().localAddress().isLoopback()) {
        response.writeHead(Tufao::HttpResponseStatus::FORBIDDEN, "Forbidden");
        response.end();
        return true;
    }

    // Adds the CORS header.
    if (request.headers().contains("Origin")) {
        response.headers().insert(
                    "Access-Control-Allow-Origin", CORS_ALLOW_ORIGIN);
    }
    return m_router.handleRequest(request, response);
}

static QJsonArray streamUrls() {
    // Generates the stream URLs for all IPs of this machine.
    QJsonArray stream_urls;
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.isLoopback()) {
            continue;
        }

        QString address_str =
                address.protocol() == QAbstractSocket::IPv4Protocol
                    ? address.toString()
                    : QString("[%1]").arg(address.toString());
        QString stream_url = QString("http://%1:%2/%3")
                .arg(address_str)
                .arg(STREAM_SERVER_PORT)
                .arg(MASTER_PLAYLIST_FILENAME);
        stream_urls.append(stream_url);
    }

    return stream_urls;
}

Tufao::HttpServerRequestRouter::Handler ControlServer::streamInfoHandler() {
    return [this](Tufao::HttpServerRequest &request,
                  Tufao::HttpServerResponse &response)
    {
        Q_UNUSED(request);

        // Creates the JSON reply.
        QJsonObject stream_info;
        stream_info.insert("stream_secret", m_stream_secret);
        stream_info.insert("stream_urls", streamUrls());

        QByteArray body = QJsonDocument(stream_info)
                .toJson(QJsonDocument::Compact);

        // Writes the reply.
        response.writeHead(Tufao::HttpResponseStatus::OK);
        response.headers().insert("Content-Type", "application/json");
        response.write(body);
        response.end();
        return true;
    };
}

Tufao::HttpServerRequestRouter::Handler ControlServer::volumeInfoHandler() {
    return [this](Tufao::HttpServerRequest &request,
                  Tufao::HttpServerResponse &response)
    {
        // Reads the known volume.
        bool ok;
        double known_volume_percent =
                QUrlQuery(request.url()).queryItemValue("v").toInt(&ok);
        if (!ok) {
            known_volume_percent = -2;
        }

        // Handlers.
        ChangeNotifier<int>::update_f on_update = [this, &response]() {
            int volume_percent = m_volume_notifier->value();

            // Creates the JSON reply.
            QJsonObject volume_info;
            volume_info.insert("muted", volume_percent == -1);
            volume_info.insert("volume_percent", std::max(0, volume_percent));

            QByteArray body = QJsonDocument(volume_info)
                    .toJson(QJsonDocument::Compact);

            // Writes the reply.
            response.writeHead(Tufao::HttpResponseStatus::OK);
            response.headers().insert("Content-Type", "application/json");
            response.write(body);
            response.end();
        };

        ChangeNotifier<int>::timeout_f on_timeout = [&response]() {
            response.writeHead(Tufao::HttpResponseStatus::NO_CONTENT);
            response.end();
        };

        // We wait if the known volume is not different.
        m_volume_notifier->waitForUpdate(known_volume_percent,
                                         VOLUME_MAX_WAIT_MS,
                                         on_update, on_timeout);

        return true;
    };
}
