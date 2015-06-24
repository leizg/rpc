#include "rpc_protocol.h"
#include "rpc_channel_proxy.h"

#include "io/io_buf.h"

namespace {

class RequestObject : public io::OutVectorObject::IoObject {
  public:
    RequestObject() {
    }
    virtual ~RequestObject() {
    }

    void serialize(uint64 id, const std::string& func_name,
                   const Message& request);

  private:
    virtual const std::vector<iovec>& ioVec() const {
      return ios_;
    }

    std::vector<iovec> ios_;
    scoped_ptr<io::ExternableChunk> chunk_;

    DISALLOW_COPY_AND_ASSIGN(RequestObject);
};

void RequestObject::serialize(uint64 id, const std::string& func_name,
                              const Message& msg) {
  chunk_.reset(new io::ExternableChunk(RPC_HEADER_LENGTH + msg.ByteSize()));
  chunk_->ensureLeft(RPC_HEADER_LENGTH + msg.ByteSize());

  MessageHeader* hdr = reinterpret_cast<MessageHeader*>(chunk_->peekW());
  chunk_->skipWrite(RPC_HEADER_LENGTH);
  hdr->id = id;
  hdr->fun_id = SuperFastHash(func_name);
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

uint64 RpcChannelProxy::RpcContext::push(ClientCallback* cb) {
  ScopedMutex l(&mutex);
  if (!cb->isRetry()) {
    auto cc = reinterpret_cast<CancelCallback*>(cb);
    cc->Ref();
  }

  cb_map[id] = cb;
  cb_list.push_front(cb);
  return id++;
}

ClientCallback* RpcChannelProxy::RpcContext::get(uint64 id) {
  ScopedMutex l(&mutex);
  auto it = cb_map.find(id);
  if (it == cb_map.end()) {
    return nullptr;
  }

  auto cb = it->second;
  cb_map.erase(it);
  for (auto it = cb_list.begin(); it != cb_list.end(); ++it) {
    if ((*it)->id() == id) {
      cb_list.erase(it);
      break;
    }
  }

  return cb;
}

bool RpcChannelProxy::getCallbackById(uint64 id, ClientCallback** cb) {
  *cb = ctx_.get(id);
  return *cb != nullptr;
}

void RpcChannelProxy::CallMethod(
    const google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response, ::google::protobuf::Closure* done) {
  ClientCallback* cb = reinterpret_cast<ClientCallback*>(done);
  uint64 id = ctx_.push(cb);
  cb->setContext(id, method, request, response);
  sendCallback(cb);
}

void RpcChannelProxy::init() {
  timer_.reset(
      new async::RepeatTimer(
          3 * TimeStamp::kMicroSecsPerSecond, ev_mgr_,
          NewPermanentCallback(this, &RpcChannelProxy::checkTimedout)));
  timer_->start();
}

void RpcChannelProxy::sendCallback(ClientCallback* cb) {
  auto req_obj = new RequestObject;
  req_obj->serialize(cb->id(), cb->getMethod()->full_name(), cb->getRequest());
  sender_->send(new io::OutVectorObject(req_obj));
}

void RpcChannelProxy::firedTimedoutCbs(const TimeStamp& now,
                                       std::vector<ClientCallback*>* cbs) {
  auto& cb_list = ctx_.cb_list;
  while (true) {
    auto it = cb_list.begin();
    auto& cb = *it;
    if (!cb->isTimedout(now)) {
      break;
    }

    cb_list.erase(it);
    cbs->push_back(cb);
  }
}

void RpcChannelProxy::checkTimedout(const TimeStamp& time_stamp) {
  auto& cb_list = ctx_.cb_list;
  auto& cb_map = ctx_.cb_map;

  ScopedMutex l(&ctx_.mutex);
  std::vector<ClientCallback*> cbs;
  firedTimedoutCbs(time_stamp, &cbs);
  for (auto& cb : cbs) {
    if (!cb->isRetry()) {
      cb_map.erase(cb->id());
      cb->Cancel();
      continue;
    }

    cb->reset();
    cb_list.push_back(cb);
    sendCallback(cb);
  }
}

}
