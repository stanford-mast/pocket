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

#ifndef CRAIL_DISPATCHER_H
#define CRAIL_DISPATCHER_H

#include <string>

#include "crail_store.h"

using namespace std;

class PocketDispatcher {
public:
  PocketDispatcher();
  virtual ~PocketDispatcher();

  int Initialize(string address, int port);

  int MakeDir(string name);
  int Lookup(string name);
  int Enumerate(string name);
  int PutFile(string local_file, string dst_file, bool enumerable);
  int GetFile(string src_file, string local_file);
  int PutBuffer(const char buf[], int len, string dst_file, bool enumerable);
  int GetBuffer(char buf[], int len, string src_file);
  int DeleteFile(string file);
  int DeleteDir(string directory);
  int CountFiles(string directory);

private:
  CrailStore crail_;
};

#endif /* CRAIL_DISPATCHER_H */
