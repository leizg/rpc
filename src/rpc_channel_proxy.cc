#include "rpc_protocol.h"
#include "rpc_channel_proxy.h"

#include "io/io_buf.h"

namespace rpc {

void RpcChannelProxy::Serializer::serialize(const Message& msg) {
  chunk_.reset(new io::ExternableChunk(RPC_HEADER_LENGTH + msg.ByteSize()));
  chunk_->ensureLeft(RPC_HEADER_LENGTH + msg.ByteSize());

  MessageHeader* hdr = reinterpret_cast<MessageHeader*>(chunk_->peekW());
  chunk_->skipWrite(RPC_HEADER_LENGTH);
  hdr->id = id_;
  hdr->fun_id = SuperFastHash(func_);
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

void RpcChannelProxy::destory() {
  if (timer_ != nullptr) {
    timer_->stop();
    timer_.reset();
  }

  CallbackList cbs;
  {
    ScopedMutex l(&mutex_);
    cbs.swap(cb_list_);
    cb_map_.clear();
    cb_list_.clear();
  }

  for (auto cb : cbs) {
    cb->Cancel();
  }
}

uint64 RpcChannelProxy::push(ClientCallback* cb) {
  ScopedMutex l(&mutex_);
  if (!cb->isRetry()) {
    auto cc = reinterpret_cast<CancelCallback*>(cb);
    cc->Ref();
  }

  cb_map_[id_] = cb;
  cb_list_.push_front(cb);
  return id_++;
}

ClientCallback* RpcChannelProxy::release(uint64 id) {
  ScopedMutex l(&mutex_);
  auto it = cb_map_.find(id);
  if (it == cb_map_.end()) {
    return nullptr;
  }

  auto cb = it->second;
  cb_map_.erase(it);
  for (auto it = cb_list_.begin(); it != cb_list_.end(); ++it) {
    if ((*it)->id() == id) {
      cb_list_.erase(it);
      break;
    }
  }

  return cb;
}

void RpcChannelProxy::CallMethod(const MethodDescriptor* method,
                                 RpcController* controller,
                                 const Message* request, Message* response,
                                 ::google::protobuf::Closure* done) {
  ClientCallback* cb = reinterpret_cast<ClientCallback*>(done);
  uint64 id = push(cb);
  cb->setContext(id, method, request, response);
  sendCallback(cb);
}

void RpcChannelProxy::sendCallback(ClientCallback* cb) {
  auto obj = new Serializer(cb->id(), cb->getMethod()->full_name());
  obj->serialize(cb->getRequest());
  sender_->send(new io::OutVectorObject(obj));
}

void RpcChannelProxy::firedTimedoutCbs(const TimeStamp& now,
                                       std::vector<ClientCallback*>* cbs) {
  while (true) {
    auto it = cb_list_.begin();
    auto& cb = *it;
    if (!cb->isTimedout(now)) {
      break;
    }

    cb_list_.erase(it);
    cbs->push_back(cb);
  }
}

void RpcChannelProxy::checkTimedout(const TimeStamp& time_stamp) {
  ScopedMutex l(&mutex_);
  std::vector<ClientCallback*> cbs;
  firedTimedoutCbs(time_stamp, &cbs);
  for (auto& cb : cbs) {
    if (!cb->isRetry()) {
      cb_map_.erase(cb->id());
      cb->Cancel();
      continue;
    }

    cb->reset();
    cb_list_.push_back(cb);
    sendCallback(cb);
  }
}

}
