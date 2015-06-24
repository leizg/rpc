#pragma once

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {

class ClientCallback : public ::google::protobuf::Closure {
  public:
    ClientCallback(bool is_retry)
        : id_(0), fail_(false), is_retry_(is_retry) {
      time_stamp_ = TimeStamp::now();
      method_ = request_ = response_ = nullptr;
    }
    virtual ~ClientCallback() {
    }

    bool isRetry() const {
      return is_retry_;
    }
    uint64 id() const {
      return id_;
    }

    void reset() {
      time_stamp_ = TimeStamp::now();
    }
    const TimeStamp& timestamp() const {
      return time_stamp_;
    }

    bool isTimedout(const TimeStamp& now) const;

    const Message& getRequest() const {
      return *request_;
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

    bool is_failed() const {
      return fail_;
    }

    void wait() {
      sync_evnet_.Wait();
    }
    void Cancel() {
      onCancel();
    }
    virtual void Run() {
      onDone();
    }

  protected:
    uint64 id_;
    bool fail_;
    bool is_retry_;
    SyncEvent sync_evnet_;

    virtual void onDone() {
      sync_evnet_.Signal();
    }
    virtual void onCancel() {
      fail_ = true;
      sync_evnet_.Signal();
    }

  private:
    TimeStamp time_stamp_;
    const MethodDescriptor* method_;
    const Message* request_;
    Message* response_;

    DISALLOW_COPY_AND_ASSIGN(ClientCallback);
};

inline bool ClientCallback::isTimedout(const TimeStamp& now) const {
  return now - time_stamp_ > 1000 * TimeStamp::kMilliSecsPerSecond;
}

class SyncCallback : public ClientCallback {
  public:
    SyncCallback()
        : ClientCallback(true) {
    }
    virtual ~SyncCallback() {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(SyncCallback);
};

class CancelCallback : public ClientCallback, public RefCounted {
  public:
    CancelCallback()
        : ClientCallback(false) {
    }
    virtual ~CancelCallback() {
      delete request_;
      delete response_;
    }

    // return false iif timedout.
    bool timedWait(uint32 timedout) {
      return sync_evnet_.TimeWait(timedout);
    }

  private:
    virtual void onDone() {
      ClientCallback::onDone();
      UnRef();
    }
    virtual void onCancel() {
      ClientCallback::onCancel();
      UnRef();
    }

    DISALLOW_COPY_AND_ASSIGN(CancelCallback);
};

}

