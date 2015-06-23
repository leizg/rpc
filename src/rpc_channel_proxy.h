#pragma once

#include "rpc_dispatcher.h"

namespace async {
class EventManager;
class OutputObject;
}

namespace rpc {

// todo: handle timeout and retry...
class RpcChannelProxy : public ::google::protobuf::RpcChannel,
    public RpcResponseScheduler::CbContext {

  public:
    class Sender {
      public:
        virtual ~Sender() {
        }

        virtual void send(io::OutputObject* object) = 0;
    };
    explicit RpcChannelProxy(Sender* sender)
        : sender_(sender) {
      DCHECK_NOTNULL(sender);
    }
    virtual ~RpcChannelProxy();

  private:
    Sender* sender_;

    struct RpcContext {
        RpcContext()
            : id(1) {
        }
        ~RpcContext();

        uint64 id;
        Mutex mutex;

        typedef std::map<uint64, ClientCallback*> CallbackMap;
        CallbackMap cb_map;

        typedef std::list<ClientCallback*> CallbackList;
        CallbackList cb_list;
    };

    RpcContext ctx_;

    virtual bool getCallbackById(uint64 id, ClientCallback** cb);
    virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

    // todo: add timer.
    void checkTimedout(const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN (RpcChannelProxy);
};
}
