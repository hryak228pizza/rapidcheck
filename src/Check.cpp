#include "rapidcheck/Check.h"

#include "detail/DefaultTestListener.h"
#include "detail/Testing.h"

namespace rc {
namespace detail {

TestResult
checkProperty(const Property &property,
              const TestMetadata &metadata,
              const TestParams &params,
              TestListener &listener,
              const std::unordered_map<std::string, Reproduce> &reproduceMap,
              bool VerboseMode) {
  if (reproduceMap.empty()) {
    return testProperty(property, metadata, params, listener, VerboseMode);
  }

  const auto it = reproduceMap.find(metadata.id);
  if (metadata.id.empty() || (it == end(reproduceMap))) {
    SuccessResult success;
    success.numSuccess = 0;
    return success;
  } else {
    auto reproduce = it->second;
    if (params.disableShrinking) {
      reproduce.shrinkPath.clear();
    }
    return reproduceProperty(property, reproduce);
  }
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         TestListener &listener,
                         bool VerboseMode) {
  return checkProperty(property,
                       metadata,
                       params,
                       listener,
                       configuration().reproduce,
                       VerboseMode);
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         bool VerboseMode) {
  return checkProperty(
      property, metadata, params, globalTestListener(), VerboseMode);
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         bool VerboseMode) {
  return checkProperty(
      property, metadata, configuration().testParams, VerboseMode);
}

TestResult checkProperty(const Property &property, bool VerboseMode) {
  return checkProperty(property, TestMetadata(), VerboseMode);
}

} // namespace detail
} // namespace rc
