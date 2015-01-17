#include "control_server.h"

#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkInterface>

#include "Tufao/Headers"
#include "Tufao/HttpServerRequest"
#include "Tufao/HttpServerResponse"
#include "Tufao/NotFoundHandler"

#include "constants.h"

#define STREAM_INFO_PATH "/streamInfo"

ControlServer::ControlServer(const QString &stream_secret)
    : m_stream_secret(stream_secret)
{
    // Sets up the request router.
    m_router.map({QRegularExpression("^" STREAM_INFO_PATH "(/|$)"), "GET",
                  streamInfoHandler()});
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

        QJsonObject stream_info;
        stream_info.insert("stream_secret", m_stream_secret);
        stream_info.insert("stream_urls", streamUrls());

        // Creates the JSON reply.
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
