#pragma once

#include "rpc_processor.h"

namespace rpc {
class HandlerMap;

class RpcServerProcessor : public RpcProcessor::Delegate {
  public:
    explicit RpcServerProcessor(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcServerProcessor() {
    }

  private:
    const HandlerMap* handler_map_;

    virtual void process(async::Connection* conn, io::InputStream* input_stream,
                         const TimeStamp& time_stamp);

    class ReplyClosure : public ::google::protobuf::Closure {
      public:
        ReplyClosure(async::Connection* conn, const MessageHeader& header,
                     Message* reply);
        virtual ~ReplyClosure();

      private:
        const MessageHeader hdr_;
        scoped_ptr<Message> reply_;

        scoped_ref<async::Connection> conn_;

        virtual void Run();

        DISALLOW_COPY_AND_ASSIGN(ReplyClosure);
    };

    DISALLOW_COPY_AND_ASSIGN(RpcServerProcessor);
};
}
