/* vim: tabstop=2 shiftwidth=2 expandtab textwidth=80 linebreak wrap
 *
 * Copyright 2012 Matthew McCormick
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <getopt.h>

#include "version.h"
#include "stats.h"
#include "conversions.h"

uint8_t get_cpu_count()
{
  return sysconf( _SC_NPROCESSORS_ONLN );
}

std::string cpu_string( unsigned int cpu_usage_delay )
{

  float percentage;
  float multiplier = 1.0f;

  //output stuff
  std::ostringstream oss;
  oss.precision( 1 );
  oss.setf( std::ios::fixed | std::ios::right );

  // get %
  percentage = cpu_percentage( cpu_usage_delay );

  // if percentage*multiplier >= 100, remove decimal point to keep number short
  if ( percentage*multiplier >= 100.0f )
  {
    oss.precision( 0 );
  }

  unsigned int percent = static_cast<unsigned int>( percentage );

  oss.width( 5 );
  oss << percentage * multiplier;

  return oss.str();
}

/** Memory status string output mode.
 *
 * Examples:
 *
 * MEMORY_MODE_DEFAULT:          11156/16003MB
 * MEMORY_MODE_FREE_MEMORY:
 * MEMORY_MODE_USAGE_PERCENTAGE: 
 */
enum MEMORY_MODE
{
  MEMORY_MODE_DEFAULT,
  MEMORY_MODE_FREE_MEMORY,
  MEMORY_MODE_USAGE_PERCENTAGE
};

std::string mem_string( const MemoryStatus & mem_status, MEMORY_MODE mode = MEMORY_MODE_DEFAULT )
{
  std::ostringstream oss;
  // Change the percision for floats, for a pretty output
  oss.precision( 2 );
  oss.setf( std::ios::fixed | std::ios::right );

  switch( mode )
  {
  case MEMORY_MODE_FREE_MEMORY: // Show free memory in MB or GB
    {
    const float free_mem = mem_status.total_mem - mem_status.used_mem;
    const float free_mem_in_gigabytes = convert_unit( free_mem, GIGABYTES, MEGABYTES );

    // if free memory is less than 1 GB, use MB instead
    if(  free_mem_in_gigabytes < 1.0f )
    {
      oss << free_mem << "MB";
    }
    else
    {
      oss << free_mem_in_gigabytes << "GB";
    }
    break;
    }
  case MEMORY_MODE_USAGE_PERCENTAGE:
    {
    // Calculate the percentage of used memory
    const float percentage_mem = mem_status.used_mem /
      static_cast<float>( mem_status.total_mem ) * 100.0;

    oss << percentage_mem << '%';
    break;
    }
  default: // Default mode, just show the used/total memory in MB
    oss << static_cast< unsigned int >( mem_status.used_mem ) << '/'
      << static_cast< unsigned int >( mem_status.total_mem ) << "MB";
  }

  return oss.str();
}

std::string load_string( short num_averages = 3 )
{
  std::ostringstream ss;
  double averages[num_averages];
  // based on: opensource.apple.com/source/Libc/Libc-262/gen/getloadavg.c

  if( num_averages <= 0 || num_averages > 3)
  {
    ss << (char) 0;
    return ss.str();
  }

  if( getloadavg( averages, num_averages ) < 0 )
  {
    ss << " 0.00 0.00 0.00"; // couldn't get averages.
  }
  else
  {
    unsigned load_percent = static_cast<unsigned int>( averages[0] /
        get_cpu_count() * 0.5f * 100.0f );

    if( load_percent > 100 )
    {
      load_percent = 100;
    }

    ss << ' ';
    for( int i = 0; i < num_averages; ++i )
    {
      // Round to nearest, make sure this is only a 0.00 value not a 0.0000
      float avg = floorf( static_cast<float>( averages[i] ) * 100 + 0.5 ) / 100;
      // Don't print trailing whitespace for last element
      if ( i == num_averages-1 )
      {
        ss << avg;
      }
      else
      {
        ss << avg << " ";
      }
    }
  }

  return ss.str();
}

void print_help()
{
  using std::cout;
  using std::endl;

  cout << "tmux-host-stats v" << tmux_host_stats_VERSION << endl
    << "Usage: tmux-host-stats [OPTIONS]\n\n"
    << "Available options:\n"
    << "-h, --help\n"
    << "\t Prints this help message\n"
    << "-i <value>, --interval <value>\n"
    << "\tSet tmux status refresh interval in seconds. Default: 1 second\n"
    << "-m <value>, --mem-mode <value>\n"
    << "\tSet memory display mode. 0: Default, 1: Free memory, 2: Usage percent.\n"
    << "-t <value>, --cpu-mode <value>\n"
    << "\tSet cpu % display mode. 0: Default max 100%, 1: Max 100% * number of threads. \n"
    << "-a <value>, --averages-count <value>\n"
    << "\tSet how many load-averages should be drawn. Default: 3\n"
    << endl;
}

int main( int argc, char** argv )
{
  unsigned cpu_usage_delay = 990000;
  short averages_count = 3;
  MEMORY_MODE mem_mode = MEMORY_MODE_FREE_MEMORY;

  static struct option long_options[] =
  {
    // Struct is a s follows:
    //   const char * name, int has_arg, int *flag, int val
    // if *flag is null, val is option identifier to use in switch()
    // otherwise it's a value to set the variable *flag points to
    { "help", no_argument, NULL, 'h' },
    { "interval", required_argument, NULL, 'i' },
    { "mem-mode", required_argument, NULL, 'm' },
    { "cpu-mode", required_argument, NULL, 't' },
    { "averages-count", required_argument, NULL, 'a' },
    { 0, 0, 0, 0 } // used to handle unknown long options
  };

  int c;
  // while c != -1
  while( (c = getopt_long( argc, argv, "hi:cpqg:m:a:t:", long_options, NULL) ) != -1 )
  {
    switch( c )
    {
      case 'h': // --help, -h
        print_help();
        return EXIT_FAILURE;
        break;
      case 'i': // --interval, -i
        if( atoi( optarg ) < 1 )
          {
            std::cerr << "Status interval argument must be one or greater.\n";
            return EXIT_FAILURE;
          }
        cpu_usage_delay = atoi( optarg ) * 1000000 - 10000;
        break;
      case 'm': // --mem-mode, -m
        if( atoi( optarg ) < 0 )
          {
            std::cerr << "Memory mode argument must be zero or greater.\n";
            return EXIT_FAILURE;
          }
        mem_mode = static_cast< MEMORY_MODE >( atoi( optarg ) );
        break;
      case 'a': // --averages-count, -a
        if( atoi( optarg ) < 0 || atoi( optarg ) > 3 )
          {
            std::cerr << "Valid averages-count arguments are: 0, 1, 2, 3\n";
            return EXIT_FAILURE;
          }
        averages_count = atoi( optarg );
        break;
      case '?':
        // getopt_long prints error message automatically
        return EXIT_FAILURE;
        break;
      default:
        std::cerr << "?? getopt returned character code 0 " << c << std::endl;
        return EXIT_FAILURE;
    }
  }

  MemoryStatus memory_status;
  mem_status( memory_status );
  std::cout << mem_string( memory_status, mem_mode )
    << cpu_string( cpu_usage_delay )
    << load_string( averages_count ) << "\n";

  return EXIT_SUCCESS;
}

