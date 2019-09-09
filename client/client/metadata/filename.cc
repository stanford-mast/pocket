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

#include "filename.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

#include "utils/crail_hash.h"

using namespace crail;

Filename::Filename(string &name) {
  int index_name = 1;
  int index_comp = 0;
  int name_start = index_name;
  int name_stop = name_start;
  while (index_name < name.length()) {
    name_start = index_name;
    int hash_code = file_hash(name, index_name);
    name_stop = index_name - 1;
    components_[index_comp] = hash_code;
    index_comp++;
    index_name++;
  }
  this->length_ = index_comp;
  this->name_ = name.substr(name_start, name_stop - name_start + 1);

  /*
char slash = '/';
int start_index = 1;
std::string::size_type pos = name.find(&slash, start_index);
int index = 0;
while (pos != std::string::npos) {
int hash_code = 0;
for (uint32_t i = start_index; i < pos; i++) {
hash_code = 31 * hash_code + name[i];
}
components_[index] = hash_code;
index++;
start_index = pos + 1;
}
if (start_index < name.length()) {
int hash_code = 0;
for (uint32_t i = start_index; i < name.length(); i++) {
hash_code = 31 * hash_code + name[i];
}
components_[index] = hash_code;
index++;
}
length_ = index;
while (index < kDirectoryDepth) {
components_[index] = 0;
index++;
}
  */

  /*
std::string token;
std::stringstream tokenStream(name);
int index = 0;
while (std::getline(tokenStream, token, '/')) {
if (token.empty()) {
continue;
}
components_[index] = file_hash(token);
name_ = token;
index++;
}
length_ = index;
while (index < kDirectoryDepth) {
components_[index] = 0;
index++;
}
  */
}

Filename::~Filename() {}

int Filename::Write(ByteBuffer &buf) const {
  buf.PutInt(length_);
  for (int i = 0; i < kDirectoryDepth; i++) {
    buf.PutInt(components_[i]);
  }

  return Size();
}

int Filename::Update(ByteBuffer &buf) {
  length_ = buf.GetInt();
  for (int i = 0; i < kDirectoryDepth; i++) {
    components_[i] = buf.GetInt();
  }
  return Size();
}

int Filename::Size() const { return 4 + 16 * 4; }
