#include "rpc_processor.h"
#include "rpc_protocol.h"

#include "io/input_buf.h"
#include "io/output_buf.h"

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

class MessageSerialier : public rpc::RpcSyncChannel::Serializer {
  public:
    MessageSerialier() {
    }
    virtual ~MessageSerialier() {
    }

  private:
    io::OutputObject* Serialize(uint64 id, const std::string& func_name,
                                const Message& msg) const {
      return new io::OutVectorObject(new RequestObject(id, func_name, msg));
    }

    DISALLOW_COPY_AND_ASSIGN(MessageSerialier);
};
}

namespace rpc {

RpcSyncChannel::RpcSyncChannel(Sender* sender)
    : sender_(sender), id_(1) {
  DCHECK_NOTNULL(sender);

  serializer_.reset(new MessageSerialier);
}

RpcSyncChannel::~RpcSyncChannel() {
  STLMapClear(&call_back_map_);
}

void RpcSyncChannel::process(io::Connection* conn, io::InputBuf* input_buf,
                               const TimeStamp& time_stamp) {
  ClientCallback* cb = NULL;
  AutoRunner r(cb);

  detail::RpcAttr* attr = static_cast<detail::RpcAttr*>(conn->getAttr());
  uint64 id = attr->header()->id;
  {
    ScopedMutex l(&mutex_);
    cb = call_back_map_[id];
    call_back_map_.erase(id);
  }

  Message* reply = cb->getResponse();
  //  reply->ParseFromArray(input_buf);  // FIXME : 1
}

void RpcSyncChannel::CallMethod(const MethodDescriptor* method,
                                  RpcController* controller,
                                  const Message* request, Message* response,
                                  google::protobuf::Closure* done) {
  // why not dynamic_cast, because google not allowed.
  ClientCallback* cb = static_cast<ClientCallback*>(done);
  cb->SetContext(method, request, response);

  uint64 id;
  {
    ScopedMutex l(&mutex_);
    id = id_;
    call_back_map_[id_++] = cb;
  }
  io::OutputObject* out_obj = serializer_->Serialize(id_, method->full_name(),
                                                     *request);
  sender_->send(out_obj);
}

}
