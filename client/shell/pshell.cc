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

#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include "pocket_dispatcher.h"

using namespace std;

enum class Operation {
  Undefined = 0,
  MakeDir = 1,
  Lookup = 2,
  Enumerate = 3,
  PutFile = 4,
  GetFile = 5,
  DeleteDir = 6,
  DeleteFile = 7,
  Ioctl = 8,
  PutBuffer = 9,
  GetBuffer = 10,
  PutBenchmark = 11,
  GetBenchmark = 12
};

struct Settings {
  string address;
  int port;
  Operation operation;
  string filename;
  int loop;
  int size;
  string dstfile;
};

Operation getOperation(string name) {
  if (name == "MakeDir") {
    return Operation::MakeDir;
  } else if (name == "Lookup") {
    return Operation::Lookup;
  } else if (name == "Enumerate") {
    return Operation::Enumerate;
  } else if (name == "PutFile") {
    return Operation::PutFile;
  } else if (name == "GetFile") {
    return Operation::GetFile;
  } else if (name == "DeleteDir") {
    return Operation::DeleteDir;
  } else if (name == "DeleteFile") {
    return Operation::DeleteFile;
  } else if (name == "Ioctl") {
    return Operation::Ioctl;
  } else if (name == "PutBuffer") {
    return Operation::PutBuffer;
  } else if (name == "GetBuffer") {
    return Operation::GetBuffer;
  } else if (name == "PutBenchmark") {
    return Operation::PutBenchmark;
  } else if (name == "GetBenchmark") {
    return Operation::GetBenchmark;
  } else {
    return Operation::Undefined;
  }
}

void printSettings(Settings &settings) {
  cout << "Settings, address " << settings.address << ", port " << settings.port
       << ", operation " << static_cast<int>(settings.operation)
       << ", filename " << settings.filename << ", loop " << settings.loop
       << ", size " << settings.size << ", dstfile " << settings.dstfile
       << endl;
}

int main(int argc, char *argv[]) {
  Settings settings;

  int opt = 0;
  while ((opt = getopt(argc, argv, "t:f:k:s:a:p:d:")) != -1) {
    switch (opt) {
    case 't':
      settings.operation = getOperation(optarg);
      break;
    case 'f':
      settings.filename = optarg;
      break;
    case 'k':
      settings.loop = atoi(optarg);
      break;
    case 's':
      settings.size = atoi(optarg);
      break;
    case 'a':
      settings.address = optarg;
      break;
    case 'p':
      settings.port = atoi(optarg);
      break;
    case 'd':
      settings.dstfile = optarg;
      break;
    }
  }

  printSettings(settings);

  PocketDispatcher dispatcher;
  if (dispatcher.Initialize(settings.address, settings.port)) {
    cout << "Cannot initialize dispatcher" << endl;
    return -1;
  }

  int res = -1;
  if (settings.operation == Operation::MakeDir) {
    res = dispatcher.MakeDir(settings.filename);
  } else if (settings.operation == Operation::Lookup) {
    res = dispatcher.Lookup(settings.filename);
  } else if (settings.operation == Operation::Enumerate) {
    res = dispatcher.Enumerate(settings.filename);
  } else if (settings.operation == Operation::PutFile) {
    res = dispatcher.PutFile(settings.filename, settings.dstfile, true);
  } else if (settings.operation == Operation::GetFile) {
    res = dispatcher.GetFile(settings.filename, settings.dstfile);
  } else if (settings.operation == Operation::DeleteDir) {
    res = dispatcher.DeleteDir(settings.filename);
  } else if (settings.operation == Operation::DeleteFile) {
    res = dispatcher.DeleteFile(settings.filename);
  } else if (settings.operation == Operation::Ioctl) {
    res = dispatcher.CountFiles(settings.filename);
  } else if (settings.operation == Operation::PutBuffer) {
    char data[settings.size];
    res = dispatcher.PutBuffer(data, settings.size, settings.filename, false);
  } else if (settings.operation == Operation::GetBuffer) {
    char data[settings.size];
    res = dispatcher.GetBuffer(data, settings.size, settings.filename);
  } else if (settings.operation == Operation::PutBenchmark ||
             settings.operation == Operation::GetBenchmark) {
    vector<string> names;
    for (int i = 0; i < settings.loop; i++) {
      string tmp = settings.filename;
      tmp.append(to_string(i));
      names.push_back(tmp);
    }

    char data[settings.size];
    for (string n : names) {
      if (settings.operation == Operation::PutBenchmark) {
        res = dispatcher.PutBuffer(data, settings.size, n, true);
      } else if (settings.operation == Operation::GetBenchmark) {
        res = dispatcher.GetBuffer(data, settings.size, n);
      } else {
        cout << "No valid operation defined " << endl;
        break;
      }
      if (res < 0) {
        cout << "Error when putting file" << endl;
        break;
      }
    }
  }

  if (res >= 0) {
    cout << "pocket operation successful, res " << res << endl;
  } else {
    cout << "pocket operation failed, res " << res << endl;
  }

  return 0;
}
