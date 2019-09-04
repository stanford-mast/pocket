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

#include "crail_hash.h"

int file_hash(string name) {
  const char *str = name.c_str();
  int length = name.length();
  int hash_code = 0;
  for (uint32_t i = 0; i < length; i++) {
    hash_code = 31 * hash_code + str[i];
  }
  return hash_code;
}

int file_hash(string &name, int &start) {
  int hash_code = 0;
  while (start < name.length() && name.at(start) != '/') {
    hash_code = 31 * hash_code + name.at(start);
    start++;
  }
  return hash_code;
}
