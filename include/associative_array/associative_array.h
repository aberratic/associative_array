/*
MIT License

Copyright (c) 2021 Stefan Luecke <git@aberrational.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ASSOCIATIVE_ARRAY_H
#define ASSOCIATIVE_ARRAY_H

#include "bitstring.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#ifdef DEBUG
#include "assert.h"
#endif
#include "memory.h"
#include "stdbool.h"
#include "stdlib.h"

/**
 * @brief Creates a simple comperator function which expects two pointer to
 * type types. Only integer types are supported.
 *
 * @param type Which type the comperator should use
 */
#define ASA_CREATE_POINTER_TO_INTEGER_COMPERATOR(type)                         \
  __attribute__((nonnull(1, 2))) int asa_comperator_##type(const void *aptr,   \
                                                           const void *bptr) { \
    type a = *(type *)aptr;                                                    \
    type b = *(type *)bptr;                                                    \
    if (a > b)                                                                 \
      return -1;                                                               \
    if (a == b)                                                                \
      return 0;                                                                \
    return 1;                                                                  \
  }

/**
 * @brief Definition of the needed comperator function.
 *
 * @return typedef
 */
typedef int asa_cmp_keys_f(const void *, const void *);

/**
 * @brief Enumeratio of error values that could occur at some functions.
 *
 */
typedef enum asa_err_t {
  /**
   * @brief No error happend. :)
   *
   */
  ASA_NONE = 0,
  /**
   * @brief Memory allocation failed. Free up some memory an try again.
   *
   */
  ASA_MALLOC_FAILED = -1,
  /**
   * @brief There is free bucket left. Either remove values or resize the
   * associative array
   *
   */
  ASA_NO_SPACE_LEFT = -2,
  /**
   * @brief Something aweful happend. Your tool should abort when this happens.
   *
   */
  ASA_DATASTRUCTURE_CORRUPTED = -3,
  /**
   * @brief The key you requested could not be found.
   *
   */
  ASA_KEY_NOT_FOUND = -4,
  /**
   * @brief The key you tried to insert is already there.
   *
   */
  ASA_DUPLICATE_KEY = -5,
} asa_err_t;

/**
 * @brief Compound type to hold the key and value pointer.
 *
 */
typedef struct asa_unit_t {
  void *_key;
  void *_value;
} asa_unit_t;

/**
 * @brief Iterator type. Used in conjunction with asa_foreach(). Create it with
 * asa_new_iterator()
 *
 */
typedef int asa_iterator_t;

/**
 * @brief Your main handle to an associative array. Create it with
 * asa_create_map()
 *
 */
typedef struct asa_t {
  unsigned int _capacity;
  asa_unit_t *_buckets;
  bstr_bitstr_t *_used_buckets;
  asa_cmp_keys_f *_comperator;
} asa_t;

/**
 * @brief Creates a new associative array object.
 *
 * @param capacity How many entries you want to save.
 * @param comperator Pointer to your comperator function. See asa_cmp_keys_f
 * @return asa_t* Pointer to your freshly generated array. NULL on allocation
 * failure.
 */
asa_t *asa_create_map(unsigned int capacity, asa_cmp_keys_f *comperator)
    __attribute__((warn_unused_result, nonnull(2)));

/**
 * @brief Deletes your array. ATTENTION: This does _NOT_ free your pointers that
 * were saved in this datastructure.
 *
 */
void asa_delete_map(asa_t *map) __attribute__((nonnull(1)));

/**
 * @brief Insert a key/value pair into the array.
 *
 * @return asa_err_t Returns ASA_DUPLICATE_KEY when the key is already there.
 * ASA_NO_SPACE_LEFT when no bucket is available. ASA_NONE on success.
 */
asa_err_t asa_insert(asa_t *const map, void *const key, void *const value)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief This function inserts a key/value pair if the key is not in the array.
 * It will update the values of a existing key.
 *
 * @return asa_err_t
 */
asa_err_t asa_upsert(asa_t *const map, void *const key, void *const value)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief This method will update a value referenced by the key. If the key is
 * not found ASA_KEY_NOT_FOUND is returned.
 *
 * @returns asa_err_t
 */
asa_err_t asa_update(asa_t *const map, const void *const key, void *const value)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief Removes a key/value pair from the map.
 *
 * @returns asa_err_t
 */
asa_err_t asa_remove(asa_t *const map, const void *const key)
    __attribute__((nonnull(1)));

/**
 * @brief Tells you whether a key exits or not.
 *
 * @return bool True when key exists
 */
bool asa_key_exists(const asa_t *const map, const void *const key)
    __attribute__((nonnull(1)));

/**
 * @brief True when the map is empty
 *
 * @return bool
 */
bool asa_is_empty(const asa_t *const map) __attribute__((nonnull(1)));

/**
 * @brief Defragments the array and frees up as much space as possible.
 *
 * @return asa_err_t
 */
asa_err_t asa_shrink_to_fit(asa_t *const map)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief Reserve space for later use
 *
 */
asa_err_t asa_reserve_space(asa_t *const map, unsigned int capacity)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief Returns how many buckets this array consists of.
 *
 */
unsigned int asa_get_capacity(const asa_t *const map)
    __attribute__((nonnull(1)));

/**
 * @brief Returns how many buckets are filled
 *
 */
unsigned int asa_get_length(const asa_t *const map) __attribute__((nonnull(1)));

/**
 * @brief Returns a value to a given key
 *
 * @return void* NULL when nothing found
 *
 */
void *asa_get_value_by_key(const asa_t *const map, const void *const key)
    __attribute__((nonnull(1)));

/**
 * @brief Creates a new iterator. It will point to the first value. If the array
 * is empty the iterator will have the value -1
 *
 */
asa_iterator_t asa_new_iterator(const asa_t *const map)
    __attribute__((nonnull(1)));

/**
 * @brief This function will iterate through the array.
 *
 */
asa_iterator_t asa_foreach(const asa_t *const map, void **key, void **value,
                           asa_iterator_t offset) __attribute__((nonnull(1)));

#endif
