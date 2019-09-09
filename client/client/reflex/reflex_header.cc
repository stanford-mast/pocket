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

#include "reflex_header.h"

ReflexHeader::ReflexHeader(short type, long long ticket, long long lba,
                           int count)
    : type_(type), ticket_(ticket), lba_(lba), count_(count),
      payload_(nullptr) {
  this->magic_ = sizeof(short) * 2 + sizeof(long long) * 2 + sizeof(count);
}

ReflexHeader::ReflexHeader(short type, long long ticket, long long lba,
                           int count, shared_ptr<ByteBuffer> payload)
    : ReflexHeader(type, ticket, lba, count) {
  this->payload_ = payload;
}

ReflexHeader::ReflexHeader() : payload_(nullptr) {}

ReflexHeader::~ReflexHeader() {}

int ReflexHeader::Write(ByteBuffer &buf) const {
  buf.set_order(ByteOrder::LittleEndian);
  buf.PutShort(magic_);
  buf.PutShort(type_);
  buf.PutLong(ticket_);
  buf.PutLong(lba_);
  buf.PutInt(count_);

  return 0;
}

int ReflexHeader::Update(ByteBuffer &buf) {
  buf.set_order(ByteOrder::LittleEndian);
  this->magic_ = buf.GetShort();
  this->type_ = buf.GetShort();
  this->ticket_ = buf.GetLong();
  this->lba_ = buf.GetLong();
  this->count_ = buf.GetInt();
  return 0;
}
