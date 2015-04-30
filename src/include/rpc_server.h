#pragma once

#include "rpc_def.h"
#include "base/base.h"

namespace async {
class Protocol;
class AsyncServer;
class EventManager;
}

namespace rpc {
class HandlerMap;

// thread model: master + workers.
//              one (and only one) event loop per thread.
// master accept new connection and dispach it to work thread.
class RpcServer {
  public:
    // ev_mgr must be initialized first.
    // worker: default is 0, means that all events managed by master.
    RpcServer(async::EventManager* ev_mgr, uint8 worker = 0)
        : worker_(worker) {
      ev_mgr_ = ev_mgr;
    }
    ~RpcServer();

    // hander_map will be released by RpcServer.
    void setHandlerMap(HandlerMap* handler_map);

    // must set handler map first.
    // server: listen fd. see async_server.h
    bool start(int server);
    void stop();

  private:
    uint8 worker_;
    async::EventManager* ev_mgr_;

    scoped_ptr<HandlerMap> handler_map_;
    scoped_ptr<async::Protocol> protocol_;
    scoped_ptr<async::AsyncServer> serv_;

    DISALLOW_COPY_AND_ASSIGN(RpcServer);
};
}
