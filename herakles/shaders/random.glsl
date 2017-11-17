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
  * Random utilities for GLSL.
  * Before using any functions in this module, ensure randInit() is called with
  * a proper seed first, or the algorithms won't generate proper pseudo-random
  * numbers. Each shader work unit should have a different seed.
  * To save 
  */

#ifndef HERAKLES_SHADERS_RANDOM_GLSL
#define HERAKLES_SHADERS_RANDOM_GLSL

#include "extensions.glsl"

/// Maximum number representable by an uint.
const uint UINT_MAX = 4294967295U;

/// Multiplication parameter used in the RNG algorithm.
const uint RNG_PARAM_ = 4294883355U;

/// State used by the random algorithms in this module. Must be initialized with
/// a seed by calling randInit().
uvec2 RandState_;

/**
 * Initializes the random number generator with a 64 bits unsigned integer seed.
 * Ideally, this seed should come from random numbers generated from good
 * pseudo-random algorithms in the CPU.
 * @param seed Seed to be used in the random algorithms from this module.
 */
void randInit(uvec2 seed) {
  RandState_ = seed;
}

/**
 * Returns the current seed to be saved for a next shader invocation.
 */
uvec2 randState() {
  return RandState_;
}

/**
 * Generates uniformly distributed random integers in the range [0, UINT_MAX].
 * The algorithm used is the MWC64X, taken from:
 * http://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html
 *
 * Be sure to have called randInit() before calling this function.
 *
 * @return The next unsigned integer from the RNG.
 */
uint urand() {
  const uint result = RandState_.x ^ RandState_.y;  // Calculate the result.

  uint lsb, msb;
  umulExtended(RandState_.x, RNG_PARAM_, msb, lsb);  // Step the RNG.

  // Pack the state back up.
  RandState_.x = lsb + RandState_.y; 
  RandState_.y = msb + (RandState_.x < RandState_.y ?
                        RandState_.x : RandState_.y);

  return result;
}

/**
 * Generates a random unsigned integer in the range [0, end).
 * Ensure randInit() is called before calling this function.
 */
uint urand(uint end) {
  return urand() % end;
}

/**
 * Generates a random unsigned integer in the range [begin, end).
 * Ensure randInit() is called before calling this function.
 */
uint urand(uint begin, uint end) {
  return urand() % (end - begin) + begin;
}

/**
 * Generates a random floating point number in the range [0, 1].
 * Ensure randInit() is called before calling this function.
 */
float rand() {
  return float(urand()) / UINT_MAX;
}

#endif
