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

    class Delegate {
      public:
        virtual ~Delegate() {
        }

        virtual ClientCallback* release(uint64 id) = 0;
    };
    explicit RpcResponseScheduler(Delegate* delegate)
        : delegate_(delegate) {
      DCHECK_NOTNULL(delegate);
    }

  private:
    Delegate* delegate_;

    virtual void dispatch(async::Connection* conn,
                          io::InputStream* input_stream, TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcResponseScheduler);
};
}
