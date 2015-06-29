#include "rpc_client_impl.h"

#include "async_client.h"
#include "event/event_manager.h"

namespace {

class RpcLocalClient : public rpc::RpcClientImpl {
  public:
    RpcLocalClient(async::EventManager* ev_mgr, const std::string& path)
        : rpc::RpcClient(ev_mgr), path_(path) {
      DCHECK(!path.empty());
    }
    virtual ~RpcLocalClient() {
    }

    const std::string& path() {
      return path_;
    }

  private:
    const std::string path_;

    virtual bool connectInternal(uint32 timeout) {
      client_.reset(new async::LocalAsyncClient(ev_mgr_, path_));
      if (!client_->connect(timeout)) {
        client_->stop();
        client_.reset();
        return false;
      }
      return true;
    }

    DISALLOW_COPY_AND_ASSIGN (RpcLocalClient);
};

class RpcTcpClient : public rpc::RpcClientImpl {
  public:
    RpcTcpClient(async::EventManager* ev_mgr, const std::string& ip,
                 uint16 port)
        : rpc::RpcClient(ev_mgr), ip_(ip) {
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

    virtual bool connectInternal(uint32 timeout) {
      client_.reset(new async::TcpAsyncClient(ev_mgr_, ip_, port_));
      if (!client_->connect(timeout)) {
        client_->stop();
        client_.reset();
        return false;
      }
      return true;
    }

    DISALLOW_COPY_AND_ASSIGN (RpcTcpClient);
};
}

namespace rpc {

bool RpcClientImpl::connect(uint32 time_out) {
  ScopedMutex m(&mutex_);
  if (!connectInternal(time_out)) {
    return false;
  }

  client_->setProtocol(protocol_.get());
  client_->setCloseClosure(::NewCallback(this, &RpcClient::onAbort));
  client_->setReconnectClosure(reconnect_closure_);

  return true;
}

void RpcClientImpl::onAbort() {
  {
    ScopedMutex m(&mutex_);
    client_.reset();
  }

  if (close_closure_ != nullptr) {
    close_closure_->Run();
  }
}

RpcClient* RpcClient::create(async::EventManager* ev_mgr,
                             const std::string& path) {
  return new RpcLocalClient(ev_mgr, path);
}

RpcClient* RpcClient::create(async::EventManager* ev_mgr, const std::string& ip,
                             uint16 port) {
  return new RpcTcpClient(ev_mgr, ip, port);
}

// rpc client implemention.
RpcClientImpl::RpcClientImpl(async::EventManager* ev_mgr)
    : RpcClient(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);

  proxy_.reset(new RpcChannelProxy(this, ev_mgr));
  // it is here because maybe you want be initialize it here.
  // and it will be released by RpcProtocol.
  auto scheluder = new RpcResponseScheduler(proxy_.get());
  protocol_.reset(new RpcProtocol(scheluder));
}

void RpcClientImpl::stop() {
  ScopedMutex m(&mutex_);
  if (client_ != nullptr) {
    client_->stop();
    client_.reset();
  }
}

void RpcClientImpl::send(io::OutputObject* object) {
  ScopedMutex m(&mutex_);
  if (client_ == nullptr) {
    delete object;
    return;
  }

  client_->send(object);
}

}

