#pragma once

#include "rpc_dispatcher.h"

namespace async {
class EventManager;
class OutputObject;
}

namespace rpc {

class RpcChannelProxy : public ::google::protobuf::RpcChannel,
    public RpcResponseDispatcher::CbFinder {

  public:
    virtual ~RpcChannelProxy();

    // handle request.
    virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

    class Sender {
      public:
        virtual ~Sender() {
        }

        virtual void send(io::OutputObject* object) = 0;
    };
    explicit RpcChannelProxy(Sender* sender);

  private:
    Sender* sender_;

    uint64 id_;
    Mutex mutex_;
    typedef std::map<uint64, ClientCallback*> CallbackMap;
    CallbackMap call_back_map_;

    class Serializer {
      public:
        virtual ~Serializer() {
        }

        io::OutputObject* Serialize(uint64 id, const std::string& fun_name,
                                    const Message& msg) const = 0;
    };
    scoped_ptr<Serializer> serializer_;

    virtual bool find(uint64 id, ClientCallback** cb);

    void checkTimedout(const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN (RpcChannelProxy);
};
}
