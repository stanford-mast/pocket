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

#ifndef GETBLOCK_REQUEST_H
#define GETBLOCK_REQUEST_H

#include <memory>

#include "common/byte_buffer.h"
#include "namenode_request.h"
#include "narpc/rpc_client.h"

class GetblockRequest : public NamenodeRequest, public RpcMessage {
public:
  GetblockRequest(long long fd, long long token, long long position,
                  long long capacity);
  virtual ~GetblockRequest();

  shared_ptr<ByteBuffer> Payload() { return nullptr; }

  int Size() const { return NamenodeRequest::Size() + sizeof(long long) * 4; };
  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  long long fd() const { return fd_; }
  long long token() const { return token_; }
  long long position() const { return position_; }
  long long capacity() const { return capacity_; }

private:
  long long fd_;
  long long token_;
  long long position_;
  long long capacity_;
};

#endif /* GETBLOCK_REQUEST_H */
