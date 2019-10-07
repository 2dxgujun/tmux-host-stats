#ifndef STATS_H_
#define STATS_H_

#include <sys/types.h>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
  #define CP_USER 0
  #define CP_SYS  1
  #define CP_IDLE 2
  #define CP_NICE 3
  #define CP_STATES 4
#else
  #define CP_USER 0
  #define CP_NICE 1
  #define CP_SYS  2
  #define CP_IDLE 3
  #define CP_STATES 4
#endif

struct MemoryStatus
{
  float used_mem; // megabytes
  float free_mem;
  float total_mem;
};

uint8_t get_cpu_count();

float cpu_percentage( unsigned );

void mem_status( MemoryStatus & status );

#endif
