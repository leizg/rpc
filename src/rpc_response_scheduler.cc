#include "io/io_buf.h"
#include "io/memory_block.h"
#include "async/connection.h"

#include "handler_map.h"
#include "rpc_scheduler.h"
#include "zero_copy_stream.h"

namespace rpc {

void RpcResponseScheduler::dispatch(async::Connection* conn,
                                    io::InputStream* input_stream,
                                    const TimeStamp& time_stamp) {
  const MessageHeader* header = GetRpcHeaderFromConnection(conn);
  // todo: check rpc is resonse message.
  ClientCallback* cb;
  if (!cb_ctx_->getCallbackById(header->id, &cb)) {
    delete input_stream;
    LOG(WARNING)<< "unknown id: " << header->id;
    return;
  }

  scoped_ptr<InputStream> stream(new InputStream(input_stream));
  Message* reply = cb->getResponse();
  if (!reply->ParseFromZeroCopyStream(stream.get())) {
    LOG(WARNING)<< "parse error: " << cb->getMethod()->DebugString();
    return;
  }
  cb->Run();
}
}
