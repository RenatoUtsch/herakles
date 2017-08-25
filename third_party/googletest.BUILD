# Copyright 2017 Renato Utsch
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD 3-clause

_LINK_DEFS = select({
    ":windows": ["-lpsapi"],
    "//conditions:default": ["-lpthread"],
})

cc_library(
    name = "gmock",
    srcs = glob(
        [
            "googlemock/src/*.cc",
            "googlemock/src/*.h",
        ],
        exclude = [
            "googlemock/src/gmock-all.cc",
        ],
    ),
    hdrs = glob(["googlemock/include/gmock/**/*.h"]),
    strip_include_prefix = "googlemock/include",
    deps = [
        ":gtest",
    ],
)

cc_library(
    name = "gtest",
    srcs = glob(
        [
            "googletest/src/*.cc",
            "googletest/src/*.h",
        ],
        exclude = [
            "googletest/src/gtest-all.cc",
        ],
    ),
    hdrs = glob(["googletest/include/gtest/**/*.h"]),
    includes = ["googletest"],
    linkopts = _LINK_DEFS,
    strip_include_prefix = "googletest/include",
)

cc_test(
    name = "gmock_test",
    size = "small",
    srcs = [
        "googlemock/test/gmock_test.cc",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":gmock",
        ":gtest",
    ],
)

cc_test(
    name = "gtest_test",
    size = "small",
    srcs = [
        "googletest/test/gtest_unittest.cc",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":gtest",
    ],
)

config_setting(
    name = "windows",
    values = {"cpu": "x64_windows_msvc"},
    visibility = ["//visibility:private"],
)
