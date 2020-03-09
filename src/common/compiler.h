/*
 * Compiler.h
 *
 *  Created on: Sep 6, 2017
 *      Author: hongxu
 */

#ifndef LIB_COMMON_COMPILER_H_
#define LIB_COMMON_COMPILER_H_

#define likely(x)  __builtin_expect(!!(x), 1)

#define unlikely(x)  __builtin_expect(!!(x), 0)



#endif /* LIB_COMMON_COMPILER_H_ */
