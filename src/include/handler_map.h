#ifndef HANDLER_MAP_H_
#define HANDLER_MAP_H_

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {

class MethodHandler {
  public:
    MethodHandler()
        : service(nullptr), method(nullptr), request(nullptr), reply(nullptr) {
    }
    ~MethodHandler() {
      delete request;
      delete reply;
    }

    Service* service;  // shouldn't delete service.
    const MethodDescriptor* method;  // shouldn't delete method.

    const Message* request;
    const Message* reply;

  private:
    DISALLOW_COPY_AND_ASSIGN(MethodHandler);
};

class HandlerMap {
  public:
    explicit HandlerMap(Service* serv = nullptr) {
      if (serv != nullptr) {
        AddService(serv);
      }
    }
    ~HandlerMap() {
      STLMapClear(&serv_map_);
    }

    bool AddService(Service* serv);

    MethodHandler* FindMehodById(uint32 id) const {
      auto it = serv_map_.find(id);
      if (it != serv_map_.end()) {
        return it->second;
      }
      return NULL;
    }

  private:
    typedef std::map<uint32, MethodHandler*> ServMap;

    bool AddHandler(uint32 hash_id, MethodHandler* handler) {
      MethodHandler* old = FindMehodById(hash_id);
      if (old != NULL) return false;

      return serv_map_.insert(std::make_pair(hash_id, handler)).second;
    }

    ServMap serv_map_;

    DISALLOW_COPY_AND_ASSIGN(HandlerMap);
};
}
#endif /* HANDLER_MAP_H_ */
