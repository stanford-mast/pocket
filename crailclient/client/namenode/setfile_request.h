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

#ifndef SETFILE_REQUEST_H
#define SETFILE_REQUEST_H

#include <memory>
#include <string>

#include "common/byte_buffer.h"
#include "common/serializable.h"
#include "metadata/file_info.h"
#include "metadata/filename.h"
#include "namenode_request.h"
#include "narpc/rpc_message.h"

class SetfileRequest : public NamenodeRequest, public RpcMessage {
public:
  SetfileRequest(shared_ptr<FileInfo> file_info, bool close);
  virtual ~SetfileRequest();

  shared_ptr<ByteBuffer> Payload() { return nullptr; }

  int Size() const {
    return NamenodeRequest::Size() + file_info_->Size() + sizeof(int);
  }
  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  const shared_ptr<FileInfo> file_info() const { return file_info_; }

private:
  shared_ptr<FileInfo> file_info_;
  bool close_;
};

#endif /* SETFILE_REQUEST_H */
