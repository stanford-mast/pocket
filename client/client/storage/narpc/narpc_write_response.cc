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

#include "narpc_write_response.h"

NarpcWriteResponse::NarpcWriteResponse(RpcChecker *rpc_checker)
    : NarpcStorageResponse(rpc_checker, -1, -1), ret_(-1) {}

NarpcWriteResponse::~NarpcWriteResponse() {}

int NarpcWriteResponse::Write(ByteBuffer &buf) const {
  NarpcStorageResponse::Write(buf);

  buf.PutInt(ret_);

  return 0;
}

int NarpcWriteResponse::Update(ByteBuffer &buf) {
  NarpcStorageResponse::Update(buf);

  ret_ = buf.GetInt();

  return 0;
}
