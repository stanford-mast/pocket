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

#include "create_request.h"

Createrequest::Createrequest(Filename &name, int type, int storage_class,
                             int location_class, int enumerable)
    : NamenodeRequest(static_cast<short>(RpcCommand::Create),
                      static_cast<short>(RequestType::Create)),
      filename_(name), type_(type), storage_class_(storage_class),
      location_class_(location_class), enumerable_(enumerable) {
  this->filename_ = std::move(name);
}

Createrequest::~Createrequest() {}

int Createrequest::Write(ByteBuffer &buf) const {
  NamenodeRequest::Write(buf);

  filename_.Write(buf);
  buf.PutInt(type_);
  buf.PutInt(storage_class_);
  buf.PutInt(location_class_);
  buf.PutInt(enumerable_);

  return Size();
}

int Createrequest::Update(ByteBuffer &buf) {
  NamenodeRequest::Update(buf);

  filename_.Update(buf);
  type_ = buf.GetInt();
  storage_class_ = buf.GetInt();
  location_class_ = buf.GetInt();
  enumerable_ = buf.GetInt();

  return Size();
}
