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

#include "byte_buffer.h"

#include <arpa/inet.h>
#include <endian.h>
#include <iostream>
#include <string.h>

using namespace crail;
using namespace std;

ByteBuffer::ByteBuffer(int size) {
  this->buf_ = new unsigned char[size];
  this->size_ = size;
  this->limit_ = size;
  this->position_ = 0;
  this->order_ = ByteOrder::BigEndian;
  Zero();
}

ByteBuffer::~ByteBuffer() { delete[] buf_; }

void ByteBuffer::PutByte(unsigned char value) {
  unsigned char *_tmp = (unsigned char *)get_bytes();
  *_tmp = value;
  this->position_ += sizeof(value);
}

void ByteBuffer::PutInt(int value) {
  int *_tmp = (int *)get_bytes();
  if (order_ == ByteOrder::BigEndian) {
    *_tmp = htonl(value);
  } else {
    *_tmp = value;
  }
  this->position_ += sizeof(value);
}

void ByteBuffer::PutShort(short value) {
  short *_tmp = (short *)get_bytes();
  if (order_ == ByteOrder::BigEndian) {
    *_tmp = htons(value);
  } else {
    *_tmp = value;
  }
  this->position_ += sizeof(value);
}

void ByteBuffer::PutLong(long long value) {
  long long *_tmp = (long long *)get_bytes();
  if (order_ == ByteOrder::BigEndian) {
    *_tmp = htobe64(value);
  } else {
    *_tmp = value;
  }
  this->position_ += sizeof(value);
}

void ByteBuffer::PutBytes(const char value[], int length) {
  unsigned char *_tmp = (unsigned char *)get_bytes();
  memcpy(_tmp, value, length);
  this->position_ += length;
}

unsigned char ByteBuffer::GetByte() {
  unsigned char *_tmp = (unsigned char *)get_bytes();
  this->position_ += sizeof(unsigned char);
  return *_tmp;
}

short ByteBuffer::GetShort() {
  short *_tmp = (short *)get_bytes();
  this->position_ += sizeof(short);
  if (order_ == ByteOrder::BigEndian) {
    return ntohs(*_tmp);
  } else {
    return *_tmp;
  }
}

int ByteBuffer::GetInt() {
  int *_tmp = (int *)get_bytes();
  this->position_ += sizeof(int);
  if (order_ == ByteOrder::BigEndian) {
    return ntohl(*_tmp);
  } else {
    return *_tmp;
  }
}

long long ByteBuffer::GetLong() {
  long long *_tmp = (long long *)get_bytes();
  this->position_ += sizeof(long long);
  if (order_ == ByteOrder::BigEndian) {
    return be64toh(*_tmp);
  } else {
    return *_tmp;
  }
}

void ByteBuffer::GetBytes(char value[], int length) {
  unsigned char *_tmp = (unsigned char *)get_bytes();
  memcpy(value, _tmp, length);
  this->position_ += length;
}

void ByteBuffer::PrintBytes(string message) {
  cout << "printbytes [" << message.c_str() << "]" << endl;
  for (int i = 0; i < size_; i++) {
    unsigned int ch = (unsigned int)buf_[i];
    cout << ch << ".";
  }
  cout << endl;
}

void ByteBuffer::Zero() { bzero(buf_, size_); }
