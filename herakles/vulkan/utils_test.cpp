/*
 * Copyright 2017 Renato Utsch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "herakles/vulkan/utils.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {
using ::hk::stringJoin;

TEST(StringJoinTest, HandlesNoElements) {
  EXPECT_EQ("", stringJoin(std::vector<int>(), ", "));
}

TEST(StringJoinTest, HandlesOneElement) {
  EXPECT_EQ("12", stringJoin(std::vector<int>() = {12}, ", "));
}

TEST(StringJoinTest, HandlesMultipleElements) {
  const std::string expected = "a, bc, de";
  const std::string actual =
      stringJoin(std::vector<const char *>() = {"a", "bc", "de"}, ", ");

  EXPECT_EQ(expected, actual);
}

}  // namespace
