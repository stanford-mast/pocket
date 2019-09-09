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

#include "create_response.h"

CreateResponse::CreateResponse(RpcClient *rpc_client)
    : NamenodeResponse(rpc_client), file_info_(new FileInfo()),
      parent_info_(new FileInfo()), file_block_(new BlockInfo()),
      parent_block_(new BlockInfo()) {}

CreateResponse::~CreateResponse() {}

int CreateResponse::Write(ByteBuffer &buf) const {
  NamenodeResponse::Write(buf);

  file_info_->Write(buf);
  parent_info_->Write(buf);
  file_block_->Write(buf);
  parent_block_->Write(buf);

  return 0;
}

int CreateResponse::Update(ByteBuffer &buf) {
  NamenodeResponse::Update(buf);

  file_info_->Update(buf);
  parent_info_->Update(buf);
  file_block_->Update(buf);
  parent_block_->Update(buf);

  return 0;
}
