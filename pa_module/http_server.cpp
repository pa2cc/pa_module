#include "http_server.h"

#include <Tufao/NotFoundHandler>

#include "constants.h"
#include "control_handler.h"
#include "streaming_handler.h"

#define SERVER_PORT    51348

HttpServer::HttpServer()
    : m_control_handler(new ControlHandler)
    , m_streaming_handler(new StreamingHandler)
{
    // Sets up the request router.
    m_router.map({QRegularExpression("^" HTTP_CONTROL_PREFIX "/"),
                  m_control_handler->handler()});
    m_router.map({QRegularExpression("^" HTTP_STREAM_PREFIX "/"),
                  m_streaming_handler->handler()});
    m_router.map({QRegularExpression(""), Tufao::NotFoundHandler::handler()});

    QObject::connect(&m_http_server, &Tufao::HttpServer::requestReady,
                     &m_router, &Tufao::HttpServerRequestRouter::handleRequest);

    // Starts the Http server.
    bool ok = m_http_server.listen(QHostAddress::Any, SERVER_PORT);
    Q_ASSERT(ok && "Could not open the Http server socket.");
}

HttpServer::~HttpServer() {
}
