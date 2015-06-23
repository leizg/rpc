#pragma once

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {

class MethodHandler {
  public:
    MethodHandler(Service* s, const MethodDescriptor* m)
        : service(s), method(m) {
    }
    ~MethodHandler() {
    }

    // shouldn't delete service and method.
    Service* service;
    const MethodDescriptor* method;

    scoped_ptr<Message> request;
    scoped_ptr<Message> reply;

  private:
    DISALLOW_COPY_AND_ASSIGN(MethodHandler);
};

// no need threadsafe.
// must add service as soon as possible.
class HandlerMap {
  public:
    explicit HandlerMap(Service* serv = nullptr) {
      if (serv != nullptr) {
        addService(serv);
      }
    }
    ~HandlerMap() {
      STLMapClear(&serv_map_);
    }

    bool addService(Service* serv);

    MethodHandler* findMehodById(uint32 id) const {
      auto it = serv_map_.find(id);
      if (it != serv_map_.end()) {
        return it->second;
      }
      return nullptr;
    }

  private:
    typedef std::map<uint32, MethodHandler*> ServMap;
    ServMap serv_map_;

    bool registeHandler(uint32 hash_id, MethodHandler* handler);

    DISALLOW_COPY_AND_ASSIGN(HandlerMap);
};
}
