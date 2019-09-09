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

#include <iostream>

#include "narpc_read_request.h"
#include "narpc_read_response.h"
#include "narpc_storage_request.h"
#include "narpc_storage_response.h"
#include "narpc_write_request.h"
#include "narpc_write_response.h"
#include "storage/narpc/narpc_storage_client.h"

using namespace std;

NarpcStorageClient::NarpcStorageClient()
    : RpcClient(NarpcStorageClient::kNodelay) {}

NarpcStorageClient::~NarpcStorageClient() {}

shared_ptr<Future> NarpcStorageClient::WriteData(int key, long long address,
                                                 shared_ptr<ByteBuffer> buf) {
  NarpcWriteRequest write_request(key, address, buf->remaining(), buf);
  shared_ptr<NarpcWriteResponse> write_response =
      make_shared<NarpcWriteResponse>(this);
  if (IssueRequest(write_request, write_response) < 0) {
    return nullptr;
  }
  return write_response;
}

shared_ptr<Future> NarpcStorageClient::ReadData(int key, long long address,
                                                shared_ptr<ByteBuffer> buf) {
  NarpcReadRequest read_request(key, address, buf->remaining());
  shared_ptr<NarpcReadResponse> read_response =
      make_shared<NarpcReadResponse>(this, buf);
  if (IssueRequest(read_request, read_response) < 0) {
    return nullptr;
  }
  return read_response;
}
