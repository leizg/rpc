#include "rpc_client.h"

#include "async_client.h"
#include "event/event_manager.h"

namespace rpc {

RpcClient::RpcClient(async::EventManager* ev_mgr)
    : ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
#if 0
  protocol_.reset(
      new RpcProtocol(
          new RpcScheduler(new RpcRequestHandler(handler_map),
              new RpcChannelProxy)));
  client_.reset(new io::TcpClient(ev_mgr, ip, port));
  client_->SetProtocol(protocol_.get());
#endif
}

RpcClient::~RpcClient() {
  if (client_ != nullptr) {
    client_->stop();
  }
}

void RpcClient::setHandlerMap(HandlerMap* handler_map) {
  handler_map_.reset(handler_map);
}

bool RpcClient::connect(uint32 time_out) {
  if (!reconnectInternal(time_out)) {
    // TODO:
    return false;
  }

  client_->setCloseClosure(close_closure_.get());
  client_->setReconnectClosure(reconnect_closure_.get());

  return true;
}

void RpcClient::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {

  channel_proxy_->CallMethod(method, controller, request, response, done);
}

void RpcClient::send(io::OutputObject* object) {
  client_->send(object);
}

}

