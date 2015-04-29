#include "rpc_protocol.h"
#include "include/rpc_server.h"
#include "include/handler_map.h"

#include "async/async_server.h"
#include "async/event/event_manager.h"

namespace rpc {

RpcServer::~RpcServer() {
}

void RpcServer::setHandlerMap(HandlerMap* handler_map) {
  DCHECK_NOTNULL(handler_map);
  handler_map_.reset(handler_map);
}

bool RpcServer::start(int server) {
  DCHECK_NOTNULL(handler_map_.get());
  DCHECK_NOTNULL(protocol_.get());
  protocol_.reset(
      new RpcProtocol(
          new RpcProcessor(new RpcServerProcessor(handler_map_.get()))));

  serv_.reset(new async::AsyncServer(ev_mgr_, server, worker_));
  serv_->setProtocol(protocol_.get());
  if (!serv_->init()) {
    fail: protocol_.reset();
    serv_.reset();
    return false;
  }

  return true;
}

void RpcServer::stop() {
  if (serv_ != nullptr) serv_->stop();
}
}
