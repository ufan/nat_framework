/*
 * test2.cpp
 *
 *  Created on: 2018年8月8日
 *      Author: hongxu
 *
 * Test CTimer
 */

#include <iostream>
#include <thread>

#include "CTimer.h"
#include "Logger.h"
using namespace std;

inline string calcTime2(string date, int diff, string format = "%Y%m%d") {
  long t = parseTime(date.c_str(), format.c_str());
  cout << t << endl;
  t += diff * (24L * 3600L * 1000000000L);
  cout << t << endl;
  return parseNano(t, format.c_str());
}

void testlog() {
  for (int i = 0; i < 200000; i++) {
    LOG_DBG("test %d", 1);
  }
}

int main() {
  initLogger("./l.cnf");

  thread th1(testlog);
  thread th2(testlog);

  th1.join();
  th2.join();

  cout << "end of main" << endl;

  return 0;
}
