/**
 * This code was collaboratively generated with Claude AI (Anthropic)
 * through an iterative process of requirements gathering,
 * code generation, and refinement.
 */

#include <intrin.h>
#include <windows.h>

#include <bitset>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

struct CacheDetails {
  size_t size;
  int lineSize;
  int sharingCores;
  std::vector<ULONG_PTR> processorMasks;
};

void printProcessorInfo() {
  int physicalCores, logicalCores;

  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);

  DWORD length = 0;
  GetLogicalProcessorInformation(nullptr, &length);
  std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
      length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
  GetLogicalProcessorInformation(&buffer[0], &length);

  physicalCores = 0;
  logicalCores = sysInfo.dwNumberOfProcessors;

  for (const auto& info : buffer) {
    if (info.Relationship == RelationProcessorCore) {
      physicalCores++;
    }
  }

  std::cout << "Processor Information:\n" << std::string(50, '=') << "\n";
  std::cout << "Physical Cores: " << physicalCores << "\n";
  std::cout << "Logical Processors: " << logicalCores << "\n";
  std::cout << "Hyper-Threading: "
            << (logicalCores > physicalCores ? "Enabled" : "Disabled") << "\n";
  std::cout << "Maximum Parallel Threads: " << logicalCores << "\n\n";
}

int countSetBits(ULONG_PTR mask) {
  int count = 0;
  while (mask) {
    count += mask & 1;
    mask >>= 1;
  }
  return count;
}

std::string getCacheTypeString(PROCESSOR_CACHE_TYPE type) {
  switch (type) {
    case CacheUnified:
      return "Unified Cache";
    case CacheInstruction:
      return "Instruction Cache";
    case CacheData:
      return "Data Cache";
    case CacheTrace:
      return "Trace Cache";
    default:
      return "Unknown Cache";
  }
}

void printCacheInfo() {
  std::cout << "CPU Cache Information:\n" << std::string(50, '=') << "\n\n";

  char vendor[13];
  int vendorInfo[4];
  __cpuid(vendorInfo, 0);
  memcpy(vendor, &vendorInfo[1], 4);
  memcpy(vendor + 4, &vendorInfo[3], 4);
  memcpy(vendor + 8, &vendorInfo[2], 4);
  vendor[12] = '\0';
  std::cout << "CPU Vendor: " << vendor << "\n\n";

  DWORD length = 0;
  GetLogicalProcessorInformation(nullptr, &length);
  std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
      length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
  GetLogicalProcessorInformation(&buffer[0], &length);

  std::map<int, std::map<PROCESSOR_CACHE_TYPE, CacheDetails>> cacheMap;

  for (const auto& info : buffer) {
    if (info.Relationship == RelationCache && info.Cache.Level > 0) {
      const auto& cache = info.Cache;
      auto& cacheDetails = cacheMap[cache.Level][cache.Type];

      if (cacheDetails.processorMasks.empty()) {
        cacheDetails.size = cache.Size;
        cacheDetails.lineSize = cache.LineSize;
        cacheDetails.sharingCores = countSetBits(info.ProcessorMask);
      }
      cacheDetails.processorMasks.push_back(info.ProcessorMask);
    }
  }

  for (const auto& levelPair : cacheMap) {
    std::cout << "L" << levelPair.first << " Cache Information:\n";
    std::cout << std::string(30, '-') << "\n";

    for (const auto& typePair : levelPair.second) {
      const auto& cacheDetails = typePair.second;
      std::cout << "Type: " << getCacheTypeString(typePair.first) << "\n";
      std::cout << "Size: " << cacheDetails.size / 1024 << " KB\n";
      std::cout << "Line Size: " << cacheDetails.lineSize << " bytes\n";
      std::cout << "Instances: " << cacheDetails.processorMasks.size() << "\n";
      std::cout << "Each instance shared by " << cacheDetails.sharingCores
                << " logical processor(s)\n";
      std::cout << std::string(30, '-') << "\n";
    }
    std::cout << "\n";
  }
}

int main() {
  try {
    printProcessorInfo();
    printCacheInfo();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
