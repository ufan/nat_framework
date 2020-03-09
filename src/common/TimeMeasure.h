/*
 * TimeMeasure.h
 *
 *  Created on: 2017年10月30日
 *      Author: hongxu
 */

#ifndef SRC_COMMON_TIMEMEASURE_H_
#define SRC_COMMON_TIMEMEASURE_H_

#define getCycleBegin(cycles_high, cycles_low)\
	asm volatile ("CPUID\n\t"\
	"RDTSC\n\t"\
	"mov %%edx, %0\n\t"\
	"mov %%eax, %1\n\t": "=r" ((cycles_high)), "=r" ((cycles_low))::\
	"%rax", "%rbx", "%rcx", "%rdx")


#define getCycleEnd(cycles_high, cycles_low)\
	asm volatile("RDTSCP\n\t"\
	"mov %%edx, %0\n\t"\
	"mov %%eax, %1\n\t"\
	"CPUID\n\t": "=r" ((cycles_high)), "=r" ((cycles_low)):: "%rax",\
	"%rbx", "%rcx", "%rdx")


#define getu64(h,l) ((((uint64_t)(h)) << 32) | (l))


#endif /* SRC_COMMON_TIMEMEASURE_H_ */
