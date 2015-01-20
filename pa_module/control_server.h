#ifndef CONTROL_SERVER_H
#define CONTROL_SERVER_H

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerRequestRouter>
#include <Tufao/HttpServerResponse>

template<class T> class ChangeNotifier;

class ControlServer : public QObject {
    Q_OBJECT

public:
    ControlServer(const QString &stream_secret,
                  ChangeNotifier<int> *volume_notifier);
    virtual ~ControlServer();

private slots:
    bool handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);

private:
    Tufao::HttpServerRequestRouter::Handler handleRequest();
    Tufao::HttpServerRequestRouter::Handler streamInfoHandler();
    Tufao::HttpServerRequestRouter::Handler volumeInfoHandler();

    QString m_stream_secret;
    ChangeNotifier<int> *m_volume_notifier;

    Tufao::HttpServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;

    Q_DISABLE_COPY(ControlServer)
};

#endif // CONTROL_SERVER_H
