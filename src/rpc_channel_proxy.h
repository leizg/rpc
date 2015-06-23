#pragma once

#include "rpc_scheduler.h"
#include "async/event/timer.h"

namespace io {
class OutputObject;
}
namespace async {
class EventManager;
}

namespace rpc {

class RpcChannelProxy : public ::google::protobuf::RpcChannel,
    public RpcResponseScheduler::CbContext {

  public:
    virtual ~RpcChannelProxy() {
      if (timer_ != nullptr) {
        timer_->stop();
        timer_.reset();
      }
    }

    class Sender {
      public:
        virtual ~Sender() {
        }

        virtual void send(io::OutputObject* object) = 0;
    };
    RpcChannelProxy(Sender* sender, async::EventManager* ev_mgr)
        : sender_(sender), ev_mgr_(ev_mgr) {
      DCHECK_NOTNULL(sender);
      DCHECK_NOTNULL(ev_mgr);
      init();
    }

  private:
    Sender* sender_;
    async::EventManager* ev_mgr_;

    struct RpcContext {
        RpcContext()
            : id(1) {
        }
        ~RpcContext();

        uint64 push(ClientCallback* cb);
        ClientCallback* get(uint64 id);

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

    void sendCallback(ClientCallback* cb);

    void init();
    void firedTimedoutCbs(const TimeStamp& now,
                          std::vector<ClientCallback*>* cbs);
    void checkTimedout(const TimeStamp& time_stamp);
    scoped_ptr<async::RepeatTimer> timer_;

    DISALLOW_COPY_AND_ASSIGN (RpcChannelProxy);
};
}
