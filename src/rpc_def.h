#pragma once

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

typedef google::protobuf::Service Service;
typedef google::protobuf::Message Message;
typedef google::protobuf::RpcController RpcController;
typedef google::protobuf::MethodDescriptor MethodDescriptor;
typedef google::protobuf::ServiceDescriptor ServiceDescriptor;

#define RPC_RESPONSE (1<<15)
#define SET_RESPONSE_TAG(hdr) \
  do { \
    (hdr).flags |= RPC_RESPONSE; \
  } while (0)
#define IS_RESPONSE(hdr) ((hdr).flags & RPC_RESPONSE)

#define RPC_LAST_PACKAGE (1<<14)
#define SET_LAST_TAG(hdr) \
  do { \
    (hdr).flags |= RPC_LAST_PACKAGE; \
  } while (0)
#define IS_LAST_PACKAGE(hdr) ((hdr).flags & RPC_LAST_PACKAGE)

// fixed length: 16.
struct MessageHeader {
    uint32 fun_id;
    uint16 flags;  // request or response.
    uint16 length;
    uint64 id;
}__attribute__((packed));

#define RPC_HEADER_LENGTH (sizeof (MessageHeader))

//static_assert(RPC_HEADER_LENGTH == 16);

