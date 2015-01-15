#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QScopedPointer>
#include <QtGlobal>

#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequestRouter>

class ControlHandler;
class StreamingHandler;

class HttpServer {
public:
    HttpServer();
    virtual ~HttpServer();

private:
    Tufao::HttpServerRequestRouter::Handler handler();

    Tufao::HttpServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;

    QScopedPointer<ControlHandler> m_control_handler;
    QScopedPointer<StreamingHandler> m_streaming_handler;

    Q_DISABLE_COPY(HttpServer)
};

#endif // HTTP_SERVER_H
