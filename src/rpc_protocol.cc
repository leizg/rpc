#include "rpc_def.h"
#include "rpc_protocol.h"
#include "rpc_dispatcher.h"

#include "io/io_buf.h"

namespace {

void parseRpcHeader(const char* data, uint32 len, MessageHeader* hdr) {
  // FIXME: big edian.
  ::memcpy(hdr, data, RPC_HEADER_LENGTH);
}
}

namespace rpc {

bool RpcProtocol::RpcParser::parseHeader(async::Connection* conn) const {
  RpcData* attr = reinterpret_cast<RpcData*>(conn->getData());
  DCHECK_EQ(attr->pending_size, 0);

  MessageHeader* hdr = attr->header();
  parseRpcHeader(attr->peekHeader(), RPC_HEADER_LENGTH, hdr);
  attr->chunk->skipRead(RPC_HEADER_LENGTH);

  attr->pending_size = hdr->length;
  attr->is_last = IS_LAST_PACKAGE(*hdr);

  return true;
}
}
