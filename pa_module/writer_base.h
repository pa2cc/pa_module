#ifndef WRITER_BASE_H
#define WRITER_BASE_H

#include "writer.h"

class PASink;

class BaseWriter : public Writer {
public:
    PASink *pa_sink() const;

protected:
    explicit BaseWriter(PASink *pa_sink);

private:
    PASink *m_pa_sink;
};

#endif // WRITER_BASE_H
