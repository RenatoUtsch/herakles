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

/**
 * Scene structures and functions to simplify using Herakles scenes in GLSL.
 *
 * Do NOT change the order of the elements in these structs, unless you know
 * what you're doing. Changing this without changing the C++ code and scene file
 * format will break the code because you're changing the binary format.
 */

#ifndef HERAKLES_SHADERS_EXTENSIONS_GLSL
#define HERAKLES_SHADERS_EXTENSIONS_GLSL

#version 450
#extension GL_ARB_gpu_shader5 : require
#extension GL_NV_shader_atomic_float : enable

#endif // !HERAKLES_SHADERS_EXTENSIONS_GLSL
