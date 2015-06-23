#pragma once

#include "async/protocol.h"

namespace rpc {
class HandlerMap;

class RpcRequestScheduler : public async::ProActorProtocol::Scheduler {
  public:
    explicit RpcRequestScheduler(const HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcRequestScheduler() {
    }

  private:
    const HandlerMap* handler_map_;

    virtual void dispatch(async::Connection* conn, io::InputStream* in_stream,
                          TimeStamp time_stamp);

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

    DISALLOW_COPY_AND_ASSIGN(RpcRequestScheduler);
};

class RpcResponseScheduler : public async::ProActorProtocol::Scheduler {
  public:
    virtual ~RpcResponseScheduler() {
    }

    class CbContext {
      public:
        virtual ~CbContext() {
        }

        virtual bool getCallbackById(uint64 id, ClientCallback** cb) = 0;
    };
    explicit RpcResponseScheduler(CbContext* cb_ctx)
        : cb_ctx_(cb_ctx) {
      DCHECK_NOTNULL(cb_ctx);
    }

  private:
    CbContext* cb_ctx_;

    virtual void dispatch(async::Connection* conn,
                          io::InputStream* input_stream,
                          const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcResponseScheduler);
};
}
