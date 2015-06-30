#pragma once

#include "rpc_scheduler.h"
#include "async/event/timer.h"

namespace io {
class OutputObject;
}

namespace rpc {

class RpcChannelProxy : public RpcResponseScheduler::Delegate {
  public:
    virtual ~RpcChannelProxy() {
      destory();
    }

    // threadsafe, can be called from any thread.
    // same as ::google::protobuf::RpcChannel.
    // maybe better that inherit from RpcChannel.
    void CallMethod(const MethodDescriptor* method, RpcController* controller,
                    const Message* request, Message* response,
                    ::google::protobuf::Closure* done);

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
      id_ = 1;
    }

  private:
    Sender* sender_;
    async::EventManager* ev_mgr_;

    void destory();
    void init() {
      timer_.reset(
          new async::RepeatTimer(
              3 * TimeStamp::kMicroSecsPerSecond, ev_mgr_,
              NewPermanentCallback(this, &RpcChannelProxy::checkTimedout)));
      timer_->start();
    }

    Mutex mutex_;

    uint64 id_;
    typedef std::map<uint64, ClientCallback*> CallbackMap;
    CallbackMap cb_map_;
    typedef std::list<ClientCallback*> CallbackList;
    CallbackList cb_list_;

    uint64 push(ClientCallback* cb);
    virtual ClientCallback* release(uint64 id);

    void firedTimedoutCbs(const TimeStamp& now,
                          std::vector<ClientCallback*>* cbs);
    void checkTimedout(const TimeStamp& time_stamp);
    scoped_ptr<async::RepeatTimer> timer_;

    void sendCallback(ClientCallback* cb);
    class Serializer : public io::OutVectorObject::IoObject {
      public:
        Serializer(uint64 id, const std::string& func)
            : id_(id), func_(func) {
          DCHECK(!func.empty());
        }
        virtual ~Serializer() {
        }

        void serialize(const Message& request);

      private:
        uint64 id_;
        const std::string func_;

        std::vector<iovec> ios_;
        virtual const std::vector<iovec>& ioVec() const {
          return ios_;
        }

        scoped_ptr<io::ExternableChunk> chunk_;

        DISALLOW_COPY_AND_ASSIGN(Serializer);
    };

    DISALLOW_COPY_AND_ASSIGN(RpcChannelProxy);
};

}
