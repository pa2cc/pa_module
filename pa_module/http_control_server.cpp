#include "http_control_server.h"

#include <algorithm>

#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

#define signals Q_SIGNALS
#define slots Q_SLOTS
#include "Tufao/Headers"
#include "Tufao/HttpServerRequest"
#include "Tufao/HttpServerResponse"
#include "Tufao/NotFoundHandler"
#undef signals
#undef slots

#include "change_notifier.h"
#include "http_streaming_server.h"
#include "writer_av_base.h"

namespace {
const char kStreamInfoPath[] = "/streamInfo";
const char kVolumeInfoPath[] = "/volumeInfo";

const int kVolumeMaxWaitMs = 50000; // 50s
} // namespace


HttpControlServer::HttpControlServer(BaseAVWriter *writer,
                                     ChangeNotifier<int> *volume_notifier,
                                     quint16 port)
    : m_writer(writer)
    , m_streaming_server(new HttpStreamingServer(writer))
    , m_volume_notifier(volume_notifier)
{
    // Sets up the request router.
    m_router.map({QRegularExpression(QString("^%1(/|$)").arg(kStreamInfoPath)),
                  "GET", streamInfoHandler()});
    m_router.map({QRegularExpression(QString("^%1(/|$)").arg(kVolumeInfoPath)),
                  "GET", volumeInfoHandler()});
    m_router.map({QRegularExpression(""), Tufao::NotFoundHandler::handler()});

    bool (HttpControlServer:: *request_handler)(
                Tufao::HttpServerRequest &request,
                Tufao::HttpServerResponse &response) =
            &HttpControlServer::handleRequest;
    QObject::connect(&m_http_server, &Tufao::HttpServer::requestReady,
                     this, request_handler);

    // Sets the SSL certificate.
    QFile cert_file(":/certs/localhost.crt");
    cert_file.open(QIODevice::ReadOnly);
    QSslCertificate cert(cert_file.readAll());
    m_http_server.setLocalCertificate(cert);

    // Sets the SSL key.
    QFile key_file(":/certs/localhost.key");
    key_file.open(QIODevice::ReadOnly);
    QSslKey key(key_file.readAll(), QSsl::Rsa);
    m_http_server.setPrivateKey(key);

    // Starts the HTTP server.
    bool ok = m_http_server.listen(QHostAddress::Any, port);
    Q_ASSERT(ok && "Could not open the control server socket.");
}

HttpControlServer::~HttpControlServer() {
}

void HttpControlServer::sendMessage(const QString &type,
                                    const QJsonValue &payload) {
    Q_UNUSED(type);
    Q_UNUSED(payload);

    Q_ASSERT(false && "Not implemented.");
}

bool HttpControlServer::handleRequest(Tufao::HttpServerRequest &request,
                                      Tufao::HttpServerResponse &response) {
    // We only accept requests from localhost.
    if (!request.socket().localAddress().isLoopback()) {
        response.writeHead(Tufao::HttpResponseStatus::FORBIDDEN, "Forbidden");
        response.end();
        return true;
    }

    // Adds the CORS header.
    if (request.headers().contains("Origin")) {
        response.headers().insert("Access-Control-Allow-Origin",
                                  HttpStreamingServer::CORS::kAllowOrigin);
    }
    return m_router.handleRequest(request, response);
}

QJsonArray HttpControlServer::streamUrls() {
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
                .arg(m_streaming_server->streamingPort())
                .arg(m_writer->masterPlaylistFilename());
        stream_urls.append(stream_url);
    }

    return stream_urls;
}

Tufao::HttpServerRequestRouter::Handler HttpControlServer::streamInfoHandler() {
    return [this](Tufao::HttpServerRequest &request,
                  Tufao::HttpServerResponse &response)
    {
        Q_UNUSED(request);

        // Creates the JSON reply.
        QJsonObject stream_info;
        stream_info.insert("stream_secret", m_streaming_server->streamSecret());
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

Tufao::HttpServerRequestRouter::Handler HttpControlServer::volumeInfoHandler() {
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
        m_volume_notifier->waitForUpdate(known_volume_percent, kVolumeMaxWaitMs,
                                         on_update, on_timeout);

        return true;
    };
}
