#include "rpc_protocol.h"
#include "rpc_channel_proxy.h"

#include "io/io_buf.h"

namespace {

class RequestObject : public io::OutVectorObject::IoObject {
  public:
    RequestObject(uint64 id, const std::string& fun_name, const Message& msg) {
      buildData(id, fun_name, msg);
    }
    virtual ~RequestObject() {
    }

  private:
    virtual const std::vector<iovec>& ioVec() const {
      return ios_;
    }

    void buildData(uint64 id, const std::string& fun_name, const Message& msg);

    std::vector<iovec> ios_;
    scoped_ptr<io::ExternableChunk> chunk_;

    DISALLOW_COPY_AND_ASSIGN(RequestObject);
};

void RequestObject::buildData(uint64 id, const std::string& fun_name,
                              const Message& msg) {
  chunk_.reset(new io::ExternableChunk(RPC_HEADER_LENGTH + msg.ByteSize()));
  chunk_->ensureLeft(RPC_HEADER_LENGTH + msg.ByteSize());

  MessageHeader* hdr = reinterpret_cast<MessageHeader*>(chunk_->peekW());
  chunk_->skipWrite(RPC_HEADER_LENGTH);
  hdr->id = id;
  hdr->fun_id = Hash(fun_name);
  hdr->flags = 0;
  SET_LAST_TAG(*hdr);
  hdr->length = msg.ByteSize();

  msg.SerializePartialToArray(chunk_->peekW(), msg.ByteSize());
  chunk_->skipWrite(msg.ByteSize());

  iovec io;
  io.iov_base = hdr;
  io.iov_len = RPC_HEADER_LENGTH + msg.ByteSize();
  ios_.push_back(io);
}

io::OutputObject* Serialize(uint64 id, const std::string& func_name,
                            const Message& msg) const {
  return new io::OutVectorObject(new RequestObject(id, func_name, msg));
}
}

namespace rpc {

RpcChannelProxy::RpcContext::~RpcContext() {
  CallbackList cbs;
  {
    ScopedMutex l(&mutex);
    cbs.swap(cb_list);
    cb_map.clear();
    cb_list.clear();
  }

  for (auto cb : cbs) {
    cb->Cancel();
  }
}

bool RpcChannelProxy::getCallbackById(uint64 id, ClientCallback** cb) {
  ScopedMutex l(&ctx_.mutex);
  auto im = ctx_.cb_map.find(id);
  if (im == ctx_.cb_map.end()) return false;

  *cb = im->second;
  ctx_.cb_map.erase(im);
  auto il = std::find(ctx_.cb_list.begin(), ctx_.cb_list.end(), *cb);
  if (il != ctx_.cb_list.end()) {
    ctx_.cb_list.erase(il);
  }

  return true;
}

void RpcChannelProxy::CallMethod(
    const google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response, ::google::protobuf::Closure* done) {
  ClientCallback* cb = reinterpret_cast<ClientCallback*>(done);

  uint64 id;
  {
    ScopedMutex l(&ctx_.mutex);
    id = ctx_.id++;
    ctx_.cb_map[id] = cb;
    ctx_.cb_list.push_front(cb);
    cb->SetContext(id, method, request, response);
  }

  io::OutputObject* out_obj = Serialize(id, method->full_name(), *request);
  sender_->send(out_obj);
}

void RpcChannelProxy::checkTimedout(const TimeStamp& time_stamp) {

}

}
