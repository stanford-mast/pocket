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

#include "block_info.h"

#include <iostream>

BlockInfo::BlockInfo() { this->datanode_info_ = new DatanodeInfo(); }

BlockInfo::~BlockInfo() { delete datanode_info_; }

int BlockInfo::Write(ByteBuffer &buf) const {
  datanode_info_->Write(buf);
  buf.PutLong(lba_);
  buf.PutLong(addr_);
  buf.PutInt(length_);
  buf.PutInt(lkey_);

  return 0;
}

int BlockInfo::Update(ByteBuffer &buf) {
  datanode_info_->Update(buf);
  lba_ = buf.GetLong();
  addr_ = buf.GetLong();
  length_ = buf.GetInt();
  lkey_ = buf.GetInt();

  return 0;
}

int BlockInfo::Dump() const {
  datanode_info_->Dump();
  cout << "lba " << lba_ << ", addr " << addr_ << ", length_ " << length_
       << ", lkey " << lkey_ << endl;
  return 0;
}
