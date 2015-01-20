#include "change_notifier.h"

_ChangeNotifierWaiter::_ChangeNotifierWaiter(update_f update_cb,
                                             timeout_f timeout_cb)
    : QObject()
    , m_update_cb(update_cb)
    , m_timeout_cb(timeout_cb)
{
}

void _ChangeNotifierWaiter::onUpdate() {
    m_update_cb();
    deleteLater();
}

void _ChangeNotifierWaiter::onTimeout() {
    if (m_timeout_cb) {
        m_timeout_cb();
    }
    deleteLater();
}
