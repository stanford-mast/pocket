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

#ifndef REMOVE_REQUEST_H
#define REMOVE_REQUEST_H

#include <string>

#include "metadata/filename.h"
#include "namenode_request.h"
#include "narpc/rpc_message.h"

class RemoveRequest : public NamenodeRequest, public RpcMessage {
public:
  RemoveRequest(Filename &name, bool recursive);
  virtual ~RemoveRequest();

  shared_ptr<ByteBuffer> Payload() { return nullptr; }

  int Size() const {
    return NamenodeRequest::Size() + filename_.Size() + sizeof(int);
  }
  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  const Filename &filename() const { return filename_; }
  bool recursive() const { return recursive_; }

private:
  Filename filename_;
  bool recursive_;
};

#endif /* REMOVE_REQUEST_H */
