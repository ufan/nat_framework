/*
 * CTimer.h
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#ifndef SRC_TIMER_CTIMER_H_
#define SRC_TIMER_CTIMER_H_

#include <chrono>
#include <time.h>
#include <string>
using namespace std;

class CTimer
{
	CTimer();
public:
	long getNano() const;

	long getDayBeginTime() const {return day_begin_time_;}

	void setTime(long now_nano);

	static CTimer& instance() {return instance_;}

private:
	long sec_diff_;

	long day_begin_time_;

	static CTimer instance_;
};

/**
 * dump long time to string with format
 * @param nano nano time in long
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return string-formatted time
 */
inline string parseNano(long nano, const char* format)
{
    if (nano <= 0)
        return string("NULL");
    nano /= 1000000000L;
    struct tm * dt;
    char buffer [30];
    dt = localtime(&nano);
    strftime(buffer, sizeof(buffer), format, dt);
    return string(buffer);
}

/**
 * parse struct tm to nano time
 * @param _tm ctime struct
 * @return nano time in long
 */
inline long parseTm(struct tm _tm)
{
    return timelocal(&_tm) * 1000000000L;
}

/**
 * parse string time to nano time
 * @param timeStr string-formatted time
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return nano time in long
 */
inline long parseTime(const char* timeStr, const char* format)
{
    struct tm _tm;
    strptime(timeStr, format, &_tm);
    return parseTm(_tm);
}

inline string calcDate(string date, int diff)
{
	date += " 00:00:00";
	long t = parseTime(date.c_str(), "%Y%m%d %H:%M:%S");
	t += diff * (24L * 3600L * 1000000000L);
	return parseNano(t, "%Y%m%d");
}

#define elapse_begin(val_name) long val_name = CTimer::instance().getNano();
#define elapse_end(val_name) val_name = CTimer::instance().getNano() - val_name;

#endif /* SRC_TIMER_CTIMER_H_ */

