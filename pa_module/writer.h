#ifndef WRITER_H
#define WRITER_H

class Writer {
public:
    virtual ~Writer() {}

    virtual  ssize_t write(const void *buf, size_t count) =0;
};

#endif // WRITER_H

