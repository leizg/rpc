#ifndef RPC_HELPER_H_
#define RPC_HELPER_H_

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {

class ClientCallback : public ::google::protobuf::Closure {
  public:
    explicit ClientCallback(const TimeStamp& time_stamp)
        : time_stamp_(time_stamp), id_(0) {
      method_ = request_ = response_ = nullptr;
    }
    virtual ~ClientCallback() {
    }

    void reset() {
      time_stamp_ = TimeStamp::now();
    }

    uint64 id() const {
      return id_;
    }

    Message* getRequest() const {
      return request_;
    }
    Message* getResponse() const {
      return response_;
    }

    const MethodDescriptor* getMethod() const {
      return method_;
    }

    void setContext(uint64 req_id, const MethodDescriptor* method,
                    const Message* request, Message* response) {
      id_ = req_id;

      method_ = method;
      request_ = request;
      response_ = response;
    }

    virtual bool isRetry() = 0;
    void Cancel() {
      onCancel();
    }
    virtual void Run() {
      onDone();
    }

  protected:
    uint64 id_;
    SyncEvent sync_evnet_;

    virtual void onDone() {
      sync_evnet_.Signal();
    }
    virtual void onCancel() {
      sync_evnet_.Signal();
    }

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
    }
    virtual void onCancel() {
    }
    virtual bool isRetry() {
      return true;
    }

    DISALLOW_COPY_AND_ASSIGN(SyncCallback);
};

class CancelCallback : public ClientCallback, public RefCounted {
  public:
    CancelCallback(const TimeStamp& time_stamp)
        : ClientCallback(time_stamp), fail_(false) {
    }
    virtual ~CancelCallback();

    // return false iif timedout.
    bool timedWait();

    bool is_failed() const {
      return fail_;
    }

  private:
    bool fail_;

    virtual void onDone() {
      UnRef();
    }
    virtual void onCancel() {
      fail_ = true;
      UnRef();
    }
    virtual bool isRetry() {
      return false;
    }

    DISALLOW_COPY_AND_ASSIGN(CancelCallback);
};

}

#endif /* RPC_HELPER_H_ */
