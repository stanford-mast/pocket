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

#ifndef REFLEX_FUTURE_H
#define REFLEX_FUTURE_H

#include "common/byte_buffer.h"
#include "common/future.h"
#include "reflex_checker.h"
#include <memory>

using namespace std;
using namespace crail;

class ReflexFuture : public Future {
public:
  ReflexFuture(ReflexChecker *reflex_checker, long long ticket,
               shared_ptr<ByteBuffer> buffer);
  virtual ~ReflexFuture();

  int Get();

  long long ticket() const { return ticket_; }
  bool is_done() const { return done_; }
  shared_ptr<ByteBuffer> buffer() { return buffer_; }

private:
  ReflexChecker *reflex_checker_;
  shared_ptr<ByteBuffer> buffer_;
  long long ticket_;
  bool done_;
};

#endif /* REFLEX_FUTURE_H */
