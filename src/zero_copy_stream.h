#pragma once

#include "io/input_stream.h"
#include "io/output_stream.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace rpc {

class InputStream : public ::google::protobuf::io::ZeroCopyInputStream {
  public:
    explicit InputStream(io::InputBuf* buf)
        : buf_(buf) {
      CHECK_NOTNULL(buf);
    }
    virtual ~InputStream() {
    }

    virtual bool Next(const void** buf, int* len) {
      return buf_->Next((const char**) buf, len);
    }

    virtual bool Skip(int len) {
      return buf_->Skip(len);
    }
    virtual void BackUp(int len) {
      buf_->Backup(len);
    }
    virtual int64 ByteCount() const {
      return buf_->ByteCount();
    }

  private:
    scoped_ptr<io::InputBuf> buf_;

    DISALLOW_COPY_AND_ASSIGN (InputStream);
};

class OutputStream : public ::google::protobuf::io::ZeroCopyOutputStream {
  public:
    explicit OutputStream(uint32 size)
        : buf_(new io::OutputBuf(size)) {
    }
    virtual ~OutputStream() {
    }

    virtual bool Next(void** data, int* size) {
      return buf_->Next((char**) data, size);
    }
    virtual void BackUp(int len) {
      buf_->Backup(len);
    }
    virtual uint64 ByteCount() const {
      return buf_->ByteCount();
    }

  private:
    scoped_ptr<io::OutputBuf> buf_;

    DISALLOW_COPY_AND_ASSIGN (OutputStream);
};

}
