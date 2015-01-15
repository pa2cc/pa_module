#include "control_server.h"

#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHostAddress>
#include <QNetworkInterface>

#include "Tufao/Headers"
#include "Tufao/HttpServerRequest"
#include "Tufao/HttpServerResponse"
#include "Tufao/NotFoundHandler"

#include "constants.h"

#define STREAM_INFO_PATH "/streamInfo"

ControlServer::ControlServer() {
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

    return m_router.handleRequest(request, response);
}

Tufao::HttpServerRequestRouter::Handler ControlServer::streamInfoHandler() {
    return [this](Tufao::HttpServerRequest &request,
                  Tufao::HttpServerResponse &response)
    {
        Q_UNUSED(request);

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

        // Creates the JSON reply.
        QByteArray body =
                QJsonDocument(stream_urls).toJson(QJsonDocument::Compact);

        // Writes the reply.
        response.writeHead(Tufao::HttpResponseStatus::OK, "OK");
        response.headers().insert("Content-Type", "application/json");
        response.write(body);
        response.end();
        return true;
    };
}
