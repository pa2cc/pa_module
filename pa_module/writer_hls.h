#ifndef WRITER_HLS_H
#define WRITER_HLS_H

#include <QtCore/QtGlobal>

#include "writer_av_base.h"

class HLSWriter : public BaseAVWriter {
public:
    explicit HLSWriter(PASink *pa_sink);

private:
    Q_DISABLE_COPY(HLSWriter)
};

#endif // WRITER_HLS_H

