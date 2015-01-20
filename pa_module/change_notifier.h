#ifndef CHANGE_NOTIFIER_H
#define CHANGE_NOTIFIER_H

#include <functional>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QScopedPointer>
#include <QTimer>

class _ChangeNotifierHelper;
class _ChangeNotifierWaiter : public QObject {
    Q_OBJECT

public:
    typedef std::function<void ()> update_f;
    typedef std::function<void ()> timeout_f;

    _ChangeNotifierWaiter(update_f update_cb, timeout_f timeout_cb);

public slots:
    void onUpdate();
    void onTimeout();

private:
    update_f m_update_cb;
    timeout_f m_timeout_cb;
};


template<class T>
class ChangeNotifier {
public:
    typedef _ChangeNotifierWaiter::update_f update_f;
    typedef _ChangeNotifierWaiter::timeout_f timeout_f;

    ChangeNotifier();
    virtual ~ChangeNotifier() {}

    void updateValue(const T &value);
    void waitForUpdate(const T &known_value, unsigned long max_wait_ms,
                       update_f on_update, timeout_f on_timeout);
    const T &value() const;

private:
    QScopedPointer<_ChangeNotifierHelper> m_helper;

    T m_value;
    mutable QMutex m_mutex;
};

class _ChangeNotifierHelper : public QObject {
    Q_OBJECT

signals:
    void valueUpdated();
};


/******************************************************************************/


template<class T>
ChangeNotifier<T>::ChangeNotifier()
    : m_helper(new _ChangeNotifierHelper)
    , m_mutex(QMutex::Recursive)
{
}

template<class T>
void ChangeNotifier<T>::updateValue(const T &value) {
    QMutexLocker l(&m_mutex);
    if (m_value != value) {
        m_value = value;

        emit m_helper->valueUpdated();
    }
}

template<class T>
void ChangeNotifier<T>::waitForUpdate(const T &known_value,
                                      unsigned long max_wait_ms,
                                      update_f on_update,
                                      timeout_f on_timeout) {
    Q_ASSERT(on_update);

    QMutexLocker l(&m_mutex);
    // The value is newer -> do the update right away.
    if (known_value != m_value) {
        on_update();
        return;
    }

    if (known_value == m_value) {
        // Waits for the value to change.
        _ChangeNotifierWaiter *waiter =
                new _ChangeNotifierWaiter(on_update, on_timeout);
        QObject::connect(m_helper.data(), &_ChangeNotifierHelper::valueUpdated,
                         waiter, &_ChangeNotifierWaiter::onUpdate);
        QTimer::singleShot(max_wait_ms, waiter,
                           &_ChangeNotifierWaiter::onTimeout);
    }
}

template<class T>
const T &ChangeNotifier<T>::value() const {
    QMutexLocker l(&m_mutex);
    return m_value;
}

#endif // CHANGENOTIFIER_H
