#include "rpc_client.h"
#include "rpc_response_handler.h"

#include "io/tcp_client.h"
#include "io/event_manager.h"

namespace rpc {

RpcClient::RpcClient(io::EventManager* ev_mgr, HandlerMap* handler_map,
                     const std::string& ip, uint16 port)
    : ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
  protocol_.reset(
      new RpcProtocol(
          new RpcProcessor(new RpcRequestHandler(handler_map),
                           new RpcClientChannel)));
  client_.reset(new io::TcpClient(ev_mgr, ip, port));
  client_->SetProtocol(protocol_.get());
}

RpcClient::~RpcClient() {
  client_->SetCloseClosure(NULL);
  client_.reset();
}

bool RpcClient::Connect(uint32 time_out) {
  if (client_->IsConnected()) return true;
  if (!client_->Connect(time_out)) {
    // TODO:
    return false;
  }

  client_->SetCloseClosure(::NewPermanentCallback(this, &RpcClient::Reconnect));
  return true;
}

void RpcClient::CallMethod(const MethodDescriptor* method,
                           RpcController* controller, const Message* request,
                           Message* response, google::protobuf::Closure* done) {
  if (client_.get() == NULL || !client_->IsConnected()) {
    // TODO: cancel rpc directly.
    return;
  }

  channel_impl_->CallMethod(method, controller, request, response, done);
}

void RpcClient::send(io::OutputObject* object) {
  if (client_.get() != NULL && client_->IsConnected()) {
    client_->Send(object);
  }
}

void RpcClient::Reconnect() {
  while (!client_->IsConnected()) {
    if (!client_->Connect(3)) {
      continue;
    }

    LOG(WARNING)<<"reconnect master successfully";
    if (reconnect_closure_ != NULL) {
      reconnect_closure_->Run();
    }
    break;
  }
}
}

