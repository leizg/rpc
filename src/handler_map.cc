#include "include/handler_map.h"
#include <google/protobuf/descriptor.h>

namespace rpc {

bool HandlerMap::registeHandler(uint32 hash_id, MethodHandler* handler) {
  MethodHandler* old = findMehodById(hash_id);
  if (old != nullptr) {
    LOG(WARNING)<< "exist handler, name: " << handler->method->DebugString();
    return false;
  }

  return serv_map_.insert(std::make_pair(hash_id, handler)).second;
}

bool HandlerMap::addService(Service* serv) {
  const ServiceDescriptor* const serv_desc = serv->GetDescriptor();
  int method_count = serv_desc->method_count();
  for (int i = 0; i < method_count; ++i) {
    auto method_desc = serv_desc->method(i);

    MethodHandler* handler = new MethodHandler(serv, method_desc);
    handler->request.reset(serv->GetRequestPrototype(method_desc).New());
    handler->reply.reset(serv->GetResponsePrototype(method_desc).New());

    auto& method_name = method_desc->full_name();
    uint32 hash_id = SuperFastHash(method_name);
    if (!registeHandler(hash_id, handler)) {
      LOG(WARNING)<<"duplicate handler, name: " << method_name;
      delete handler;
      return false;
    }
  }

  return true;
}

}
