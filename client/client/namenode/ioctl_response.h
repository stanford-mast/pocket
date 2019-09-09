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

#ifndef IOCTL_RESPONSE_H
#define IOCTL_RESPONSE_H

#include "common/serializable.h"
#include "metadata/block_info.h"
#include "metadata/file_info.h"
#include "namenode_response.h"
#include "narpc/rpc_client.h"
#include "narpc/rpc_message.h"

using namespace crail;

class IoctlResponse : public NamenodeResponse {
public:
  IoctlResponse(RpcClient *rpc_client);
  virtual ~IoctlResponse();

  shared_ptr<ByteBuffer> Payload() { return nullptr; }

  int Size() const {
    return NamenodeResponse::Size() + sizeof(op_) + sizeof(long long);
  }
  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  long long count() const { return count_; }

private:
  unsigned char op_;
  long long count_;
};

#endif /* IOCTL_RESPONSE_H */
