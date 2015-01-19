#ifndef CHANGE_NOTIFIER_H
#define CHANGE_NOTIFIER_H

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

template<class T>
class ChangeNotifier {
public:
    ChangeNotifier() {}
    virtual ~ChangeNotifier() {}

    void updateValue(const T &value) {
        QMutexLocker l(&m_mutex);
        if (m_value != value) {
            m_value = value;
            m_wc.wakeAll();
        }
    }

    bool waitForUpdate(const T &known_value, unsigned long max_wait_ms,
                       T *new_value) {
        Q_ASSERT(new_value);

        QMutexLocker l(&m_mutex);
        if (known_value == m_value) {
            // Waits for the value to change.
            bool ok = m_wc.wait(l.mutex(), max_wait_ms);
            if (!ok) {
                // Timeout occured.
                return false;
            }
        }

        *new_value = m_value;
        return true;
    }

    const T &readValue() const {
        QMutexLocker l(&m_mutex);
        return m_value;
    }

private:
    T m_value;
    QMutex m_mutex;
    QWaitCondition m_wc;
};

#endif // CHANGENOTIFIER_H
