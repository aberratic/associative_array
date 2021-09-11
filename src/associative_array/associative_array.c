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

#include "associative_array/associative_array.h"

static unsigned int __attribute__((pure))
_calculate_bitstr_size(unsigned int capacity) {
  unsigned int size = capacity / (sizeof(unsigned int) * 8);
  if (capacity % (sizeof(unsigned int) * 8) != 0)
    size++;
  return size;
}

static asa_unit_t *_get_unit_by_index(const asa_t *const map,
                                      unsigned int index) {
  return map->_buckets + index;
}

static int _get_index_by_key(const asa_t *const map, const void *const key) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  for (unsigned int i = 0; i != map->_capacity; i++) {
    if (bstr_get(map->_used_buckets, i)) {
      asa_unit_t *target = map->_buckets + i;
      if (map->_comperator(key, target->_key) == 0) {
        return i;
      }
    }
  }
  return -1;
}

asa_t *asa_create_map(unsigned int capacity, asa_cmp_keys_f *comperator) {
  asa_t *result = (asa_t *)malloc(sizeof(asa_t));
  if (result == NULL)
    return NULL;

  asa_unit_t *buckets = (asa_unit_t *)malloc(sizeof(asa_unit_t) * capacity);
  if (buckets == NULL) {
    free(result);
    return NULL;
  }

  bstr_bitstr_t *used = bstr_create_bitstr(_calculate_bitstr_size(capacity));
  if (used == NULL) {
    free(buckets);
    free(result);
    return NULL;
  }
  memset(buckets, 0, sizeof(asa_unit_t) * capacity);

  result->_comperator = comperator;
  result->_capacity = capacity;
  result->_buckets = buckets;
  result->_used_buckets = used;
  return result;
}

void asa_delete_map(asa_t *map) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  free(map->_buckets);
  bstr_delete_bitstr(map->_used_buckets);
  free(map);
  return;
}

asa_err_t asa_insert(asa_t *const map, void *const key, void *const value) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  if (asa_key_exists(map, key))
    return ASA_DUPLICATE_KEY;

  int free_bucket_index = bstr_ffus(map->_used_buckets);
  bstr_set(map->_used_buckets, free_bucket_index);
  if (free_bucket_index == -1)
    return ASA_NO_SPACE_LEFT;
  asa_unit_t entry = {._key = key, ._value = value};

  asa_unit_t *target = map->_buckets + free_bucket_index;
  *target = entry;
  return ASA_NONE;
}

asa_err_t asa_upsert(asa_t *const map, void *const key, void *const value) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  int index = _get_index_by_key(map, key);
  if (index == -1)
    return asa_insert(map, key, value);

  asa_unit_t *target = _get_unit_by_index(map, index);
  target->_value = value;
  return ASA_NONE;
}

asa_err_t asa_update(asa_t *const map, const void *const key,
                     void *const value) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  int index = _get_index_by_key(map, key);
  if (index == -1)
    return ASA_KEY_NOT_FOUND;

  asa_unit_t *target = _get_unit_by_index(map, index);
  target->_value = value;
  return ASA_NONE;
}

asa_err_t asa_remove(asa_t *const map, const void *const key) {
#ifdef DEBUG
  assert(map != NULL);
  assert(key != NULL);
#endif
  int index = _get_index_by_key(map, key);
  if (index == -1)
    return ASA_KEY_NOT_FOUND;
  bstr_clr(map->_used_buckets, index);
  asa_unit_t *target = map->_buckets + index;
  memset(target, 0, sizeof(asa_unit_t));
  return ASA_NONE;
}

bool asa_key_exists(const asa_t *const map, const void *const key) {
#ifdef DEBUG
  assert(map != NULL);
  assert(key != NULL);
#endif

  int index = _get_index_by_key(map, key);
  if (index == -1)
    return false;
  return true;
}

bool asa_is_empty(const asa_t *const map) {
  if (bstr_popcnt(map->_used_buckets) == 0)
    return true;
  return false;
}

