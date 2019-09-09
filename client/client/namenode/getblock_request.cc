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

#include "getblock_request.h"

GetblockRequest::GetblockRequest(long long fd, long long token,
                                 long long position, long long capacity)
    : NamenodeRequest(static_cast<short>(RpcCommand::Getblock),
                      static_cast<short>(RequestType::Getblock)),
      fd_(fd), token_(token), position_(position), capacity_(capacity) {}

GetblockRequest::~GetblockRequest() {}

int GetblockRequest::Write(ByteBuffer &buf) const {
  NamenodeRequest::Write(buf);

  buf.PutLong(fd_);
  buf.PutLong(token_);
  buf.PutLong(position_);
  buf.PutLong(capacity_);

  return Size();
}

int GetblockRequest::Update(ByteBuffer &buf) {
  NamenodeRequest::Update(buf);

  fd_ = buf.GetLong();
  token_ = buf.GetLong();
  position_ = buf.GetLong();
  capacity_ = buf.GetLong();

  return Size();
}
