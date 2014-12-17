#ifndef RPC_PROCESSOR_H_
#define RPC_PROCESSOR_H_

//#include "rpc_def.h"
#include "io/protocol.h"

namespace rpc {

class RpcProcessor : public io::Protocol::Processor {
  public:
    // used to dispatch messages.
    // it is here because response message and request message should be
    //    dispatched separately.
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        // dispatch message.
        virtual void process(io::Connection* conn, io::InputBuf* input_buf,
                             const TimeStamp& time_stamp) = 0;
    };

    explicit RpcProcessor(Delegate* delegate)
        : delegate_(delegate) {
      DCHECK_NOTNULL(delegate);
    }
    virtual ~RpcProcessor() {
    }

  private:
    scoped_ptr<Delegate> delegate_;

    virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp) {
      delegate_->process(conn, input_buf, time_stamp);
    }

    DISALLOW_COPY_AND_ASSIGN(RpcProcessor);
};
}
#endif /* RPC_PROCESSOR_H_ */
