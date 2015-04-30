#include "rpc_def.h"
#include "zero_copy_stream.h"
#include "rpc_scheduler.h"

namespace rpc {

void RpcScheduler::dispatch(async::Connection* conn, io::InputStream* in_stream,
                            TimeStamp time_stamp) {
  const MessageHeader& header = GetRpcHeaderFromConnection(conn);
  if (IS_RESPONSE(header)) {
    DCHECK_NOTNULL(reply_delegate_.get());
    reply_delegate_->dispatch(conn, in_stream, time_stamp);
    return;
  }

  request_delegate_->dispatch(conn, in_stream, time_stamp);
}
}
