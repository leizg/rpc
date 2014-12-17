#include "rpc_def.h"
#include "zero_copy_stream.h"
#include "rpc_processor.h"

namespace rpc {

#if 0
void RpcProcessor::dispatch(io::Connection* conn, io::InputBuf* input_buf,
    const TimeStamp& time_stamp) {
  const MessageHeader& header = GetRpcHeaderFromConnection(conn);
  if (IS_RESPONSE(header)) {
    DCHECK_NOTNULL(reply_delegate_.get());
    reply_delegate_->process(conn, input_buf, time_stamp);
    return;
  }

  request_delegate_->process(conn, input_buf, time_stamp);
}
#endif
}
