#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"

namespace scheme_misc::table {
class A;
typedef std::shared_ptr<A> APtr;

struct QueryReq;
struct QueryRsp;
struct QueryReceipt;

class Session {
 public:
  Session(APtr a);
  bool OnQueryReq(QueryReq const& req, QueryRsp& rsp);
  bool OnQueryReceipt(QueryReceipt const& receipt, Fr& r);

 private:
  APtr a_;

 private:
  Fr r_;
  G1 g_exp_r_;
};

typedef std::unique_ptr<Session> SessionUPtr;

}  // namespace scheme_misc::table