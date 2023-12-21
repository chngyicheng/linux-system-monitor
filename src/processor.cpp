#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  // Get CPU utilization values
  std::vector<std::string> cpuValues = LinuxParser::CpuUtilization();

  // Ensure there are enough values to calculate utilization
  if (cpuValues.size() >= LinuxParser::kSteal_ + 1) {
    // Calculate the total active and total jiffies
    float activeJiffies = std::stof(cpuValues[LinuxParser::kUser_]) +
                          std::stof(cpuValues[LinuxParser::kNice_]) +
                          std::stof(cpuValues[LinuxParser::kSystem_]) +
                          std::stof(cpuValues[LinuxParser::kIRQ_]) +
                          std::stof(cpuValues[LinuxParser::kSoftIRQ_]) +
                          std::stof(cpuValues[LinuxParser::kSteal_]) +
                          std::stof(cpuValues[LinuxParser::kGuest_]) +
                          std::stof(cpuValues[LinuxParser::kGuestNice_]);

    float totalJiffies = activeJiffies +
                          std::stof(cpuValues[LinuxParser::kIdle_]) +
                          std::stof(cpuValues[LinuxParser::kIOwait_]);

    // Calculate CPU utilization as a percentage
    return activeJiffies / totalJiffies;
  }

  // Return 0 if there was an issue getting CPU utilization
  return 0.0;
}