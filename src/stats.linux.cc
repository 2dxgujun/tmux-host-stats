#include <unistd.h>
#include <sstream>
#include <fstream>
#include <sys/sysinfo.h>

#include "stats.h"
#include "conversions.h"

float cpu_percentage( unsigned cpu_usage_delay )
{
  std::string line;
  size_t substr_start = 0;
  size_t substr_len;

  // cpu stats
  // user, nice, system, idle
  // in that order
  unsigned long long stats[CP_STATES];

  std::ifstream stat_file( "/proc/stat" );
  getline( stat_file, line );
  stat_file.close();

  // skip "cpu"
  substr_len = line.find_first_of( " ", 3 );
  // parse cpu line
  for( unsigned i=0; i < 4; i++ )
  {
    substr_start = line.find_first_not_of( " ", substr_len );
    substr_len   = line.find_first_of( " ", substr_start );
    stats[i] = std::stoll( line.substr( substr_start, substr_len ) );
  }

  usleep( cpu_usage_delay );

  stat_file.open( "/proc/stat" );
  getline( stat_file, line );
  stat_file.close();

  // skip "cpu"
  substr_len = line.find_first_of( " ", 3 );
  // parse cpu line
  for( unsigned i=0; i < 4; i++ )
  {
    substr_start = line.find_first_not_of( " ", substr_len );
    substr_len   = line.find_first_of    ( " ", substr_start );
    stats[i] = std::stoll( line.substr( substr_start, substr_len ) ) - stats[i];
  }

  return static_cast<float>( 
    stats[CP_USER] + stats[CP_NICE] + stats[CP_SYS]
    ) / static_cast<float>( 
        stats[CP_USER] + stats[CP_NICE] + stats[CP_SYS] + stats[CP_IDLE] 
    ) * 100.0;
}

void mem_status( MemoryStatus & status )
{
  std::string line;
  std::string substr;
  size_t substr_start;
  size_t substr_len;

  unsigned int total_mem;
  unsigned int used_mem;

  /* Since linux uses some RAM for disk caching, the actuall used ram is lower
   * than what sysinfo(), top or free reports. htop reports the usage in a
   * correct way. The memory used for caching doesn't count as used, since it
   * can be freed in any moment. Usually it hapens automatically, when an
   * application requests memory.
   * In order to calculate the ram that's actually used we need to use the
   * following formula:
   *    total_ram - free_ram - buffered_ram - cached_ram
   *
   * example data, junk removed, with comments added:
   *
   * MemTotal:        61768 kB    old
   * MemFree:          1436 kB    old
   * MemAvailable     ????? kB    ??
   * MemShared:           0 kB    old (now always zero; not calculated)
   * Buffers:          1312 kB    old
   * Cached:          20932 kB    old
   * SwapTotal:      122580 kB    old
   * SwapFree:        60352 kB    old
   */

  std::ifstream memory_info("/proc/meminfo");

  while( std::getline( memory_info, line ) )
  {
    substr_start = 0;
    substr_len = line.find_first_of( ':' );
    substr = line.substr( substr_start, substr_len );
    substr_start = line.find_first_not_of( " ", substr_len + 1 );
    substr_len = line.find_first_of( 'k' ) - substr_start;
    if( substr.compare( "MemTotal" ) == 0 )
    {
      // get total memory
      total_mem = stoi( line.substr( substr_start, substr_len ) );
    }
    else if( substr.compare( "MemFree" ) == 0 )
    {
      used_mem = total_mem - stoi( line.substr( substr_start, substr_len ) );
    }
    else if( substr.compare( "Buffers" ) == 0 ||
             substr.compare( "Cached" ) == 0 )
    {
      used_mem -= stoi( line.substr( substr_start, substr_len ) );
    }
  }

  // we want megabytes on output, but since the values already come as
  // kilobytes we need to divide them by 1024 only once, thus we use
  // KILOBYTES
  status.used_mem = convert_unit(static_cast< float >( used_mem ), MEGABYTES, KILOBYTES);
  status.total_mem = convert_unit(static_cast< float >( total_mem ), MEGABYTES, KILOBYTES);
}