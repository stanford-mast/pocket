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

#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include <memory>

using namespace std;

namespace crail {

enum class ByteOrder : short { BigEndian = 1, LittleEndian = 2 };

class ByteBuffer {
public:
  ByteBuffer(int size);
  virtual ~ByteBuffer();

  void PutByte(unsigned char value);
  void PutShort(short value);
  void PutInt(int value);
  void PutLong(long long value);
  void PutBytes(const char value[], int length);

  unsigned char GetByte();
  short GetShort();
  int GetInt();
  long long GetLong();
  void GetBytes(char value[], int length);
  void PrintBytes(string message);
  void Zero();

  ByteBuffer &Clear() {
    position_ = 0;
    limit_ = size_;
    return *this;
  }

  inline ByteBuffer &Flip() {
    limit_ = position_;
    position_ = 0;
    return *this;
  }

  int size() const { return size_; }
  int position() const { return position_; }
  void set_position(int new_position) { this->position_ = new_position; }
  int limit() const { return limit_; }
  void set_limit(int new_limit) { this->limit_ = new_limit; }
  int remaining() const { return limit_ - position_; }
  unsigned char *get_bytes() const { return buf_ + position_; }
  ByteOrder order() { return order_; }
  void set_order(ByteOrder order) { this->order_ = order; }

private:
  int size_;
  int position_;
  int limit_;
  unsigned char *buf_;

  ByteOrder order_;
};
} // namespace crail

#endif /* BYTE_BUFFER_H */
