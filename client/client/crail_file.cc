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

#include "crail_file.h"

CrailFile::CrailFile(shared_ptr<FileInfo> file_info,
                     shared_ptr<NamenodeClient> namenode_client,
                     shared_ptr<StorageCache> storage_cache,
                     shared_ptr<BlockCache> block_cache)
    : CrailNode(file_info) {
  this->namenode_client_ = namenode_client;
  this->storage_cache_ = storage_cache;
  this->block_cache_ = block_cache;
}

CrailFile::~CrailFile() {}

unique_ptr<CrailOutputstream> CrailFile::outputstream() {
  return make_unique<CrailOutputstream>(namenode_client_, storage_cache_,
                                        block_cache_, file_info_, 0);
}

unique_ptr<CrailInputstream> CrailFile::inputstream() {
  return make_unique<CrailInputstream>(namenode_client_, storage_cache_,
                                       block_cache_, file_info_, 0);
}
