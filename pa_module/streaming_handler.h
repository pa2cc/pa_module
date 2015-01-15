#ifndef STREAMING_HANDLER_H
#define STREAMING_HANDLER_H

#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

#include <Tufao/HttpFileServer>
#include <Tufao/HttpServerRequestRouter>

class StreamingHandler {
public:
    StreamingHandler();
    virtual ~StreamingHandler();

    Tufao::HttpServerRequestRouter::Handler handler();

private:
    void createMasterPlaylist();

    QScopedPointer<Tufao::HttpFileServer> m_file_server;

    Q_DISABLE_COPY(StreamingHandler)
};

#endif // STREAMING_HANDLER_H
