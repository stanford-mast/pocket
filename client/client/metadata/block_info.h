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

#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include "common/byte_buffer.h"
#include "common/serializable.h"
#include "datanode_info.h"

class BlockInfo : public Serializable {
public:
  BlockInfo();
  virtual ~BlockInfo();

  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  int Size() const {
    return datanode_info_->Size() + sizeof(unsigned long long) * 2 +
           sizeof(int) * 2;
  }

  int Dump() const;

  DatanodeInfo *datanode() const { return datanode_info_; }
  unsigned long long lba() const { return lba_; }
  unsigned long long addr() const { return addr_; }
  int length() const { return length_; }
  int lkey() const { return lkey_; }

private:
  DatanodeInfo *datanode_info_;
  unsigned long long lba_;
  unsigned long long addr_;
  int length_;
  int lkey_;
};

#endif /* BLOCK_INFO_H */
