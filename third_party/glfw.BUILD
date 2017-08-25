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

load("@rules_system//system:defs.bzl", "cmake_configure_file")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # zlib

_GLFW_COMMON_SOURCES = [
    "context.c",
    "init.c",
    "input.c",
    "monitor.c",
    "vulkan.c",
    "window.c",
    ":glfw_config",
]

_GLFW_COMMON_COPTS = [
    "-D_GLFW_USE_CONFIG_H",
]

_GLFW_WIN32_SOURCES = _GLFW_COMMON_SOURCES + [
    "win32_platform.h",
    "win32_joystick.h",
    "wgl_context.h",
    "egl_context.h",
    "win32_init.c",
    "win32_joystick.c",
    "win32_monitor.c",
    "win32_time.c",
    "win32_tls.c",
    "win32_window.c",
    "wgl_context.c",
    "egl_context.c",
]

_GLFW_WIN32_COPTS = _GLFW_COMMON_COPTS + [
    "-lgdi32",
]

_GLFW_WIN32_DEPS = []

_GLFW_X11_SOURCES = _GLFW_COMMON_SOURCES + [
    "x11_platform.h",
    "xkb_unicode.h",
    "linux_joystick.h",
    "posix_time.h",
    "posix_tls.h",
    "glx_context.h",
    "egl_context.h",
    "x11_init.c",
    "x11_monitor.c",
    "x11_window.c",
    "xkb_unicode.c",
    "linux_joystick.c",
    "posix_time.c",
    "posix_tls.c",
    "glx_context.c",
    "egl_context.c",
]

_GLFW_X11_COPTS = _GLFW_COMMON_COPTS + [
    "-lrt",
    "-lm",
]

_GLFW_X11_DEPS = [
    ":x11",
    ":x11_xrandr",
    ":x11_xinerama",
    ":x11_xf86vmode",
    ":x11_xkb",
    ":x11_xcursor",
]

cc_library(
    name = "glfw",
    srcs = select({
        "@rules_system//platform:windows": _GLFW_WIN32_SOURCES,
        "//conditions:default": _GLFW_X11_SOURCES,
    }),
    hdrs = [
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
    ],
    copts = select({
        "@rules_system//platform:windows": _GLFW_WIN32_COPTS,
        "//conditions:default": _GLFW_X11_COPTS,
    }),
    includes = ["include"],
    strip_include_prefix = "include",
    deps = select({
        "@rules_system//platform:windows": _GLFW_WIN32_DEPS,
        "//conditions:default": _GLFW_X11_DEPS,
    }),
)

cmake_configure_file(
    name = "glfw_config",
    at_only = True,
    output = "src/glfw_config.h",
    template = "src/glfw_config.h.in",
    vars = select({
        "@rules_system//platform:windows": {
            "_GLFW_WIN32": 1,
        },
        "//conditions:default": {
            "_GLFW_X11": 1,
        },
    }),
)
