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

/// Maximum number representable by an uint.
const uint UINT_MAX = 4294967295;

/// Seed used by the random algorithms in this module.
uint RandSeed_;

/**
 * Generates uniformly distributed random integers in the range [0, UINT_MAX].
 * Taken from:
 * http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
 *
 * TODO(renatoutsch): this function is no good at all. Find a way to generate
 * native floating point numbers with a good prng with uniformly distributed
 * numbers. Maybe a good idea:
 * https://math.stackexchange.com/questions/337782/pseudo-random-number-generation-on-the-gpu
 * https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
 * http://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html
 * http://www0.cs.ucl.ac.uk/staff/ucacbbl/ftp/papers/langdon_2009_CIGPU.pdf
 *
 * @param seed Seed to use to generate the next unsigned integer.
 * @return The next unsigned integer from the hash algorithm, can be used again
 *   as seed in a next call.
 */
uint wangHash_(uint seed) {
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

/**
 * Initializes the random number generator with an unsigned integer seed.
 * Ideally, this seed should come from random numbers generated from good
 * pseudo-random algorithms in the CPU.
 * @param seed Seed to be used in the random algorithms from this module.
 */
void randInit(uint seed) {
  RandSeed_ = seed;
}

/**
 * Returns the current seed to be saved for a next shader invocation.
 */
uint randSeed() {
  return RandSeed_;
}

/**
 * Generates a random unsigned integer in the range [0, UINT_MAX].
 * Ensure randInit() is called before calling this function.
 */
uint urand() {
  RandSeed_ = wangHash_(RandSeed_);
  return RandSeed_;
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
