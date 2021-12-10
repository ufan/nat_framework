/*
 * CTimer.h
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#ifndef SRC_TIMER_CTIMER_H_
#define SRC_TIMER_CTIMER_H_

#include <time.h>

#include <chrono>
#include <string>
using namespace std;

class CTimer {
  CTimer();

 public:
  // The epoch time (in ns) since Unix epoch.
  // If REALTIME_TIMER defined, the real time clock is used for ticking, which
  // may be affected by NTP If not, the monotonic clock is used for tick, which
  // is not affected by NTP
  long getNano() const;

  long getDayBeginTime() const { return day_begin_time_; }

  // Synchronize to an external clock.
  // Default is synchronized to real time clock
  void setTime(long now_nano);

  static CTimer& instance() { return instance_; }

 private:
  // Time diff for synchrinaztion with another clock.
  // If REALTIME_TIMER defined, real time clock used, default sec_diff_=0
  // If not, monotonic clock is used, default sec_diff_ is the time diff between
  // REAL_TIME and MONOTONIC_TIME
  //
  // Note: it can also be synchronized to an external clock by using setTime
  long sec_diff_;

  // Epoch time in second at 00:00:00 of the day when the program using CTimer
  // started. This value is not updated during the lifecycle of the program.
  // Thus, user need to shutdown and restart the program everyday to get a
  // correct beginning time of each day.
  long day_begin_time_;

  static CTimer instance_;
};

/**
 * dump long time to string with format
 * @param nano nano time in long
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return string-formatted time
 */
inline string parseNano(long nano, const char* format) {
  if (nano <= 0) return string("NULL");
  nano /= 1000000000L;
  struct tm* dt;
  char buffer[30];
  dt = localtime(&nano);
  strftime(buffer, sizeof(buffer), format, dt);
  return string(buffer);
}

/**
 * parse struct tm to nano time
 * @param _tm ctime struct
 * @return nano time in long
 */
inline long parseTm(struct tm _tm) { return timelocal(&_tm) * 1000000000L; }

/**
 * parse string time to nano time
 * @param timeStr string-formatted time
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return nano time in long
 */
inline long parseTime(const char* timeStr, const char* format) {
  struct tm _tm;
  strptime(timeStr, format, &_tm);
  return parseTm(_tm);
}

inline string calcDate(string date, int diff) {
  date += " 00:00:00";
  long t = parseTime(date.c_str(), "%Y%m%d %H:%M:%S");
  t += diff * (24L * 3600L * 1000000000L);
  return parseNano(t, "%Y%m%d");
}

#define elapse_begin(val_name) long val_name = CTimer::instance().getNano();
#define elapse_end(val_name) val_name = CTimer::instance().getNano() - val_name;

#endif /* SRC_TIMER_CTIMER_H_ */
