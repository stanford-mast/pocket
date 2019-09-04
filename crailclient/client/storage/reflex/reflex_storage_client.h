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

#ifndef REFLEX_STORAGE_CLIENT_H
#define REFLEX_STORAGE_CLIENT_H

#include "reflex/reflex_client.h"
#include "storage/storage_client.h"

class ReflexStorageClient : public ReflexClient, public StorageClient {
public:
  ReflexStorageClient();
  virtual ~ReflexStorageClient();

  int Connect(int address, int port) {
    return ReflexClient::Connect(address, port);
  }
  int Close() { return ReflexClient::Close(); }
  shared_ptr<Future> WriteData(int key, long long address,
                               shared_ptr<ByteBuffer> buf);
  shared_ptr<Future> ReadData(int key, long long address,
                              shared_ptr<ByteBuffer> buf);

private:
  long long linearBlockAddress(long long address, int sector_size);
};

#endif /* REFLEX_STORAGE_CLIENT_H */
