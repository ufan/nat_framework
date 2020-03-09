/*
 * CTimer.cpp
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#include <string.h>
#include "CTimer.h"

#define REALTIME_TIMER

#ifdef REALTIME_TIMER
#define CTIMER_CLOCK_ID CLOCK_REALTIME
#else
#define CTIMER_CLOCK_ID CLOCK_MONOTONIC
#endif

CTimer CTimer::instance_;

inline std::chrono::steady_clock::time_point get_time_now()
{
    timespec tp;
    clock_gettime(CTIMER_CLOCK_ID, &tp);
    return std::chrono::steady_clock::time_point(
            std::chrono::steady_clock::duration(
                    std::chrono::seconds(tp.tv_sec) + std::chrono::nanoseconds(tp.tv_nsec)
            )
    );
}

inline long get_local_diff()
{
#ifdef REALTIME_TIMER
	return 0;
#else
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    auto now = std::chrono::steady_clock::time_point(
		std::chrono::steady_clock::duration(
				std::chrono::seconds(tp.tv_sec) + std::chrono::nanoseconds(tp.tv_nsec)
		)
    );
    long now_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
    		now.time_since_epoch()
    ).count();
    long _nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            get_time_now().time_since_epoch()
    ).count();
    return now_nano - _nano;
#endif
}

static long getDayBeginTimestamp()
{
	time_t now = time(NULL);
	struct tm tmp, tmp2;
	localtime_r(&now, &tmp);

	memset(&tmp2, 0, sizeof(tmp2));
	tmp2.tm_year = tmp.tm_year;
	tmp2.tm_mon = tmp.tm_mon;
	tmp2.tm_mday = tmp.tm_mday;
	return (long)mktime(&tmp2);
}

CTimer::CTimer()
{
	sec_diff_ = get_local_diff();
	day_begin_time_ = getDayBeginTimestamp();
}

long CTimer::getNano() const
{
    long _nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            get_time_now().time_since_epoch()
    ).count();
    return _nano + sec_diff_;
}

void CTimer::setTime(long now_nano)
{
    long _nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            get_time_now().time_since_epoch()
    ).count();
    sec_diff_ = now_nano - _nano;
}
