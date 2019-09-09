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

#ifndef BLOCK_CACHE_H
#define BLOCK_CACHE_H

#include "metadata/block_info.h"
#include <unordered_map>

using namespace std;
using namespace crail;

class BlockCache {
public:
  BlockCache(int fd);
  virtual ~BlockCache();

  int PutBlock(long long offset, shared_ptr<BlockInfo> block);
  shared_ptr<BlockInfo> GetBlock(long long offset);

private:
  int fd_;
  unordered_map<long long, shared_ptr<BlockInfo>> cache_;
};

#endif /* BLOCK_CACHE_H */
