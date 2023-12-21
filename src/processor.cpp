#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  // Get CPU utilization values
  std::vector<std::string> cpuValues = LinuxParser::CpuUtilization();

  // Ensure there are enough values to calculate utilization
  float totalJiffies = LinuxParser::Jiffies();

  float activeJiffies = LinuxParser::ActiveJiffies();

  // Calculate CPU utilization as a percentage
  if (totalJiffies > 0) {    // Calculate the total active and total jiffies
    return (activeJiffies) / totalJiffies;
  }
  // Return 0 if there was an issue getting CPU utilization
  return 0.0;
}