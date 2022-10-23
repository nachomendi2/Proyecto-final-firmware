#ifndef __BENCHMARK_H
#define __BENCHMARK_H

#include <USS_Lib_HAL.h>
#include <msp430.h>
#include <stdint.h>

#if defined(ENABLE_USS_LIB_BENCHMARK)

/* benchmark APIs */
extern void BENCHMARK_ENTRY_USS_LIB(void);
extern void BENCHMARK_EXIT_USS_LIB(void);

#else
#define BENCHMARK_ENTRY_USS_LIB()
#define BENCHMARK_EXIT_USS_LIB()
#endif
     
#endif /* __BENCHMARK_H */

