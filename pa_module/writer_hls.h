#ifndef WRITER_HLS_H
#define WRITER_HLS_H

#include <QtGlobal>

#include "writer_base.h"

class HLSWriter : public BaseWriter {
public:
    HLSWriter();

private:
    Q_DISABLE_COPY(HLSWriter)
};

#endif // WRITER_HLS_H

