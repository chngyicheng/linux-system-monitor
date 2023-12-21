#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return getpid(); }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  return static_cast<float>(LinuxParser::ActiveJiffies(getpid())) / LinuxParser::Jiffies();
}

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(getpid()); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(getpid()); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(getpid()); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(getpid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return this->cpuUtilization_ < a.cpuUtilization_;
}