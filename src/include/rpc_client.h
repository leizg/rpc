#ifndef RPC_CLIENT_H_
#define RPC_CLIENT_H_

#include "rpc_response_handler.h"

namespace async {
class Protocol;
class TcpClient;
}

namespace rpc {
class HandlerMap;

// todo: multi-tcp-Connection per Channel.
// RpcClient called by Service::Stub.
class RpcClient : public google::protobuf::RpcChannel,
    public RpcClientChannel::Sender {
  public:
    // ev_mgr should have been initialized successfully.
    RpcClient(io::EventManager* ev_mgr, HandlerMap* handler_map,
              const std::string& ip, uint16 port);
    virtual ~RpcClient();

    // return false iif timedout or error orrcurred.
    // time_out: unit: seconds
    // ip: it's caller's responsity that make sure ip is good format.
    bool Connect(uint32 time_out);

    // not threadsafe.
    // cb will be released by RpcClient.
    void SetReconnectClosure(Closure* cb) {
      reconnect_closure_.reset(cb);
    }

  private:
    io::EventManager* ev_mgr_;

    scoped_ptr<io::TcpClient> client_;
    scoped_ptr<io::Protocol> protocol_;

    scoped_ptr<google::protobuf::RpcChannel> channel_impl_;

    // by google::protobuf::RpcChannel.
    // called by Service::Stub.
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller, const Message* request,
                            Message* response, google::protobuf::Closure* done);

    virtual void send(io::OutputObject* object);

    void Reconnect();
    // reconnect_closure_ will be called after tcp reconnect successfully.
    scoped_ptr<Closure> reconnect_closure_;

    DISALLOW_COPY_AND_ASSIGN(RpcClient);
};
}
#endif /* RPC_CLIENT_H_ */
