/*
 * @Author: Yeonon
 * @Date: 2022-05-21 17:11:41
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-05-21 20:45:07
 * @FilePath: /test/unittest/testUtils.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-05-21 17:11:41
 */
#pragma once
#include <iostream>

static unsigned int g_testCnt = 0;
static unsigned int g_passCnt = 0;
static unsigned int g_failCnt = 0;

#define PRINT_FAILED_RESULT(fnname, expect, actual)                            \
  std::cout << "(" << g_testCnt << ") " << fnname                              \
            << " failed, expect: " << expect << ", actual: " << actual         \
            << std::endl

#define PRINT_FAILED_RESULT_OF_POINTER(fnname, actual)                         \
  std::cout << "(" << g_testCnt << ") " << fnname                              \
            << " failed, actual: " << actual << std::endl;

template <typename T, typename Fn>
bool _ASSERT(T expect, T actual, Fn &&f, const std::string &testName) {
  g_testCnt++;
  if (f(expect, actual)) {
    g_passCnt++;
    return true;
  }
  PRINT_FAILED_RESULT(testName, expect, actual);
  g_failCnt++;
  return false;
}

template <typename T> bool ASSERT_NULL(T actual) {
  g_testCnt++;
  if (actual == nullptr) {
    g_passCnt++;
    return true;
  }
  g_failCnt++;
  PRINT_FAILED_RESULT_OF_POINTER("ASSERT_NULL", actual);
  return false;
}

template <typename T> bool ASSERT_NOT_NULL(T actual) {
  g_testCnt++;
  if (actual != nullptr) {
    g_passCnt++;
    return true;
  }
  g_failCnt++;
  PRINT_FAILED_RESULT_OF_POINTER("ASSERT_NOT_NULL", actual);
  return false;
}

template <typename T> bool ASSERT_EQ(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return e == a; }, "ASSERT_EQ");
}

template <typename T> bool ASSERT_NE(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return e != a; }, "ASSERT_NE");
}

template <typename T> bool ASSERT_LT(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return a < e; }, "ASSERT_LT");
}

template <typename T> bool ASSERT_GT(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return a > e; }, "ASSERT_GT");
}

template <typename T> bool ASSERT_LT_OR_EQ(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return a <= e; }, "ASSERT_LT_OR_EQ");
}

template <typename T> bool ASSERT_GT_OR_EQ(T expect, T actual) {
  return _ASSERT(
      expect, actual, [](T &e, T &a) { return a >= e; }, "ASSERT_GT_OR_EQ");
}

void PRINT_RESULT() {
  std::cout << "Test Resul: \n"
            << "pass count: " << g_passCnt << "\n"
            << "failed count : " << g_failCnt << "\n"
            << "total count : " << g_testCnt << "\n";

  g_passCnt = 0;
  g_failCnt = 0;
  g_testCnt = 0;
}
