#ifndef RPC_HELPER_H_
#define RPC_HELPER_H_

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {

class ClientCallback : public ::google::protobuf::Closure {
  public:
    explicit ClientCallback(const TimeStamp& time_stamp)
        : time_stamp_(time_stamp), fail_(false), method_(NULL), request_(NULL), response_(
        NULL) {
    }
    virtual ~ClientCallback() {
    }

    void reset() {
      time_stamp_ = TimeStamp::now();
    }
    bool is_failed() const {
      return fail_;
    }

    const MethodDescriptor* getMethod() const {
      return method_;
    }
    Message* getRequest() const {
      return request_;
    }
    Message* getResponse() const {
      return response_;
    }

    void SetContext(const MethodDescriptor* method, const Message* request,
                    Message* response) {
      method_ = method;
      request_ = request;
      response_ = response;
    }

    void SetCancelContext(uint64 req_id);

    virtual void Cancel() = 0;
    virtual void Run() {
      onDone();
      sync_evnet_.Signal();
    }

  protected:
    bool fail_;
    SyncEvent sync_evnet_;

    virtual void onDone() = 0;
    virtual void onCancel() = 0;

  private:
    TimeStamp time_stamp_;

    const MethodDescriptor* method_;
    const Message* request_;
    Message* response_;

    DISALLOW_COPY_AND_ASSIGN(ClientCallback);
};

class SyncCallback : public ClientCallback {
  public:
    SyncCallback(const TimeStamp& time_stamp)
        : ClientCallback(time_stamp) {
    }
    virtual ~SyncCallback();

    void Wait();

  private:
    virtual void onDone() {
      fail_ = false;
    }

    virtual void onCancel() {
      fail_ = true;
    }

    DISALLOW_COPY_AND_ASSIGN(SyncCallback);
};

class CancelCallback : public ClientCallback, public RefCounted {
  public:
    CancelCallback(const TimeStamp& time_stamp)
        : ClientCallback(time_stamp) {
    }

    // return false iif timedout.
    bool TimedWait();

  private:
    virtual ~CancelCallback();

    virtual void onDone();
    virtual void onCancel();

    DISALLOW_COPY_AND_ASSIGN(CancelCallback);
};

}

#endif /* RPC_HELPER_H_ */
