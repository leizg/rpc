#include "include/handler_map.h"
#include <google/protobuf/descriptor.h>

namespace rpc {

bool HandlerMap::AddService(Service* serv) {
  const ServiceDescriptor* const serv_desc = serv->GetDescriptor();
  int method_count = serv_desc->method_count();
  for (int i = 0; i < method_count; ++i) {
    const MethodDescriptor* const method_desc = serv_desc->method(i);

    MethodHandler* handler = new MethodHandler;
    handler->service = serv;
    handler->method = method_desc;
    handler->request = serv->GetRequestPrototype(method_desc).New();
    handler->reply = serv->GetResponsePrototype(method_desc).New();

    auto& method_name = method_desc->full_name();
    uint32 hash_id = Hash(method_name);
    if (!AddHandler(hash_id, handler)) {
      LOG(WARNING)<<"duplicate handler, name: " << method_name;
      delete handler;
      return false;
    }
  }

  return true;
}

}
