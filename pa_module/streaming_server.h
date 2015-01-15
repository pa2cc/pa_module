#ifndef STREAMING_SERVER_H
#define STREAMING_SERVER_H

#include <QScopedPointer>
#include <QtGlobal>

#include <Tufao/HttpServer>
#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequestRouter>

class StreamingServer {
public:
    StreamingServer();
    virtual ~StreamingServer();

private:
    void createMasterPlaylist();
    Tufao::HttpServerRequestRouter::Handler handler();

    Tufao::HttpServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;
    QScopedPointer<Tufao::HttpFileServer> m_file_server;

    Q_DISABLE_COPY(StreamingServer)
};

#endif // STREAMING_SERVER_H
