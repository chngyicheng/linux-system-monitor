#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T LinuxParser::findValueByKey(string const &keyFilter, string const &fileName) {
  string key, line;
  T value;

  std::ifstream stream(kProcDirectory + fileName);
  while (!stream.eof()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream >> key >> value) {
      if (key == keyFilter) {
        return value;
      }
    }
  }
  return value;
};

template <typename T>
T LinuxParser::getValueOfFile(std::string const &filename) {
  string line;
  T value;

  std::ifstream stream(kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  return value;
};

// Read OS data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read Kernel data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string mem, mem_value;
  float mem_total_value, mem_free_value;

  mem_total_value = findValueByKey<float>(filterMemTotalString, kMeminfoFilename);
  mem_free_value  = findValueByKey<float>(filterMemFreeString, kMeminfoFilename);

  return (mem_total_value - mem_free_value) / mem_total_value;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
    return std::stol(uptime);
  }
  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  string cpu, proc_name, processes;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    if (cpu == "cpu") {
      std::vector<std::string> values(std::istream_iterator<std::string>{linestream}, std::istream_iterator<std::string>());
      // Ensure there are enough values to extract
      if (values.size() >= kIOwait_ + 1) {
        // Calculate the total jiffies
        long user       = std::stol(values[kUser_]);
        long nice       = std::stol(values[kNice_]);
        long system     = std::stol(values[kSystem_]);
        long irq        = std::stol(values[kIRQ_]);
        long softirq    = std::stol(values[kSoftIRQ_]);
        long steal      = std::stol(values[kSteal_]);
        long guest      = std::stol(values[kGuest_]);
        long guest_nice = std::stol(values[kGuestNice_]);
        long idle       = std::stol(values[kIdle_]);
        long iowait     = std::stol(values[kIOwait_]);

        return user + nice + system + irq + softirq + steal + guest + guest_nice + idle + iowait;
      }
    }
  }
  return 0;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string pid_, comm_, state_, ppid_, pgrp_, session_, tty_nr_, tpgid_, flags_, minflt_, cminflt_, majflt_, cmajflt_, utime_, stime_, cutime_, cstime_, discard;
  string line, pid_string = "/" + std::to_string(pid);
  std::ifstream stream(kProcDirectory + pid_string + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    // Put the information from linestream into strings
    linestream >> pid_ >> comm_ >> state_ >> ppid_ >> pgrp_ >> session_ >> tty_nr_ >> tpgid_ >> flags_ >> minflt_ >> cminflt_ >> majflt_ >> cmajflt_ >> utime_ >> stime_ >> cutime_ >> cstime_;
    long utime  = std::stol(utime_);
    long stime  = std::stol(stime_);
    long cutime = std::stol(cutime_);
    long cstime = std::stol(cstime_);
    
    // Skip the remaining fields
    for (int i = 0; i < 7; ++i) {
      linestream >> discard;
    }

    // Total active jiffies
    long total_jiffies = utime + stime + cutime + cstime;
    long process_uptime = LinuxParser::UpTime(pid);

    // Calculate the active jiffies during the process uptime
    long active_jiffies = (process_uptime * sysconf(_SC_CLK_TCK)) - total_jiffies;

    return active_jiffies;
  }
  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  string cpu, proc_name, processes;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    if (cpu == "cpu") {
      std::vector<std::string> values(std::istream_iterator<std::string>{linestream}, std::istream_iterator<std::string>());

      // Calculate the active jiffies
      long user       = std::stol(values[kUser_]);
      long nice       = std::stol(values[kNice_]);
      long system     = std::stol(values[kSystem_]);
      long irq        = std::stol(values[kIRQ_]);
      long softirq    = std::stol(values[kSoftIRQ_]);
      long steal      = std::stol(values[kSteal_]);
      long guest      = std::stol(values[kGuest_]);
      long guest_nice = std::stol(values[kGuestNice_]);

      long active_jiffies = user + nice + system + irq + softirq + steal + guest + guest_nice;
      return active_jiffies;
    }
  }
  return 0;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string cpu, proc_name, processes;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    if (cpu == "cpu:") {
      std::vector<std::string> values(std::istream_iterator<std::string>{linestream}, std::istream_iterator<std::string>());

      // Calculate the active jiffies
      long idle   = std::stol(values[kIdle_]);
      long iowait = std::stol(values[kIOwait_]);

      long idle_jiffies = idle + iowait;
      return idle_jiffies;
    }
  }
  return 0;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_utilization;

  // Open the /proc/stat file
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string line;
    while (std::getline(stream, line)) {
      // Check if the line starts with "cpu "
      if (line.substr(0, 3) == "cpu ") {
        std::istringstream linestream(line);
        std::istream_iterator<string> beg(linestream), end;
        vector<string> values(beg, end);

        // Add the CPU utilization values to the result vector
        for (size_t i = 1; i < values.size(); ++i) {
          cpu_utilization.emplace_back(values[i]);
        }

        break;  // Found the CPU utilization line, exit the loop
      }
    }
  }

  return cpu_utilization;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string proc_name;
  int processes;

  processes = findValueByKey<int>(filterProcesses, kStatFilename);
  return processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string proc_name; //, processes;
  int processes;

  processes = findValueByKey<int>(filterRunningProcesses, kStatFilename);
  return processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  return std::string(getValueOfFile<std::string>("/" + std::to_string(pid) + kCmdlineFilename));
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string ram_name, ram_value;
  float ram_mb; 

  ram_mb = findValueByKey<float>(filterVmRSS, "/" + std::to_string(pid) + kStatusFilename) / 1000;
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << ram_mb;
  return stream.str();
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string uid_name, uid_value;

  uid_value = findValueByKey<string>(filterUid, "/" + std::to_string(pid) + kStatusFilename);
  return uid_value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string username, passwd, uidStr;
  string line;
  std::ifstream stream(kPasswordPath);
  while (!stream.eof()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    // Split the line by the colon delimiter
    std::getline(linestream, username, ':'); // Get the username field
    std::getline(linestream, passwd, ':'); // Get the password field
    std::getline(linestream, uidStr, ':'); // Get the UID field

    if (uidStr == LinuxParser::Uid(pid)) {
        return username;
    }
  }
  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string pid_, comm, state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt, utime, stime, cutime, cstime, priority, nice, num_threads, itrealvalue, starttime;
  string line, pid_string = "/" + std::to_string(pid);;
  std::ifstream stream(kProcDirectory + pid_string + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> pid_ >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue >> starttime;
    string kernel_version = Kernel();
    if (std::stof(kernel_version.substr(0,3))  >= 2.6) {
      return UpTime() - stol(starttime)/sysconf(_SC_CLK_TCK);
    } else {
      return std::stol(starttime);
    }
  }
  return 0;
}
