#ifndef HTTP_CONTROL_SERVER_H
#define HTTP_CONTROL_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#define signals Q_SIGNALS
#define slots Q_SLOTS
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerRequestRouter>
#include <Tufao/HttpServerResponse>
#include <Tufao/HttpsServer>
#undef signals
#undef slots

#include "control_server.h"

class BaseAVWriter;
template<class T> class ChangeNotifier;
class HttpStreamingServer;
class QJsonArray;

class HttpControlServer : public ControlServer {
    Q_OBJECT

public:
    HttpControlServer(BaseAVWriter *writer,
                      ChangeNotifier<int> *volume_notifier, quint16 port);
    virtual ~HttpControlServer();

    void sendMessage(const QString &type, const QJsonValue &payload) override;

private Q_SLOTS:
    bool handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);

private:
    Tufao::HttpServerRequestRouter::Handler handleRequest();
    Tufao::HttpServerRequestRouter::Handler streamInfoHandler();
    Tufao::HttpServerRequestRouter::Handler volumeInfoHandler();

    QJsonArray streamUrls();

    BaseAVWriter *m_writer;
    QScopedPointer<HttpStreamingServer> m_streaming_server;
    ChangeNotifier<int> *m_volume_notifier;

    Tufao::HttpsServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;

    Q_DISABLE_COPY(HttpControlServer)
};

#endif // HTTP_CONTROL_SERVER_H
