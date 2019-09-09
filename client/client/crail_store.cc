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

#include "crail_store.h"

#include <arpa/inet.h>
#include <iostream>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/crail_constants.h"
#include "crail_directory.h"
#include "crail_file.h"
#include "directory_record.h"
#include "metadata/filename.h"
#include "storage/storage_client.h"

using namespace crail;

CrailStore::CrailStore()
    : namenode_client_(new NamenodeClient()),
      storage_cache_(new StorageCache()) {}

CrailStore::~CrailStore() {
  this->namenode_client_->Close();
  storage_cache_->Close();
}

int CrailStore::Initialize(string address, int port) {
  return this->namenode_client_->Connect((int)inet_addr(address.c_str()), port);
}

unique_ptr<CrailNode> CrailStore::Create(string &name, FileType type,
                                         int storage_class, int location_class,
                                         bool enumerable) {
  Filename filename(name);
  int _enumerable = enumerable ? 1 : 0;
  auto create_res =
      namenode_client_->Create(filename, static_cast<int>(type), storage_class,
                               location_class, _enumerable);

  if (!create_res) {
    return nullptr;
  }

  if (create_res->Get() < 0) {
    return nullptr;
  }

  if (create_res->error() != 0) {
    return nullptr;
  }

  auto file_info = create_res->file();
  AddBlock(file_info->fd(), 0, create_res->file_block());

  long long dir_offset = file_info->dir_offset();
  if (dir_offset >= 0) {
    auto parent_info = create_res->parent();
    AddBlock(parent_info->fd(), dir_offset, create_res->parent_block());
    string _name = filename.name();
    WriteDirectoryRecord(parent_info, _name, dir_offset, 1);
  }

  return DispatchType(file_info);
}

unique_ptr<CrailNode> CrailStore::Lookup(string &name) {
  Filename filename(name);
  auto lookup_res = namenode_client_->Lookup(filename);

  if (!lookup_res) {
    return nullptr;
  }

  if (lookup_res->Get() < 0) {
    return nullptr;
  }

  if (lookup_res->error() != 0) {
    return nullptr;
  }

  auto file_info = lookup_res->file();
  AddBlock(file_info->fd(), 0, lookup_res->file_block());
  return DispatchType(file_info);
}

int CrailStore::Remove(string &name, bool recursive) {
  Filename filename(name);
  auto remove_res = namenode_client_->Remove(filename, recursive);

  if (!remove_res) {
    return -1;
  }

  if (remove_res->Get() < 0) {
    return -1;
  }

  if (remove_res->error() != 0) {
    return -1;
  }

  auto parent_info = remove_res->parent();
  long long dir_offset = remove_res->file()->dir_offset();
  string _name = filename.name();
  WriteDirectoryRecord(parent_info, _name, dir_offset, 0);

  return 0;
}

int CrailStore::Ioctl(unsigned char op, string &name) {
  Filename filename(name);
  shared_ptr<IoctlResponse> ioctl_res = namenode_client_->Ioctl(op, filename);

  if (!ioctl_res) {
    return -1;
  }

  if (ioctl_res->Get() < 0) {
    return -1;
  }

  if (ioctl_res->error() != 0) {
    return -1;
  }
  return ioctl_res->count();
}

unique_ptr<CrailNode> CrailStore::DispatchType(shared_ptr<FileInfo> file_info) {
  shared_ptr<BlockCache> file_block_cache = GetBlockCache(file_info->fd());
  if (file_info->type() == static_cast<int>(FileType::File)) {
    return make_unique<CrailFile>(file_info, namenode_client_, storage_cache_,
                                  file_block_cache);
  } else if (file_info->type() == static_cast<int>(FileType::Directory)) {
    return make_unique<CrailDirectory>(file_info, namenode_client_,
                                       storage_cache_, file_block_cache);
  } else {
    return nullptr;
  }
}

shared_ptr<BlockCache> CrailStore::GetBlockCache(int fd) {
  auto iter = block_cache_.find(fd);
  if (iter != block_cache_.end()) {
    return iter->second;
  } else {
    shared_ptr<BlockCache> cache = make_shared<BlockCache>(fd);
    this->block_cache_.insert({fd, cache});
    return cache;
  }
}

int CrailStore::AddBlock(int fd, long long offset,
                         shared_ptr<BlockInfo> block) {
  shared_ptr<BlockCache> cache = GetBlockCache(fd);
  cache->PutBlock(offset, block);
}

unique_ptr<CrailOutputstream>
CrailStore::DirectoryOuput(shared_ptr<FileInfo> file_info, long long position) {
  shared_ptr<BlockCache> dir_block_cache = GetBlockCache(file_info->fd());
  auto directory_stream =
      make_unique<CrailOutputstream>(this->namenode_client_, storage_cache_,
                                     dir_block_cache, file_info, position);
  return directory_stream;
}

int CrailStore::WriteDirectoryRecord(shared_ptr<FileInfo> parent_info,
                                     string &fname, long long offset,
                                     int valid) {
  if (offset < 0) {
    return 0;
  }

  auto directory_stream = DirectoryOuput(parent_info, offset);
  DirectoryRecord record(valid, fname);
  shared_ptr<ByteBuffer> buf = make_shared<ByteBuffer>(1024);
  record.Write(*buf);
  buf->Flip();
  directory_stream->Write(buf);
  return 0;
}