asa_err_t asa_shrink_to_fit(asa_t *const map) {
  if (asa_is_empty(map))
    return ASA_NONE;
  int used_buckets = bstr_popcnt(map->_used_buckets);
  if (used_buckets == map->_capacity)
    return ASA_NONE;

  while (true) {
    int first_free_bucket = bstr_ffus(map->_used_buckets);
    if (first_free_bucket == -1 || first_free_bucket >= map->_capacity)
      break;
    bool found_used_bucket = false;
    for (int bit = first_free_bucket; bit != map->_capacity; bit++) {
      if (bstr_get(map->_used_buckets, bit)) {
        asa_unit_t *free_bucket = map->_buckets + first_free_bucket;
        asa_unit_t *used_bucket = map->_buckets + bit;
        memcpy(free_bucket, used_bucket, sizeof(asa_unit_t));
        memset(used_bucket, 0, sizeof(asa_unit_t));
        bstr_set(map->_used_buckets, first_free_bucket);
        bstr_clr(map->_used_buckets, bit);
        found_used_bucket = true;
        break;
      }
    }
    if (found_used_bucket == false)
      break;
  }
  map->_capacity = used_buckets;
  asa_unit_t *new_buckets =
      (asa_unit_t *)realloc(map->_buckets, map->_capacity * sizeof(asa_unit_t));
  if (new_buckets == NULL)
    return ASA_MALLOC_FAILED;
  map->_buckets = new_buckets;
  bstr_err_t bstrerr =
      bstr_resize(map->_used_buckets, _calculate_bitstr_size(used_buckets));
  if (bstrerr != BSTR_NO_ERROR)
    return ASA_DATASTRUCTURE_CORRUPTED;
  return ASA_NONE;
}

asa_err_t asa_reserve_space(asa_t *const map, unsigned int capacity) {
  if (map->_capacity == capacity)
    return ASA_NONE;
  asa_unit_t *newMem = realloc(map->_buckets, capacity * sizeof(asa_unit_t));
  if (newMem == NULL)
    return ASA_MALLOC_FAILED;
  if (capacity > map->_capacity) {
    for (int i = map->_capacity; i != capacity; i++) {
      asa_unit_t *target = newMem + i;
      asa_unit_t nullunit = {._key = NULL, ._value = NULL};
      *target = nullunit;
    }
  }

  map->_buckets = newMem;
  map->_capacity = capacity;

  if (bstr_resize(map->_used_buckets, _calculate_bitstr_size(capacity)) !=
      BSTR_NO_ERROR) {
    return ASA_DATASTRUCTURE_CORRUPTED;
  }
  return ASA_NONE;
}

unsigned int asa_get_capacity(const asa_t *const map) { return map->_capacity; }

unsigned int asa_get_length(const asa_t *const map) {
  return bstr_popcnt(map->_used_buckets);
}

void *asa_get_value_by_key(const asa_t *const map, const void *const key) {
  int first_candidate = bstr_ffs(map->_used_buckets);
  if (first_candidate == -1)
    return NULL;

  for (int i = first_candidate; i < map->_capacity; i++) {
    asa_unit_t *candidate = map->_buckets + i;
    if (map->_comperator(key, candidate->_key) == 0)
      return candidate->_value;
  }
  return NULL;
}

asa_iterator_t asa_new_iterator(const asa_t *const map) {
  return bstr_ffs(map->_used_buckets);
}

asa_iterator_t asa_foreach(const asa_t *const map, void **key, void **value,
                           asa_iterator_t offset) {
#ifdef DEBUG
  assert(map != NULL);
#endif
  asa_iterator_t next = bstr_next_set_bit(map->_used_buckets, offset);
  if (next == -1)
    return next;

  asa_unit_t *target = _get_unit_by_index(map, next);
  *key = target->_key;
  *value = target->_value;

  return next + 1;
}
