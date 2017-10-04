# MIT License
#
# Copyright (c) 2017 Vladimir Antonov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# https://github.com/antonovvk/bazel_glog/blob/master/glog.BUILD

load("@com_github_renatoutsch_herakles//third_party:config.bzl", "cc_fix_config")
load("@com_github_renatoutsch_rules_system//system:defs.bzl", "system_select")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

_THREAD_COPTS = system_select({
    "windows": [],
    "default": ["-pthread"],
})

_THREAD_LINKOPTS = system_select({
    "windows": ["-lpsapi"],
    "macos": [],
    "default": ["-pthread"],
})

_WARNINGS_COPTS = system_select({
    "windows": [],
    "default": [
        "-Wno-deprecated-declarations",
        "-Wno-unused-function",
        "-Wno-unused-variable",
        "-Wno-sign-compare",
    ],
})

cc_library(
    name = "glog",
    hdrs = [
        "src/demangle.h",
        "src/mock-log.h",
        "src/stacktrace.h",
        "src/symbolize.h",
        "src/utilities.h",
        "src/base/commandlineflags.h",
        "src/base/googleinit.h",
        "src/base/mutex.h",
        "src/glog/log_severity.h",
        ":gen_headers",
    ],
    srcs = [
        "src/demangle.cc",
        "src/logging.cc",
        "src/raw_logging.cc",
        "src/signalhandler.cc",
        "src/symbolize.cc",
        "src/utilities.cc",
        "src/vlog_is_on.cc",
        ":gen_config",
    ],
    deps = [
        "@com_github_gflags_gflags//:gflags",
    ],
    includes = [
        "src",
    ],
    copts = _THREAD_COPTS + _WARNINGS_COPTS,
    linkopts = _THREAD_LINKOPTS,
)

cc_fix_config(
    name = "gen_headers",
    files = {
        "src/glog/logging.h.in": "src/glog/logging.h",
        "src/glog/raw_logging.h.in": "src/glog/raw_logging.h",
        "src/glog/stl_logging.h.in": "src/glog/stl_logging.h",
        "src/glog/vlog_is_on.h.in": "src/glog/vlog_is_on.h",
    },
    values = {
        "ac_cv_have_unistd_h": "1",
        "ac_cv_have_stdint_h": "1",
        "ac_cv_have_systypes_h": "1",
        "ac_cv_have_libgflags_h": "1",
        "ac_cv_have_libgflags": "1",
        "ac_cv_have_uint16_t": "1",
        "ac_cv_have___builtin_expect": "1",
        "ac_google_start_namespace": "namespace google {",
        "ac_google_end_namespace": "}",
        "ac_google_namespace": "google",
        "ac_cv___attribute___noinline": "__attribute__((noinline))",
        "ac_cv___attribute___noreturn": "__attribute__((noreturn))",
        "ac_cv___attribute___printf_4_5": "__attribute__((__format__ (__printf__, 4, 5)))",
    },
    includes = ["src"],
    visibility = ["//visibility:private"],
)

cc_fix_config(
    name = "gen_config",
    cmake = True,
    files = { "src/config.h.cmake.in": "config.h" },
    values = {
        "_START_GOOGLE_NAMESPACE_": "namespace google {",
        "_END_GOOGLE_NAMESPACE_": "}",
        "GOOGLE_NAMESPACE": "google",
        "GOOGLE_GLOG_DLL_DECL": "",
        "HAVE_DLADDR": "1",
        "HAVE_SNPRINTF": "1",
        "HAVE_DLFCN_H": "1",
        "HAVE_FCNTL": "1",
        "HAVE_GLOB_H": "1",
        "HAVE_INTTYPES_H": "1",
        "HAVE_LIBPTHREAD": "1",
        #"HAVE_LIBUNWIND_H": "1",
        "HAVE_LIB_GFLAGS": "1",
        #"HAVE_LIB_UNWIND": "1",
        "HAVE_MEMORY_H": "1",
        "HAVE_NAMESPACES": "1",
        "HAVE_PREAD": "1",
        "HAVE_PTHREAD": "1",
        "HAVE_PWD_H": "1",
        "HAVE_PWRITE": "1",
        "HAVE_RWLOCK": "1",
        "HAVE_SIGACTION": "1",
        "HAVE_SIGALTSTACK": "1",
        "HAVE_STDINT_H": "1",
        "HAVE_STRING_H": "1",
        "HAVE_SYS_SYSCALL_H": "1",
        "HAVE_SYS_TIME_H": "1",
        "HAVE_SYS_TYPES_H": "1",
        "HAVE_SYS_UCONTEXT_H": "1",
        "HAVE_SYS_UTSNAME_H": "1",
        "HAVE_UNISTD_H": "1",
        "HAVE_USING_OPERATOR": "1",
        "HAVE_HAVE___ATTRIBUTE___": "1",
        "HAVE_HAVE___BUILTIN_EXPECT": "1",
        #"NO_FRAME_POINTER": "1",
        "_GNU_SOURCE": "1",
    },
    includes = ["."],
    visibility = ["//visibility:private"],
)
