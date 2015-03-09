#ifndef WRITER_H
#define WRITER_H

extern "C" {
#include <config.h>
#include <pulse/sample.h>
} // extern "C"

class Writer {
public:
    virtual ~Writer() {}

    virtual pa_sample_format_t sampleFormat() const =0;
    virtual ssize_t write(const void *buf, size_t length) =0;
};

#endif // WRITER_H

