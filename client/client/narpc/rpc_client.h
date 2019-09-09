/*
 * CppCrail: Native Crail
 *
 * Author: Patrick Stuedi  <stu@zurich.ibm.com>
 *
 * Copyright (C) 2015-2018, IBM Corporation
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <atomic>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

#include "common/byte_buffer.h"
#include "common/serializable.h"
#include "rpc_checker.h"
#include "rpc_message.h"
#include "rpc_response.h"

using namespace std;
using namespace crail;

namespace crail {

class RpcClient : public RpcChecker {
public:
  RpcClient(bool nodelay);
  virtual ~RpcClient();

  static const int kNarpcHeader = 12;
  static const int kRpcHeader = 4;
  static const int kMaxTicket = 8;

  int Connect(int address, int port);
  int IssueRequest(RpcMessage &request, shared_ptr<RpcResponse> response);
  int Close();

private:
  int PollResponse();

  int SendBytes(unsigned char *buf, int size);
  int RecvBytes(unsigned char *buf, int size);
  void AddNaRPCHeader(ByteBuffer &buf, int size, unsigned long long ticket);
  long long RemoveNaRPCHeader(ByteBuffer &buf);

  int socket_;
  atomic<unsigned long long> counter_;
  shared_ptr<RpcMessage> responseMap_[kMaxTicket];
  bool isConnected;
  ByteBuffer buf_;
  bool nodelay_;
  int address_;
  int port_;

  friend class RpcResponse;
};
} // namespace crail

#endif /* RPC_CLIENT_H */
