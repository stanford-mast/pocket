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

#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "common/serializable.h"

class FileInfo : public Serializable {
public:
  FileInfo();
  virtual ~FileInfo();

  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);

  int Size() const {
    return sizeof(unsigned long long) * 2 + sizeof(int) +
           sizeof(unsigned long long) * 3;
  }

  int Dump() const;

  unsigned long long fd() const { return fd_; }
  unsigned long long capacity() const { return capacity_; }
  int type() const { return node_type_; }
  long long dir_offset() const { return dir_offset_; }
  unsigned long long token() const { return token_; }
  unsigned long long modification_time() const { return modification_time_; }
  void set_capacity(unsigned long long capacity) { this->capacity_ = capacity; }

private:
  unsigned long long fd_;
  unsigned long long capacity_;
  int node_type_;
  unsigned long long dir_offset_;
  unsigned long long token_;
  unsigned long long modification_time_;
};

#endif /* FILE_INFO_H */
