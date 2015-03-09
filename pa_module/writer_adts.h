#ifndef WRITER_ADTS_H
#define WRITER_ADTS_H

#include <QtCore/QtGlobal>

#include "writer_av_base.h"

class ADTSWriter : public BaseAVWriter {
public:
    explicit ADTSWriter(PASink *pa_sink);

private:
    Q_DISABLE_COPY(ADTSWriter)
};

#endif // WRITER_ADTS_H
