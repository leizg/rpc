#include "rpc_client.h"
#include "async_client.h"
#include "event/event_manager.h"

namespace rpc {

RpcClient::RpcClient(async::EventManager* ev_mgr)
    : ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
  proxy_.reset(new RpcChannelProxy(this, ev_mgr));
  protocol_.reset(new RpcProtocol(new RpcResponseScheduler(proxy_.get())));
}

RpcClient::~RpcClient() {
  ScopedMutex m(&mutex_);
  if (client_ != nullptr) {
    client_->stop();
  }
}

bool RpcClient::connect(uint32 time_out) {
  ScopedMutex m(&mutex_);
  if (!reconnectInternal(time_out)) {
    // TODO:
    return false;
  }

  client_->setProtocol(protocol_.get());
  client_->setCloseClosure(close_closure_.get());
  client_->setReconnectClosure(reconnect_closure_.get());

  return true;
}

void RpcClient::send(io::OutputObject* object) {
  ScopedMutex m(&mutex_);
  if (client_ == nullptr) {
    delete object;
    return;
  }

  client_->send(object);
}

bool RpcTcpClient::reconnectInternal(uint32 timeout) {
  client_.reset(new async::TcpAsyncClient(ev_mgr_, ip_, port_));
  if (!client_->connect(timeout)) {
    client_->stop();
    client_.reset();
    return false;
  }

  return true;
}

bool RpcLocalClient::reconnectInternal(uint32 timeout) {
  client_.reset(new async::LocalAsyncClient(ev_mgr_, path_));
  if (!client_->connect(timeout)) {
    client_->stop();
    client_.reset();
    return false;
  }

  return true;
}

}

