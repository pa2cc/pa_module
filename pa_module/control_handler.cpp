#include "control_handler.h"

#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHostAddress>
#include <QNetworkInterface>

#include "Tufao/Headers"
#include "Tufao/HttpServerRequest"
#include "Tufao/HttpServerResponse"

#include "constants.h"

ControlHandler::ControlHandler() {
    m_router.map({QRegularExpression(HTTP_CONTROL_STREAM_URL), "GET",
                  streamUrlHandler()});
}

ControlHandler::~ControlHandler() {
}

Tufao::HttpServerRequestRouter::Handler ControlHandler::handler() {
    return [this](Tufao::HttpServerRequest &request,
                  Tufao::HttpServerResponse &response)
    {
        return m_router.handleRequest(request, response);
    };
}

Tufao::HttpServerRequestRouter::Handler ControlHandler::streamUrlHandler() {
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

            QString stream_url = QString("http://%1%2")
                    .arg(address.toString())
                    .arg(HTTP_MASTER_PLAYLIST_URL);
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
