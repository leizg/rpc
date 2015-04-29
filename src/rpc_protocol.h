#pragma once

#include "rpc_def.h"
#include "async/protocol.h"

namespace rpc {

class RpcProtocol : public async::ProActorProtocol {
  private:
    class RpcParser : Parser {
      public:
        RpcParser() {
        }
        virtual ~RpcParser() {
        }

      private:
        virtual uint32 headerLength() const {
          return RPC_HEADER_LENGTH;
        }

        virtual bool parseHeader(async::Connection* conn) const;

        DISALLOW_COPY_AND_ASSIGN(RpcParser);
    };

  public:
    class RpcData : public UserData {
      public:
        virtual ~RpcData() {
        }

        MessageHeader* header() const {
          return &header_;
        }

      private:
        MessageHeader header_;

        //todo: others...
    };

    explicit RpcProtocol(async::ProActorProtocol::Scheduler* p)
        : async::ProActorProtocol(new RpcParser, p) {
    }
    virtual ~RpcProtocol() {
    }

  private:
    virtual async::Connection::UserData* NewConnectionData() const {
      return new RpcData;
    }

    DISALLOW_COPY_AND_ASSIGN(RpcProtocol);
};

inline MessageHeader* GetRpcHeaderFromConnection(async::Connection* conn) {
  RpcProtocol::RpcData* attr =
      static_cast<RpcProtocol::RpcData*>(conn->getData());
  return attr->header();
}
}
