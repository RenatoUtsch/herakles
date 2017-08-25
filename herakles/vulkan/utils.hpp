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

#ifndef HERAKLES_HERAKLES_VULKAN_UTILS_HPP
#define HERAKLES_HERAKLES_VULKAN_UTILS_HPP

#include <sstream>
#include <vector>

namespace hk {

/**
 * Joins an iterable into a single string.
 * @param iterable Iterable to be joined.
 * @param join String to be added between each element in iterable.
 */
template <class Iterable>
std::string stringJoin(const Iterable &iterable, std::string join) {
  std::stringstream ss;
  for (auto it = iterable.cbegin(); it != iterable.cend(); ++it) {
    ss << *it;
    if (it + 1 != iterable.cend()) {
      ss << join;
    }
  }

  return ss.str();
}

/// Reads a binary file from filename.
std::vector<char> readBinaryFromFile(const std::string &filename);

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_UTILS_HPP
