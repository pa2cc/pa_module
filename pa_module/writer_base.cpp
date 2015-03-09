#include "writer_base.h"

#include "pa_sink.h"

BaseWriter::BaseWriter(PASink *pa_sink)
    : m_pa_sink(pa_sink)
{
}

PASink *BaseWriter::pa_sink() const {
    return m_pa_sink;
}

