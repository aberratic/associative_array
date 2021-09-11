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
#include "unity.h"

ASA_CREATE_POINTER_TO_INTEGER_COMPERATOR(uint32_t);

void test_asa_create_map(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  asa_delete_map(map);
}

void test_asa_insert(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_TRUE(asa_is_empty(map));

  uint32_t key = 23;
  uint32_t val = 42;
  asa_err_t err = asa_insert(map, (void *)&key, (void *)&val);
  TEST_ASSERT_EQUAL_UINT(ASA_NONE, err);
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_TRUE(asa_key_exists(map, (void *)&key));

  err = asa_insert(map, (void *)&key, (void *)&val);
  TEST_ASSERT_EQUAL_INT(ASA_DUPLICATE_KEY, err);

  TEST_ASSERT_EQUAL_UINT32(
      val, (*(uint32_t *)asa_get_value_by_key(map, (void *)&key)));
  asa_delete_map(map);
}

void test_asa_upsert(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_TRUE(asa_is_empty(map));

  uint32_t key = 23;
  uint32_t val = 42;
  uint32_t val2 = 43;

  asa_err_t err = asa_upsert(map, (void *)&key, (void *)&val);
  TEST_ASSERT_EQUAL_UINT(ASA_NONE, err);
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_TRUE(asa_key_exists(map, (void *)&key));

  err = asa_upsert(map, (void *)&key, (void *)&val2);
  TEST_ASSERT_EQUAL_INT(ASA_NONE, err);

  TEST_ASSERT_EQUAL_UINT32(
      val2, (*(uint32_t *)asa_get_value_by_key(map, (void *)&key)));
  asa_delete_map(map);
}

void test_asa_update(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_TRUE(asa_is_empty(map));

  uint32_t key = 23;
  uint32_t val = 42;
  uint32_t val2 = 43;

  asa_err_t err = asa_insert(map, (void *)&key, (void *)&val);
  TEST_ASSERT_EQUAL_INT(ASA_NONE, err);
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_TRUE(asa_key_exists(map, (void *)&key));

  err = asa_update(map, (void *)&key, (void *)&val2);
  TEST_ASSERT_EQUAL_INT(ASA_NONE, err);

  TEST_ASSERT_EQUAL_UINT32(
      val2, (*(uint32_t *)asa_get_value_by_key(map, (void *)&key)));
  asa_delete_map(map);
}

void test_asa_remove(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_TRUE(asa_key_exists(map, (void *)&key));
  asa_remove(map, (void *)&key);
  TEST_ASSERT_FALSE(asa_key_exists(map, (void *)&key));
  asa_delete_map(map);
}

void test_asa_key_exists(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_TRUE(asa_key_exists(map, (void *)&key));
  asa_remove(map, (void *)&key);
  TEST_ASSERT_FALSE(asa_key_exists(map, (void *)&key));
  asa_delete_map(map);
}

void test_asa_is_empty(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_FALSE(asa_is_empty(map));
  asa_remove(map, (void *)&key);
  TEST_ASSERT_TRUE(asa_is_empty(map));
  asa_delete_map(map);
}

void test_asa_shrink_to_fit(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_UINT32(0, asa_get_length(map));
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_EQUAL_UINT32(16, asa_get_capacity(map));
  TEST_ASSERT_EQUAL_UINT32(1, asa_get_length(map));
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_shrink_to_fit(map));
  TEST_ASSERT_EQUAL_UINT32(1, asa_get_capacity(map));

  asa_delete_map(map);
}

void test_asa_reserve_space(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_EQUAL_UINT32(16, asa_get_capacity(map));

  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_reserve_space(map, 64));
  TEST_ASSERT_EQUAL_UINT32(64, asa_get_capacity(map));

  asa_delete_map(map);
}

void test_asa_get_capacity(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_EQUAL_UINT32(16, asa_get_capacity(map));

  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_reserve_space(map, 64));
  TEST_ASSERT_EQUAL_UINT32(64, asa_get_capacity(map));

  asa_delete_map(map);
}

void test_asa_get_length(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  TEST_ASSERT_EQUAL_UINT(0, asa_get_length(map));
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_FALSE(asa_is_empty(map));
  TEST_ASSERT_EQUAL_UINT(1, asa_get_length(map));
  asa_delete_map(map);
}

void test_asa_get_value_by_key(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t key = 23;
  uint32_t val = 42;
  TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, (void *)&key, (void *)&val));
  TEST_ASSERT_EQUAL_UINT(
      val, *(uint32_t *)(asa_get_value_by_key(map, (void *)&key)));
  asa_delete_map(map);
}

void test_asa_foreach(void) {
  asa_t *map = asa_create_map(16, &asa_comperator_uint32_t);
  TEST_ASSERT_NOT_NULL(map);
  uint32_t **keys =
      (uint32_t **)malloc(sizeof(uint32_t *) * asa_get_capacity(map));

  uint32_t **values =
      (uint32_t **)malloc(sizeof(uint32_t *) * asa_get_capacity(map));

  for (uint32_t i = 0; i < asa_get_capacity(map); i++) {
    uint32_t *key = (uint32_t *)malloc(sizeof(uint32_t));
    uint32_t *value = (uint32_t *)malloc(sizeof(uint32_t));
    *(keys + i) = key;
    *(values + i) = value;
    *key = i;
    *value = i;
    TEST_ASSERT_EQUAL_INT(ASA_NONE, asa_insert(map, key, value));
  }

  for (uint32_t i = 0; i < asa_get_capacity(map); i++) {
    if (i % 2 == 0) {
      asa_remove(map, *(keys + i));
    }
  }
  uint32_t *test_key = NULL;
  uint32_t *test_val = NULL;
  for (asa_iterator_t it = asa_new_iterator(map);
       (it = asa_foreach(map, (void **)&test_key, (void **)&test_val, it)) !=
       -1;) {
    TEST_ASSERT_EQUAL_PTR(*(keys + it - 1), test_key);
    TEST_ASSERT_EQUAL_PTR(*(values + it - 1), test_val);
    TEST_ASSERT_EQUAL_UINT(**(keys + it - 1), *test_key);
    TEST_ASSERT_EQUAL_UINT(**(values + it - 1), *test_val);
  }

  for (uint32_t i = 0; i < asa_get_capacity(map); i++) {
    free(*(keys + i));
    free(*(values + i));
  }
  free(keys);
  free(values);
  asa_delete_map(map);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_asa_create_map);
  RUN_TEST(test_asa_insert);
  RUN_TEST(test_asa_upsert);
  RUN_TEST(test_asa_update);
  RUN_TEST(test_asa_remove);
  RUN_TEST(test_asa_key_exists);
  RUN_TEST(test_asa_is_empty);
  RUN_TEST(test_asa_shrink_to_fit);
  RUN_TEST(test_asa_reserve_space);
  RUN_TEST(test_asa_get_capacity);
  RUN_TEST(test_asa_get_length);
  RUN_TEST(test_asa_get_value_by_key);
  RUN_TEST(test_asa_foreach);
  UNITY_END();
}
