#ifndef RPC_SERVER_PROCESSOR_H_
#define RPC_SERVER_PROCESSOR_H_

#include "rpc_processor.h"

namespace rpc {
class HandlerMap;

// TODO: 1. retransmit rpc.
class RpcServerProcessor : public RpcProcessor::Delegate {
  public:
    explicit RpcServerProcessor(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcServerProcessor();

  private:
    HandlerMap* handler_map_;

    virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                         const TimeStamp& time_stamp);

    class ReplyClosure : public ::google::protobuf::Closure {
      public:
        ReplyClosure(io::Connection* conn, const MessageHeader& header,
                     Message* reply);
        virtual ~ReplyClosure();

      private:
        const MessageHeader hdr_;
        scoped_ptr<Message> reply_;

        scoped_ref<io::Connection> conn_;

        virtual void Run();

        DISALLOW_COPY_AND_ASSIGN(ReplyClosure);
    };

    DISALLOW_COPY_AND_ASSIGN(RpcServerProcessor);
};
}
#endif  // RPC_SERVER_PROCESSOR_H_
