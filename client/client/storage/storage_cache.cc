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

#include "storage_cache.h"
#include <iostream>

#include "common/crail_constants.h"
#include "storage/narpc/narpc_storage_client.h"
#include "storage/reflex/reflex_storage_client.h"

using namespace crail;

StorageCache::StorageCache() {}

StorageCache::~StorageCache() {}

void StorageCache::Close() {
  for (std::pair<long long, shared_ptr<StorageClient>> element : cache_) {
    element.second->Close();
    // std::cout << element.first << " :: " << element.second << std::endl;
  }
}

int StorageCache::Put(long long position, shared_ptr<StorageClient> client) {
  long long key = ComputeKey(position);
  cache_.insert({key, client});
  return 0;
}

shared_ptr<StorageClient> StorageCache::Get(long long position,
                                            int storage_class) {
  long long key = ComputeKey(position);
  auto iter = cache_.find(key);
  if (iter != cache_.end()) {
    return iter->second;
  } else {
    shared_ptr<StorageClient> client = CreateClient(storage_class);
    Put(key, client);
    return client;
  }
}

shared_ptr<StorageClient> StorageCache::CreateClient(int storage_class) {
  if (storage_class == 0) {
    return make_shared<NarpcStorageClient>();
  } else {
    return make_shared<ReflexStorageClient>();
  }
}

long long StorageCache::ComputeKey(long long position) {
  long long count = position / kBlockSize;
  return count * kBlockSize;
}
