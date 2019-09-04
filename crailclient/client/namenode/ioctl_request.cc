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

#include "ioctl_request.h"

IoctlRequest::IoctlRequest(unsigned char op, Filename &name)
    : NamenodeRequest(static_cast<short>(RpcCommand::Ioctl),
                      static_cast<short>(RequestType::Ioctl)),
      filename_(name) {
  this->op_ = op;
  this->filename_ = std::move(name);
}

IoctlRequest::~IoctlRequest() {}

int IoctlRequest::Write(ByteBuffer &buf) const {
  NamenodeRequest::Write(buf);

  buf.PutByte(op_);
  filename_.Write(buf);

  return Size();
}

int IoctlRequest::Update(ByteBuffer &buf) {
  NamenodeRequest::Update(buf);

  op_ = buf.GetByte();
  filename_.Update(buf);

  return Size();
}
