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

#ifndef FILENAME_H
#define FILENAME_H

#include <string>

#include "common/byte_buffer.h"
#include "common/crail_constants.h"
#include "common/serializable.h"

using namespace std;
using namespace crail;

namespace crail {

class Filename : public Serializable {
public:
  Filename(string &name);
  virtual ~Filename();

  int Write(ByteBuffer &buf) const;
  int Update(ByteBuffer &buf);
  int Size() const;

  int component() { return components_[length_ - 1]; }
  string name() { return name_; }

private:
  int length_;
  int components_[kDirectoryDepth];
  string name_;
};
} // namespace crail

#endif /* FILENAME_H */
