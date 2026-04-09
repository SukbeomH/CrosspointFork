#pragma once

#include <FsHelpers.h>
#include <HalStorage.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// Utilities for managing sleep screen image directories on SD card.
// Scans for /sleep or /sleep_* directories containing BMP files.
namespace SleepImageUtils {

inline std::string toLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

inline bool isSleepDirectoryName(const std::string& name) {
  std::string base = name;
  while (!base.empty() && (base.front() == '/' || base.front() == '.')) base.erase(base.begin());
  const std::string lower = toLower(base);
  return lower == "sleep" || lower.rfind("sleep_", 0) == 0;
}

inline std::vector<std::string> listSleepDirectories() {
  std::vector<std::string> dirs;
  auto root = Storage.open("/");
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    return dirs;
  }
  root.rewindDirectory();
  char name[256];
  for (auto entry = root.openNextFile(); entry; entry = root.openNextFile()) {
    if (!entry.isDirectory()) {
      entry.close();
      continue;
    }
    entry.getName(name, sizeof(name));
    if (isSleepDirectoryName(name)) {
      dirs.push_back(std::string("/") + name);
    }
    entry.close();
  }
  root.close();
  std::sort(dirs.begin(), dirs.end());
  return dirs;
}

inline std::vector<std::string> listBmpFiles(const std::string& dirPath) {
  std::vector<std::string> files;
  auto dir = Storage.open(dirPath.c_str());
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return files;
  }
  dir.rewindDirectory();
  char name[256];
  for (auto entry = dir.openNextFile(); entry; entry = dir.openNextFile()) {
    if (entry.isDirectory()) {
      entry.close();
      continue;
    }
    entry.getName(name, sizeof(name));
    if (FsHelpers::hasBmpExtension(name)) {
      files.push_back(dirPath + "/" + name);
    }
    entry.close();
  }
  dir.close();
  std::sort(files.begin(), files.end());
  return files;
}

inline std::string getDirectoryLabel(const std::string& path) {
  const size_t sep = path.find_last_of('/');
  if (sep == std::string::npos || sep + 1 >= path.size()) return path;
  return path.substr(sep + 1);
}

}  // namespace SleepImageUtils
