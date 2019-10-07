#include <unistd.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sstream>

#include "stats.h"
#include "conversions.h"

host_cpu_load_info_data_t _get_cpu_percentage()
{
  kern_return_t              error;
  mach_msg_type_number_t     count;
  host_cpu_load_info_data_t  r_load;
  mach_port_t                mach_port;

  count = HOST_CPU_LOAD_INFO_COUNT;
  mach_port = mach_host_self();
  error = host_statistics(mach_port, HOST_CPU_LOAD_INFO, 
      ( host_info_t )&r_load, &count );

  if ( error != KERN_SUCCESS )
  {
    return host_cpu_load_info_data_t();
  }

  return r_load;
}

float cpu_percentage( unsigned int cpu_usage_delay )
{
  // Get the load times from the XNU kernel
  host_cpu_load_info_data_t load1 = _get_cpu_percentage();
  usleep( cpu_usage_delay );
  host_cpu_load_info_data_t load2 = _get_cpu_percentage();

  // Current load times
  unsigned long long current_user = load1.cpu_ticks[CP_USER];
  unsigned long long current_system = load1.cpu_ticks[CP_SYS];
  unsigned long long current_nice = load1.cpu_ticks[CP_NICE];
  unsigned long long current_idle = load1.cpu_ticks[CP_IDLE];
  // Next load times
  unsigned long long next_user = load2.cpu_ticks[CP_USER];
  unsigned long long next_system = load2.cpu_ticks[CP_SYS];
  unsigned long long next_nice = load2.cpu_ticks[CP_NICE];
  unsigned long long next_idle = load2.cpu_ticks[CP_IDLE];
  // Difference between the two
  unsigned long long diff_user = next_user - current_user;
  unsigned long long diff_system = next_system - current_system;
  unsigned long long diff_nice = next_nice - current_nice;
  unsigned long long diff_idle = next_idle - current_idle;

  return static_cast<float>( diff_user + diff_system + diff_nice ) / 
    static_cast<float>( diff_user + diff_system + diff_nice + diff_idle ) * 
    100.0;
}

void mem_status( MemoryStatus & status )
{
  // These values are in bytes
  u_int64_t total_mem;
  u_int64_t used_mem;
  u_int64_t free_mem;

  vm_size_t page_size;
  vm_statistics_data_t vm_stats;

  // Get total physical memory
  int mib[] = { CTL_HW, HW_MEMSIZE };
  size_t length = sizeof( total_mem );
  sysctl( mib, 2, &total_mem, &length, NULL, 0 );

  mach_port_t mach_port = mach_host_self();
  mach_msg_type_number_t count = sizeof( vm_stats ) / sizeof( natural_t );
  if( KERN_SUCCESS == host_page_size( mach_port, &page_size ) &&
      KERN_SUCCESS == host_statistics64( mach_port, HOST_VM_INFO,
        ( host_info64_t )&vm_stats, &count )
    )
  {
    free_mem = (int64_t)vm_stats.free_count * (int64_t)page_size;
    used_mem = ((int64_t)vm_stats.active_count +
               (int64_t)vm_stats.inactive_count +
               (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
  }

  status.free_mem = convert_unit(static_cast< float >( free_mem ), MEGABYTES );
  status.used_mem = convert_unit(static_cast< float >( used_mem ), MEGABYTES );
  status.total_mem = convert_unit(static_cast< float >( total_mem ), MEGABYTES );
}
