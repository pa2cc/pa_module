#ifndef CONTROL_HANDLER_H
#define CONTROL_HANDLER_H

#include <QScopedPointer>

#include <Tufao/HttpServerRequestRouter>

class ControlHandler {
public:
    ControlHandler();
    virtual ~ControlHandler();

    Tufao::HttpServerRequestRouter::Handler handler();

private:
    Tufao::HttpServerRequestRouter::Handler streamUrlHandler();

    Tufao::HttpServerRequestRouter m_router;

    Q_DISABLE_COPY(ControlHandler)
};

#endif // CONTROL_HANDLER_H
