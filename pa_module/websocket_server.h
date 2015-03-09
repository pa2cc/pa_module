#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>
#include <QtNetwork/QSslError>

#include "control_server.h"

class QWebSocketServer;
class QWebSocket;

class WebsocketServer : public ControlServer {
    Q_OBJECT

public:
    explicit WebsocketServer(quint16 port);
    virtual ~WebsocketServer();

    void sendMessage(const QString &type, const QJsonValue &payload) override;

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);

private:
    QScopedPointer<QWebSocketServer> m_websocket_server;
    QMutex m_socket_mutex;
    QScopedPointer<QWebSocket> m_socket;
    QStringList m_pending_messages;
};


#endif // WEBSOCKET_SERVER_H
