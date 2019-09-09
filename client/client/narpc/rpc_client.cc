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

#include "rpc_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/byte_buffer.h"
#include "common/crail_constants.h"
#include "crail_store.h"
#include "utils/crail_networking.h"

using namespace std;
using namespace crail;

RpcClient::RpcClient(bool nodelay) : isConnected(false), buf_(1024) {
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  this->counter_ = 1;
  this->nodelay_ = nodelay;
}

RpcClient::~RpcClient() { Close(); }

int RpcClient::Connect(int address, int port) {
  if (isConnected) {
    return 0;
  }

  this->address_ = address;
  this->port_ = port;

  int yes = 0;
  if (nodelay_) {
    yes = 1;
  }
  setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(int));

  struct sockaddr_in addr_;
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  memset(&(addr_.sin_zero), 0, 8);
  addr_.sin_addr.s_addr = address;

  string addressport = GetAddress(address, port);
  if (connect(socket_, (struct sockaddr *)&addr_, sizeof(addr_)) == -1) {
    string message = "cannot connect to server, " + addressport;
    perror(message.c_str());
    return -1;
  } else {
    cout << "connected to " << addressport << endl;
  }
  isConnected = true;
  return 0;
}

int RpcClient::Close() {
  if (isConnected) {
    close(socket_);
    isConnected = false;
  }
  return 0;
}

int RpcClient::IssueRequest(RpcMessage &request,
                            shared_ptr<RpcResponse> response) {
  unsigned long long ticket = counter_++ % RpcClient::kMaxTicket;
  if (ticket == 0) {
    ticket++;
  }
  responseMap_[ticket] = response;
  buf_.Clear();

  // narpc header (size, ticket)
  AddNaRPCHeader(buf_, request.Size(), ticket);
  // create file request
  request.Write(buf_);

  // issue request
  buf_.Flip();
  // int _metadata = buf_.remaining();
  if (SendBytes(buf_.get_bytes(), buf_.remaining()) < 0) {
    cout << "Error when sending rpc message " << endl;
    return -1;
  }

  shared_ptr<ByteBuffer> payload = request.Payload();
  // int _data = 0;
  if (payload) {
    //_data = payload->remaining();
    if (SendBytes(payload->get_bytes(), payload->remaining()) < 0) {
      cout << "Error when sending RPC payload" << endl;
      return -1;
    }
  }

  // int _total = _metadata + _data;
  // cout << "transmitting message, port " << port_ << ", size " << _total <<
  // endl;

  return 0;
}

int RpcClient::PollResponse() {
  // recv resp header
  buf_.Clear();
  if (RecvBytes(buf_.get_bytes(), kNarpcHeader) < 0) {
    cout << "Error receiving rpc header" << endl;
    return -1;
  }
  int size = buf_.GetInt();
  long long ticket = buf_.GetLong();

  shared_ptr<RpcMessage> response = responseMap_[ticket];
  responseMap_[ticket] = nullptr;

  shared_ptr<ByteBuffer> payload = response->Payload();
  int payload_size = 0;
  if (payload) {
    payload_size = payload->remaining();
  }

  // recv resp obj
  buf_.Clear();
  int header_size = size - payload_size;
  if (RecvBytes(buf_.get_bytes(), header_size) < 0) {
    cout << "Error receiving rpc message" << endl;
    return -1;
  }

  response->Update(buf_);

  if (payload) {
    if (RecvBytes(payload->get_bytes(), payload->remaining()) < 0) {
      cout << "Error receiving rpc payload" << endl;
      return -1;
    }
  }

  // int _total = kNarpcHeader + size;
  // cout << "receiving message, port " << port_ << ", size " << _total << endl;

  /*
int extra = recv(socket_, buf_.get_bytes(), 1, MSG_DONTWAIT);
if (extra > 0) {
cout << "reading extra data! " << endl;
return -1;
}
  */

  return 0;
}

void RpcClient::AddNaRPCHeader(ByteBuffer &buf, int size,
                               unsigned long long ticket) {
  buf.PutInt(size);
  buf.PutLong(ticket);
}

long long RpcClient::RemoveNaRPCHeader(ByteBuffer &buf) {
  buf.GetInt();
  long long ticket = buf.GetLong();
  return ticket;
}

int RpcClient::SendBytes(unsigned char *buf, int size) {
  int res = send(socket_, buf, (size_t)size, (int)0);
  if (res < 0) {
    return res;
  }
  int remaining = size - res;
  while (remaining > 0) {
    int offset = size - remaining;
    res = send(socket_, buf + offset, (size_t)remaining, (int)0);
    if (res < 0) {
      return res;
    }
    remaining -= res;
  }
  return remaining;
}

int RpcClient::RecvBytes(unsigned char *buf, int size) {
  int sum = 0;
  while (sum < size) {
    int res = recv(socket_, buf + sum, (size_t)(size - sum), MSG_DONTWAIT);

    if (res < 0) {
      if (errno == EAGAIN) {
        continue;
      }
      // return res;
      break;
    }

    sum += res;
  }

  return sum != size ? -1 : 0;
}
