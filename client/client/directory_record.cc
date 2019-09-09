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

#include "directory_record.h"
#include <iostream>
#include <string.h>

using namespace std;

DirectoryRecord::DirectoryRecord() : valid_(-1) {
  this->name_ = make_unique<string>("");
}

DirectoryRecord::DirectoryRecord(int valid, string &name)
    : valid_(valid), name_(new string(name)) {}

DirectoryRecord::~DirectoryRecord() {}

int DirectoryRecord::Write(ByteBuffer &buf) const {
  int _length = name_->length();
  buf.PutInt(valid_);
  buf.PutInt(_length);
  buf.PutBytes(name_->c_str(), name_->length());
  return Size();
}

int DirectoryRecord::Update(ByteBuffer &buf) {
  this->valid_ = buf.GetInt();
  int _length = buf.GetInt();
  char _tmp[_length + 1];
  buf.GetBytes(_tmp, _length);
  _tmp[_length] = '\0';
  name_.reset(new string(_tmp));
  return Size();
}

int DirectoryRecord::Size() const { return sizeof(int) * 2 + name_->length(); }
