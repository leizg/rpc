#pragma once

#include "rpc_channel_proxy.h"
#include "include/rpc_client.h"

namespace async {
class AsyncClient;
}

namespace rpc {

class RpcClientImpl : public RpcClient, public RpcChannelProxy::Sender {
  public:
    virtual ~RpcClientImpl() {
      stop();
    }

    void stop();

  protected:
    explicit RpcClientImpl(async::EventManager* ev_mgr);

  private:
    Mutex mutex_;
    scoped_ptr<async::Protocol> protocol_;
    scoped_ptr<async::AsyncClient> client_;

    // by RpcChannelProxy::Sender.
    virtual void send(io::OutputObject* object);

    // by RpcClient.
    virtual bool connect(uint32 time_out);

    void onAbort();
    virtual bool connectInternal(uint32 timeout) = 0;

    // by google::protobuf::RpcChannel.
    // called by Service::Stub.
    scoped_ptr<RpcChannelProxy> proxy_;
    virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done) {
      DCHECK_NOTNULL(proxy_.get());
      proxy_->CallMethod(method, controller, request, response, done);
    }

    DISALLOW_COPY_AND_ASSIGN(RpcClientImpl);
};
}
