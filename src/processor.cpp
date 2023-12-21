#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  std::string uptime, idletime;
  std::string line, cpu;
  int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  int total_time, total_time_diff, idle_time, idle_time_diff;
  std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    total_time = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
    idle_time = idle + iowait;

    static int prev_total_time = 0;
    static int prev_idle_time = 0;
    total_time_diff = total_time - prev_total_time;
    idle_time_diff = idle_time - prev_idle_time;

    double cpu_utilization = (total_time_diff - idle_time_diff) / static_cast<double>(total_time_diff) * 100;

    prev_total_time = total_time;
    prev_idle_time = idle_time;
  }
  return 0;
}