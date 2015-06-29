#pragma once

#include "rpc_def.h"
#include "base/base.h"

namespace async {
class EventManager;
}

namespace rpc {

// RpcClient be called by Service::Stub,
//   you should not call it directly.
class RpcClient : public ::google::protobuf::RpcChannel {
  public:
    virtual ~RpcClient() {
    }

    RpcClient* create(async::EventManager* ev_mgr, const std::string& path);
    RpcClient* create(async::EventManager* ev_mgr, const std::string& ip,
                      uint16 port);

    // not threadsafe.
    // called when connection abort.
    void setCloseClosure(Closure* cb) {
      close_closure_.reset(cb);
    }
    // not threadsafe.
    // called after reconnect successfully.
    void setReconnectClosure(Closure* cb) {
      reconnect_closure_ = cb;
    }

    // return false iif timedout or error orrcurred.
    // time_out: unit: seconds
    // ip: it's caller's responsity that make sure ip's format is good.
    virtual bool connect(uint32 time_out) = 0;

  protected:
    explicit RpcClient(async::EventManager* ev_mgr)
        : ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(ev_mgr);
      reconnect_closure_ = nullptr;
    }

    async::EventManager* ev_mgr_;

    Closure* reconnect_closure_;
    scoped_ptr<Closure> close_closure_;

  private:
    DISALLOW_COPY_AND_ASSIGN(RpcClient);
};

}
