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

#include "setfile_request.h"

SetfileRequest::SetfileRequest(shared_ptr<FileInfo> file_info, bool close)
    : NamenodeRequest(static_cast<short>(RpcCommand::Setfile),
                      static_cast<short>(RequestType::Setfile)) {
  this->file_info_ = file_info;
  this->close_ = close;
}

SetfileRequest::~SetfileRequest() {}

int SetfileRequest::Write(ByteBuffer &buf) const {
  NamenodeRequest::Write(buf);

  file_info_->Write(buf);
  int _close = close_ ? 1 : 0;
  buf.PutInt(_close);

  return Size();
}

int SetfileRequest::Update(ByteBuffer &buf) {
  NamenodeRequest::Update(buf);

  file_info_->Update(buf);
  buf.GetInt();

  return Size();
}
