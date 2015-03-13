#ifndef PA_SINK_H
#define PA_SINK_H

#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>

struct pa_module;
class PASinkPriv;
class Writer;

class PASink {
public:
    static PASink &instance();
    void drop();

    int init(pa_module *module, Writer *writer);

    int sampleRateHz() const;
    int bitRateBps() const;
    int numChannels() const;

    bool isMuted() const;

    quint32 volume() const;
    quint32 minVolume() const;
    quint32 maxVolume() const;
    quint16 volumeStepSize() const;

    QScopedPointer<PASinkPriv> d;

private:
    friend class QScopedPointerDeleter<PASink>;
    PASink();
    virtual ~PASink();
    Q_DISABLE_COPY(PASink)

    static QScopedPointer<PASink> m_instance;
};

#endif // PA_SINK_H
