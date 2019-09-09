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

#include "narpc_write_request.h"
#include <iostream>

using namespace std;

NarpcWriteRequest::NarpcWriteRequest(int key, long long address, int length,
                                     shared_ptr<ByteBuffer> buf)
    : NarpcStorageRequest(static_cast<int>(NarpcStorageRequestType::Write)),
      key_(key), address_(address), length_(length) {
  this->buf_ = buf;
}

NarpcWriteRequest::~NarpcWriteRequest() {}

int NarpcWriteRequest::Write(ByteBuffer &buf) const {
  NarpcStorageRequest::Write(buf);

  buf.PutInt(key_);
  buf.PutLong(address_);
  buf.PutInt(length_);
  buf.PutInt(length_);

  return 0;
}

int NarpcWriteRequest::Update(ByteBuffer &buf) {
  NarpcStorageRequest::Update(buf);

  key_ = buf.GetInt();
  address_ = buf.GetLong();
  length_ = buf.GetInt();
  length_ = buf.GetInt();

  return 0;
}
