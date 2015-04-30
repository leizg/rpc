#pragma once

#include "rpc_scheduler.h"

namespace io {
class EventManager;
class OutputObject;
}

namespace rpc {

class RpcClientChannel : public RpcScheduler::Delegate,
    public google::protobuf::RpcChannel {
  public:
    class Sender {
      public:
        virtual ~Sender() {
        }

        virtual void send(io::OutputObject* object) = 0;
    };
    explicit RpcClientChannel(Sender* sender);
    virtual ~RpcClientChannel();

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

    // handle response.
    virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                         const TimeStamp& time_stamp);
    // handle request.
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller, const Message* request,
                            Message* response, google::protobuf::Closure* done);

    void checkTimedout(const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN (RpcClientChannel);
};
}
