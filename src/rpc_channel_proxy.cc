#include "rpc_protocol.h"
#include "rpc_channel_proxy.h"

#include "io/io_buf.h"

namespace {

class RequestObject : public io::OutVectorObject::IoObject {
  public:
    RequestObject(uint64 id, const std::string& fun_name, const Message& msg) {
      buf_.reset(new io::OutputBuf(RPC_HEADER_LENGTH + msg.ByteSize()));
      MessageHeader* hdr;
      int len = RPC_HEADER_LENGTH;
      buf_->Next((char**) &hdr, &len);

      hdr->id = id;
      hdr->fun_id = Hash(fun_name);
      hdr->flags = 0;
      SET_LAST_TAG(*hdr);
      hdr->length = msg.ByteSize();

      char* data;
      len = msg.ByteSize();
      buf_->Next(&data, &len);
      msg.SerializePartialToArray(data, msg.ByteSize());

      iovec io;
      io.iov_base = hdr;
      io.iov_len = RPC_HEADER_LENGTH + msg.ByteSize();
      ios_.push_back(io);
    }
    virtual ~RequestObject() {
    }

  private:
    virtual const std::vector<iovec>& ioVec() const {
      return ios_;
    }

    std::vector<iovec> ios_;
    scoped_ptr<io::OutputBuf> buf_;

    DISALLOW_COPY_AND_ASSIGN(RequestObject);
};

io::OutputObject* Serialize(uint64 id, const std::string& func_name,
                            const Message& msg) const {
  return new io::OutVectorObject(new RequestObject(id, func_name, msg));
}

}

namespace rpc {

RpcChannelProxy::RpcChannelProxy(Sender* sender)
    : sender_(sender), id_(1) {
  DCHECK_NOTNULL(sender);
}

RpcChannelProxy::~RpcChannelProxy() {
  STLMapClear(&call_back_map_);
}

void RpcChannelProxy::CallMethod(
    const google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response, ::google::protobuf::Closure* done) {
  ClientCallback* cb = reinterpret_cast<ClientCallback*>(done);

  uint64 id;
  {
    ScopedMutex l(&mutex_);
    id = id_;
    call_back_map_[id_++] = cb;
  }

  cb->SetContext(id, method, request, response);
  io::OutputObject* out_obj = Serialize(id_, method->full_name(), *request);
  sender_->send(out_obj);
}

}
