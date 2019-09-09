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

#ifndef IOBENCH_H
#define IOBENCH_H

#include <string>

#include "crail_store.h"

using namespace std;

class Iobench {
public:
  Iobench(string address, int port);
  virtual ~Iobench();

  int GetFile(string &filename, const int loop);
  int WriteFile(string local_file, string dst_file, bool enumerable);
  int ReadFile(string src_file, string local_file);
  int Write(string dst_file, int len, int loop);
  int Read(string src_file, int len, int loop);
  int PutKey(const char data[], int len, string dst_file, bool enumerable);
  int GetKey(char data[], int len, string src_file);

private:
  CrailStore crail_;
};

#endif /* IOBENCH_H */
