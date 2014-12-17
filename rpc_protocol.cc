#include "io/input_buf.h"

#include "rpc_def.h"
#include "rpc_protocol.h"
#include "rpc_processor.h"

namespace {

void parseRpcHeader(const char* data, uint32 len, MessageHeader* hdr) {
  // FIXME: big edian.
  ::memcpy(hdr, data, RPC_HEADER_LENGTH);
}
}

namespace rpc {

bool RpcProtocol::RpcParser::parse(io::Connection* const conn,
                                   io::InputBuf* const input_buf) const {
  RpcAttr* attr = static_cast<RpcAttr*>(conn->getAttr());
  CHECK_EQ(attr->pending_size, 0);

  MessageHeader* hdr = attr->header();
  char* data = input_buf->Skip(RPC_HEADER_LENGTH);
  parseRpcHeader(data, RPC_HEADER_LENGTH, hdr);

  attr->data_len = hdr->length;
  attr->is_last_pkg = IS_LAST_PACKAGE(*hdr);
  return true;
}
}
