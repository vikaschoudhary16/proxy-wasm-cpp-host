// Copyright 2016-2019 Envoy Project Authors
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdio>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace proxy_wasm {

struct HostFile {
  std::string host_path;
  std::string vm_path;
};
struct FileSystemConfig {
  std::vector<HostFile> host_files;
};

struct FSNode {
  std::filesystem::path host_path;
  std::unordered_map<std::string, FSNode> entries;
};

struct OpenedFile {
  OpenedFile(const std::filesystem::path vm_path, const FSNode &node, FILE *file): vm_path(vm_path), node(node), file(file) {}

  const std::filesystem::path vm_path;
  const FSNode &node;
  FILE *file;
};

class WASIFileSystem {
public:
  WASIFileSystem(const FileSystemConfig &config);

  int GetOpenedFile(uint32_t fd, OpenedFile **openedOut);
  int OpenFile(uint32_t fd, std::string path, uint32_t *fdOut);
  int CloseFile(uint32_t fd);

private:
  FSNode root_;

  std::unordered_map<uint32_t, OpenedFile> opened_files_;

  uint32_t last_fd_;
};

}
