#ifndef HTTP_STREAMING_SERVER_H
#define HTTP_STREAMING_SERVER_H

#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#define signals Q_SIGNALS
#define slots Q_SLOTS
#include <Tufao/HttpServer>
#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequestRouter>
#undef signals
#undef slots

class BaseAVWriter;

class HttpStreamingServer {
public:
    struct CORS {
        static const char kAllowOrigin[];
    };

    explicit HttpStreamingServer(BaseAVWriter *writer);
    virtual ~HttpStreamingServer();

    QString streamSecret() const;
    int streamingPort() const;

private:
    static QString generateStreamSecret(int secret_length);

    void createMasterPlaylist();
    Tufao::HttpServerRequestRouter::Handler handler();

    BaseAVWriter *m_writer;
    QString m_stream_secret;
    Tufao::HttpServer m_http_server;
    Tufao::HttpServerRequestRouter m_router;
    QScopedPointer<Tufao::HttpFileServer> m_file_server;

    Q_DISABLE_COPY(HttpStreamingServer)
};

#endif // HTTP_STREAMING_SERVER_H
