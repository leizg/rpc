#pragma once

#include "rpc_scheduler.h"

namespace rpc {
class HandlerMap;

class RpcRequestDispatcher : public RpcScheduler::Delegate {
  public:
    explicit RpcRequestDispatcher(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      DCHECK_NOTNULL(handler_map);
    }
    virtual ~RpcRequestDispatcher() {
    }

  private:
    const HandlerMap* handler_map_;

    virtual void dispatch(async::Connection* conn,
                          io::InputStream* input_stream,
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

    DISALLOW_COPY_AND_ASSIGN(RpcRequestDispatcher);
};

class RpcResponseDispatcher : public RpcScheduler::Delegate {
  public:
    virtual ~RpcResponseDispatcher() {
    }

    class CbFinder {
      public:
        virtual ~CbFinder() {
        }

        virtual bool find(uint64 id, ClientCallback** cb) = 0;
    };
    explicit RpcResponseDispatcher(CbFinder* cb_finder)
        : cb_finder_(cb_finder) {
      DCHECK_NOTNULL(cb_finder);
    }

  private:
    CbFinder* cb_finder_;

    virtual void dispatch(async::Connection* conn,
                          io::InputStream* input_stream,
                          const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcResponseDispatcher);
};
}