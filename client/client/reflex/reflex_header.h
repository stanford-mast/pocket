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

#ifndef REFLEX_HEADER_H
#define REFLEX_HEADER_H

#include <memory>

#include "common/byte_buffer.h"
#include "common/serializable.h"

using namespace std;

class ReflexHeader : public Serializable {
public:
  ReflexHeader(short type, long long ticket, long long lba, int count);
  ReflexHeader(short type, long long ticket, long long lba, int count,
               shared_ptr<ByteBuffer> payload);
  ReflexHeader();
  ~ReflexHeader();

  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);
  int Size() const {
    return sizeof(magic_) + sizeof(type_) + sizeof(ticket_) + sizeof(lba_) +
           sizeof(count_);
  };

  short magic() const { return magic_; }
  short type() const { return type_; }
  long long ticket() const { return ticket_; }
  int count() const { return count_; }

  shared_ptr<ByteBuffer> Payload() { return payload_; };

private:
  short magic_;
  short type_;
  long long ticket_;
  long long lba_;
  int count_;

  shared_ptr<ByteBuffer> payload_;
};

#endif /* REFLEX_HEADER_H */
