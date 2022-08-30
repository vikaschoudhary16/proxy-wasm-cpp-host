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

#include "include/proxy-wasm/wasifs.h"

#include "gtest/gtest.h"

namespace proxy_wasm {

TEST(WASIFileSystem, Empty) {
  FileSystemConfig config;
  WASIFileSystem fs(config);

  OpenedFile *opened;
  int res = fs.GetOpenedFile(3, &opened);
  EXPECT_EQ(0, res);
  EXPECT_EQ("/", opened->vm_path);
  EXPECT_EQ(NULL, opened->file);
  EXPECT_EQ("", opened->node.host_path);
  EXPECT_TRUE(opened->node.entries.empty());

  res = fs.GetOpenedFile(4, &opened);
  EXPECT_EQ(8, res);

  uint32_t fd;
  res = fs.OpenFile(4, "/animals/mammals/bear.txt", &fd);
  EXPECT_EQ(8, res);

  res = fs.OpenFile(3, "/animals/mammals/bears.txt", &fd);
  EXPECT_EQ(44, res);

  res = fs.OpenFile(3, "", &fd);
  EXPECT_EQ(28, res);

  res = fs.OpenFile(3, "../password.txt", &fd);
  EXPECT_EQ(28, res);

  res = fs.OpenFile(3, "././../password.txt", &fd);
  EXPECT_EQ(28, res);

  res = fs.OpenFile(3, "././../foo/../password.txt", &fd);
  EXPECT_EQ(28, res);
}

TEST(WASIFileSystem, OnlyFiles) {
  FileSystemConfig config;
  config.host_files.push_back(
      {
          "test/test_data/fs/animals/mammals/apes.txt",
          "/animals/mammals/apes.txt"
      });
  config.host_files.push_back(
      {
          "test/test_data/fs/animals/mammals/bears.txt",
          "/animals/mammals/bears.txt"
      });
  WASIFileSystem fs(config);

  OpenedFile *opened;
  int res = fs.GetOpenedFile(3, &opened);
  EXPECT_EQ(0, res);
  EXPECT_FALSE(opened->node.entries.empty());

  uint32_t fd;
  res = fs.OpenFile(3, "animals/mammals/apes.txt", &fd);
  EXPECT_EQ(0, res);
  EXPECT_NE(3, fd);
  res = fs.GetOpenedFile(fd, &opened);
  EXPECT_EQ("/animals/mammals/apes.txt", opened->vm_path);
  EXPECT_NE((FILE *)NULL, opened->file);
  EXPECT_EQ("test/test_data/fs/animals/mammals/apes.txt", opened->node.host_path);

  res = fs.CloseFile(fd);
  EXPECT_EQ(0, res);
  res = fs.GetOpenedFile(fd, &opened);
  EXPECT_EQ(8, res);

  res = fs.OpenFile(3, "animals/mammals/bears.txt", &fd);
  EXPECT_EQ(0, res);
  res = fs.CloseFile(fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/cats.txt", &fd);
  EXPECT_EQ(44, res);
}

TEST(WASIFileSystem, OneDir) {
  FileSystemConfig config;
  config.host_files.push_back(
      {
          "test/test_data/fs/animals",
          "/animals"
      });
  WASIFileSystem fs(config);

  OpenedFile *opened;
  int res = fs.GetOpenedFile(3, &opened);
  EXPECT_EQ(0, res);
  EXPECT_FALSE(opened->node.entries.empty());

  uint32_t fd;
  res = fs.OpenFile(3, "animals/mammals/apes.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/bears.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/cartoons.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/dinosaurs.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/cats.txt", &fd);
  EXPECT_EQ(44, res);
}

TEST(WASIFileSystem, TwoDirs) {
  FileSystemConfig config;
  config.host_files.push_back(
      {
          "test/test_data/fs/animals/birds",
          "/animals/birds"
      });
  config.host_files.push_back(
      {
          "test/test_data/fs/animals/mammals",
          "/animals/mammals"
      });
  WASIFileSystem fs(config);

  OpenedFile *opened;
  int res = fs.GetOpenedFile(3, &opened);
  EXPECT_EQ(0, res);
  EXPECT_FALSE(opened->node.entries.empty());

  uint32_t fd;
  res = fs.OpenFile(3, "animals/mammals/apes.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/bears.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/cartoons.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/dinosaurs.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/cats.txt", &fd);
  EXPECT_EQ(44, res);
}

TEST(WASIFileSystem, OverlappingDirs) {
  FileSystemConfig config;
  config.host_files.push_back(
      {
          "test/test_data/fs/animals",
          "/animals"
      });
  config.host_files.push_back(
      {
          "test/test_data/fs/animals/mammals",
          "/animals/mammals"
      });
  WASIFileSystem fs(config);

  OpenedFile *opened;
  int res = fs.GetOpenedFile(3, &opened);
  EXPECT_EQ(0, res);
  EXPECT_FALSE(opened->node.entries.empty());

  uint32_t fd;
  res = fs.OpenFile(3, "animals/mammals/apes.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/bears.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/cartoons.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/birds/dinosaurs.txt", &fd);
  EXPECT_EQ(0, res);

  res = fs.OpenFile(3, "animals/mammals/cats.txt", &fd);
  EXPECT_EQ(44, res);
}

}