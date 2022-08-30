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

#include <filesystem>

#include "include/proxy-wasm/wasifs.h"

namespace proxy_wasm {

int errnoToWASI(int eno);

WASIFileSystem::WASIFileSystem(const FileSystemConfig &config) {
  FSNode root;

  for (auto &host_file : config.host_files) {
    const auto &vm_path = !host_file.vm_path.empty()
                          ? std::filesystem::u8path(host_file.vm_path)
                          : std::filesystem::u8path(host_file.host_path);

    FSNode *node = &root;
    auto path_entry = vm_path.begin();
    if (vm_path.is_absolute()) {
      ++path_entry;
    }
    for (; path_entry != vm_path.end(); ++path_entry) {
      auto &new_node = node->entries[*path_entry];
      node = &new_node;
    }

    node->host_path = host_file.host_path;
  }

  root_ = std::move(root);

  opened_files_.emplace(3, OpenedFile("/", root_, NULL));
  last_fd_ = 3;
}

int WASIFileSystem::GetOpenedFile(uint32_t fd, OpenedFile **openedOut) {
  auto parentItr = opened_files_.find(fd);
  if (parentItr == opened_files_.end()) {
    return errnoToWASI(EBADF);
  }

  *openedOut = &parentItr->second;

  return 0;
}

int WASIFileSystem::OpenFile(uint32_t fd, std::string path_str, uint32_t *fdOut) {
  auto parentItr = opened_files_.find(fd);
  if (parentItr == opened_files_.end()) {
    return errnoToWASI(EBADF);
  }

  const std::filesystem::path &path = std::filesystem::u8path(path_str).lexically_normal();

  if (path.empty()) {
    // path is required
    return errnoToWASI(EINVAL);
  }

  auto *node = &parentItr->second.node;
  std::filesystem::path host_path;
  for (auto path_entry = path.begin(); path_entry != path.end(); ++path_entry) {
    if (*path_entry == "..") {
      // We already normalized the path so this can only be in the beginning of it.
      // Since path should always be relative to fd, it is invalid to try to navigate up from it.
      return errnoToWASI(EINVAL);
    }

    auto nodeItr = node->entries.find(*path_entry);
    if (nodeItr == node->entries.end()) {
      if (node->host_path.empty()) {
        return errnoToWASI(ENOENT);
      } else {
        host_path = node->host_path;
        for (; path_entry != path.end(); ++path_entry) {
          host_path = host_path / *path_entry;
        }
        break;
      }
    }

    node = &nodeItr->second;
    host_path = node->host_path;
  }

  FILE *file = fopen(host_path.c_str(), "r");
  if (file == NULL) {
    return errnoToWASI(errno);
  }
  uint32_t new_fd = ++last_fd_;
  opened_files_.emplace(new_fd, OpenedFile(parentItr->second.vm_path / path, *node, file));
  *fdOut = new_fd;
  return 0;
}

int WASIFileSystem::CloseFile(uint32_t fd) {
  auto parentItr = opened_files_.find(fd);
  if (parentItr == opened_files_.end()) {
    return errnoToWASI(EBADF);
  }

  auto &opened = parentItr->second;

  if (opened.file != NULL) {
    fclose(opened.file);
  }

  opened_files_.erase(fd);

  return 0;
}

int errnoToWASI(int eno) {
  switch (eno) {
  case E2BIG:
    return 1;
  case EACCES:
    return 2;
  case EADDRINUSE:
    return 3;
  case EADDRNOTAVAIL:
    return 4;
  case EAFNOSUPPORT:
    return 5;
  case EAGAIN:
    return 6;
  case EALREADY:
    return 7;
  case EBADF:
    return 8;
  case EBADMSG:
    return 9;
  case EBUSY:
    return 10;
  case ECANCELED:
    return 11;
  case ECHILD:
    return 12;
  case ECONNABORTED:
    return 13;
  case ECONNREFUSED:
    return 14;
  case ECONNRESET:
    return 15;
  case EDEADLK:
    return 16;
  case EDESTADDRREQ:
    return 17;
  case EDOM:
    return 18;
  case EDQUOT:
    return 19;
  case EEXIST:
    return 20;
  case EFAULT:
    return 21;
  case EFBIG:
    return 22;
  case EHOSTUNREACH:
    return 23;
  case EIDRM:
    return 24;
  case EILSEQ:
    return 25;
  case EINPROGRESS:
    return 26;
  case EINTR:
    return 27;
  case EINVAL:
    return 28;
  case EIO:
    return 29;
  case EISCONN:
    return 30;
  case EISDIR:
    return 31;
  case ELOOP:
    return 32;
  case EMFILE:
    return 33;
  case EMLINK:
    return 34;
  case EMSGSIZE:
    return 35;
  case EMULTIHOP:
    return 36;
  case ENAMETOOLONG:
    return 37;
  case ENETDOWN:
    return 38;
  case ENETRESET:
    return 39;
  case ENETUNREACH:
    return 40;
  case ENFILE:
    return 41;
  case ENOBUFS:
    return 42;
  case ENODEV:
    return 43;
  case ENOENT:
    return 44;
  case ENOEXEC:
    return 45;
  case ENOLCK:
    return 46;
  case ENOLINK:
    return 47;
  case ENOMEM:
    return 48;
  case ENOMSG:
    return 49;
  case ENOPROTOOPT:
    return 50;
  case ENOSPC:
    return 51;
  case ENOSYS:
    return 52;
  case ENOTCONN:
    return 53;
  case ENOTDIR:
    return 54;
  case ENOTEMPTY:
    return 55;
  case ENOTRECOVERABLE:
    return 56;
  case ENOTSOCK:
    return 57;
  case ENOTSUP:
    return 58;
  case ENOTTY:
    return 59;
  case ENXIO:
    return 60;
  case EOVERFLOW:
    return 61;
  case EOWNERDEAD:
    return 62;
  case EPERM:
    return 63;
  case EPIPE:
    return 64;
  case EPROTO:
    return 65;
  case EPROTONOSUPPORT:
    return 66;
  case EPROTOTYPE:
    return 67;
  case ERANGE:
    return 68;
  case EROFS:
    return 69;
  case ESPIPE:
    return 70;
  case ESRCH:
    return 71;
  case ESTALE:
    return 72;
  case ETIMEDOUT:
    return 73;
  case ETXTBSY:
    return 74;
  case EXDEV:
    return 75;
  default:
    return 21; // __WASI_EFAULT;
  }
}

}