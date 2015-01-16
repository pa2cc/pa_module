#ifndef STREAMING_SERVER_H
#define STREAMING_SERVER_H

#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

#include <Tufao/HttpServer>
#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequestRouter>

class StreamingServer {
public:
    explicit StreamingServer(const QString &stream_secret);
    virtual ~StreamingServer();

    static QString generateStreamSecret();

private:
    void createMasterPlaylist();
    Tufao::HttpServerRequestRouter::Handler handler();

    QString m_stream_secret;
    Tufao::HttpServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;
    QScopedPointer<Tufao::HttpFileServer> m_file_server;

    Q_DISABLE_COPY(StreamingServer)
};

#endif // STREAMING_SERVER_H
