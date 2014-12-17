#ifndef RPC_SERVER_H_
#define RPC_SERVER_H_

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

#include "rpc_def.h"
#include "base/base.h"

namespace io {
class Protocol;
class TcpServer;
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
    // ip: it's caller's responsibility that make sure ip's format is OK.
    // worker: default is 0, means that all events managed by master.
    RpcServer(io::EventManager* ev_mgr, const std::string& ip, uint16 port,
              uint8 worker = 0)
        : ip_(ip), port_(port), ev_mgr_(ev_mgr), worker_(worker) {
    }
    ~RpcServer();

    // hander_map will be released by RpcServer.
    void setHandlerMap(HandlerMap* handler_map);

    // must set handler map first.
    bool start();

  private:
    const std::string ip_;
    uint16 port_;
    uint8 worker_;

    io::EventManager* ev_mgr_;

    scoped_ptr<io::Protocol> protocol_;
    scoped_ptr<HandlerMap> handler_map_;
    scoped_ptr<io::TcpServer> tcp_serv_;

    DISALLOW_COPY_AND_ASSIGN(RpcServer);
};
}
#endif /* RPC_SERVER_H_ */
