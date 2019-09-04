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

#ifndef CREATEREQUEST_H
#define CREATEREQUEST_H

#include <memory>
#include <string>

#include "common/byte_buffer.h"
#include "common/serializable.h"
#include "metadata/filename.h"
#include "namenode_request.h"
#include "narpc/rpc_message.h"

using namespace std;

namespace crail {

class Createrequest : public NamenodeRequest, public RpcMessage {
public:
  Createrequest(Filename &name, int type, int storage_class, int location_class,
                int enumerable);
  virtual ~Createrequest();

  shared_ptr<ByteBuffer> Payload() { return nullptr; }

  int Size() const {
    return NamenodeRequest::Size() + filename_.Size() + 4 * sizeof(int);
  }
  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  const Filename &filename() const { return filename_; }
  int type() const { return type_; }
  int storage_class() const { return storage_class_; }
  int location_class() const { return location_class_; }
  int enumerable() const { return enumerable_; }

private:
  Filename filename_;
  int type_;
  int storage_class_;
  int location_class_;
  int enumerable_;
};
} // namespace crail

#endif /* CREATEREQUEST_H */
