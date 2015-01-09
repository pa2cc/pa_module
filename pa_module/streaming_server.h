#ifndef STREAMINGSERVER_H
#define STREAMINGSERVER_H

#include <QScopedPointer>
#include <QtGlobal>

#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequestRouter>

class StreamingServer {
public:
    StreamingServer();
    virtual ~StreamingServer();

private:
    void createMasterPlaylist();

    Tufao::HttpServer m_http_server;
    QScopedPointer<Tufao::HttpServerRequestRouter> m_router;

    Q_DISABLE_COPY(StreamingServer)
};

#endif // STREAMINGSERVER_H
