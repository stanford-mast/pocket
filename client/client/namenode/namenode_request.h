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

#ifndef NAMENODE_REQUEST_H
#define NAMENODE_REQUEST_H

#include "common/byte_buffer.h"
#include "common/serializable.h"

using namespace crail;

enum class RpcCommand : short {
  Create = 1,
  Lookup = 2,
  Setfile = 3,
  Removefile = 4,
  Getblock = 6,
  Ioctl = 13,
};
enum class RequestType : short {
  Create = 1,
  Lookup = 2,
  Setfile = 3,
  Removefile = 4,
  Getblock = 6,
  Ioctl = 13
};

class NamenodeRequest : public Serializable {
public:
  NamenodeRequest(short cmd, short type);
  virtual ~NamenodeRequest();

  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);
  int Size() const { return sizeof(short) * 2; }

private:
  short cmd_;
  short type_;
};

#endif /* NAMENODE_REQUEST_H */
