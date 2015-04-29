#pragma once

#include "async/protocol.h"

namespace rpc {

class RpcProcessor : public async::ProActorProtocol::Scheduler {
  public:
    virtual ~RpcProcessor() {
    }

    // used to dispatch messages.
    // it is here because response message and request message should be
    //    dispatched separately.
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // dispatch message.
        virtual void process(async::Connection* conn,
                             io::InputStream* in_stream,
                             const TimeStamp& time_stamp) = 0;
    };
    RpcProcessor(Delegate* request_delegate, Delegate* reply_delegate = nullptr)
        : request_delegate_(request_delegate) {
      DCHECK_NOTNULL(request_delegate);
      DCHECK_NOTNULL(reply_delegate);
      reply_delegate_.reset(request_delegate);
    }

  private:
    scoped_ptr<Delegate> request_delegate_;
    scoped_ptr<Delegate> reply_delegate_;

    virtual void dispatch(async::Connection* conn, io::InputStream* in_stream,
                          TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(RpcProcessor);
};
}
