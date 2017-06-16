#ifndef CONTROL_SERVER
#define CONTROL_SERVER

#include <QtCore/QJsonValue>
#include <QtCore/QObject>

class ControlServer : public QObject {
    Q_OBJECT

public:
    virtual ~ControlServer() {}

    virtual void sendMessage(const QString &type, const QJsonValue &payload) =0;

Q_SIGNALS:
    void clientConnected();
    void clientDisconnected();

    void messageReceived(const QString &type, const QJsonValue &payload);
};

#endif // CONTROL_SERVER

