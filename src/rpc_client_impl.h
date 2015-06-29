#pragma once

#include "include/rpc_client.h"
#include "rpc_channel_proxy.h"

namespace async {
class AsyncClient;
}

namespace rpc {

class RpcClientImpl : public RpcClient, public RpcChannelProxy::Sender {
  public:
    virtual ~RpcClientImpl();

  protected:
    // ev_mgr should have been initialized successfully.
    explicit RpcClientImpl(async::EventManager* ev_mgr);

    virtual bool reconnectInternal(uint32 timeout) = 0;

  private:
    Mutex mutex_;
    scoped_ptr<async::Protocol> protocol_;
    scoped_ptr<async::AsyncClient> client_;

    // by RpcChannelProxy::Sender.
    void send(io::OutputObject* object);

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

class RpcLocalClient : public RpcClient {
  public:
    RpcLocalClient(async::EventManager* ev_mgr, const std::string& path)
        : RpcClient(ev_mgr), path_(path) {
      DCHECK(!path.empty());
    }
    virtual ~RpcLocalClient() {
    }

    const std::string& path() {
      return path_;
    }

  private:
    const std::string path_;

    virtual bool reconnectInternal(uint32 timeout);

    DISALLOW_COPY_AND_ASSIGN (RpcLocalClient);
};

class RpcTcpClient : public RpcClient {
  public:
    RpcTcpClient(async::EventManager* ev_mgr, const std::string& ip,
                 uint16 port)
        : RpcClient(ev_mgr), ip_(ip) {
      DCHECK(!ip.empty());
      port_ = port;
    }
    virtual ~RpcTcpClient() {
    }

    const std::string& ip() const {
      return ip_;
    }
    uint16 port() const {
      return port_;
    }

  private:
    const std::string ip_;
    uint16 port_;

    virtual bool reconnectInternal(uint32 timeout);

    DISALLOW_COPY_AND_ASSIGN (RpcTcpClient);
};

}
