#ifndef WRITER_ADTS_H
#define WRITER_ADTS_H

#include <QtGlobal>

#include "writer_base.h"

class ADTSWriter : public BaseWriter {
public:
    ADTSWriter();

private:
    Q_DISABLE_COPY(ADTSWriter)
};

#endif // WRITER_ADTS_H
