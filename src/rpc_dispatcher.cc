#include "io/io_buf.h"
#include "io/memory_block.h"
#include "async/connection.h"

#include "handler_map.h"
#include "zero_copy_stream.h"
#include "rpc_dispatcher.h"

namespace {

class ReplyObject : public io::OutVectorObject::IoObject {
  public:
    ReplyObject(const MessageHeader& header, Message* reply)
        : msg_(reply) {
      DCHECK_NOTNULL(reply);
      buildData(header);
    }
    virtual ~ReplyObject() {
    }

  private:
    scoped_ptr<Message> msg_;
    scoped_ptr<io::ExternableChunk> chunk_;

    void buildData(const MessageHeader& header);
    void buildHeader(const MessageHeader& header, char* data) const;

    std::vector<iovec> iov_;
    virtual const std::vector<iovec>& ioVec() const {
      return iov_;
    }

    DISALLOW_COPY_AND_ASSIGN(ReplyObject);
};

// TODO: network order...
void ReplyObject::buildHeader(const MessageHeader& header, char* data) const {
  MessageHeader* reply_hdr = (MessageHeader*) data;
  reply_hdr->fun_id = header.fun_id;
  SET_LAST_TAG(*reply_hdr);
  SET_RESPONSE_TAG(*reply_hdr);
  reply_hdr->length = msg_->ByteSize();
  reply_hdr->id = header.id;
}

void ReplyObject::buildData(const MessageHeader& header) {
  uint32 total_len = msg_->ByteSize() + RPC_HEADER_LENGTH + 1;
  chunk_.reset(new io::ExternableChunk(total_len));
  buildHeader(header, chunk_->peekW());
  chunk_->skipWrite(RPC_HEADER_LENGTH);

  // TODO: ZeroCopyStream.
  chunk_->ensureLeft(msg_->ByteSize());
  msg_->SerializePartialToArray(chunk_->peekW(), msg_->ByteSize());

  iovec io;
  io.iov_base = chunk_->peekR();
  io.iov_len = chunk_->writen();
  iov_.push_back(io);
}
}
namespace rpc {

void RpcRequestDispatcher::dispatch(async::Connection* conn,
                                    io::InputStream* input_stream,
                                    const TimeStamp& time_stamp) {
  const MessageHeader* header = GetRpcHeaderFromConnection(conn);
  MethodHandler* method_handler = handler_map_->FindMehodById(header->fun_id);
  if (method_handler == nullptr) {
    delete input_stream;
    LOG(WARNING)<< "can't find handler, id: " << header->fun_id;
    return;
  }

  scoped_ptr<Message> req(method_handler->request->New());
  scoped_ptr<InputStream> stream(new InputStream(input_stream));
  bool ret = req->ParseFromZeroCopyStream(stream.get());
  if (!ret) {
    DLOG(WARNING)<< "parse request error: " << req->DebugString();
    return;
  }

  // reply will be released by Closure.
  Message* reply = method_handler->reply->New();
  method_handler->service->CallMethod(method_handler->method, NULL, req.get(),
                                      reply,
                                      new ReplyClosure(conn, header, reply));
}

void RpcResponseDispatcher::dispatch(async::Connection* conn,
                                     io::InputStream* input_stream,
                                     const TimeStamp& time_stamp) {
  const MessageHeader* header = GetRpcHeaderFromConnection(conn);
  ClientCallback* cb;
  if (!cb_finder_->find(header->id, &cb)) {
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

RpcRequestDispatcher::ReplyClosure::ReplyClosure(async::Connection* conn,
                                                 const MessageHeader& header,
                                                 Message* reply)
    : hdr_(header), reply_(reply), conn_(conn) {
  DCHECK_NOTNULL(conn);
  DCHECK_NOTNULL(reply);
  conn->Ref();
}

RpcRequestDispatcher::ReplyClosure::~ReplyClosure() {
}

void RpcRequestDispatcher::ReplyClosure::Run() {
  io::OutputObject* obj = new io::OutVectorObject(
      new ReplyObject(hdr_, reply_.release()));
  conn_->send(obj);
  delete this;
}

}
