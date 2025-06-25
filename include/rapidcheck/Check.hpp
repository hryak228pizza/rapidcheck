#pragma once

#include <iostream>

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Property.h"
#include "rapidcheck/detail/TestListener.h"

namespace rc {
namespace detail {

TestResult
checkProperty(const Property &property,
              const TestMetadata &metadata,
              const TestParams &params,
              TestListener &listener,
              const std::unordered_map<std::string, Reproduce> &reproduceMap,
              bool VerboseMode);

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         TestListener &listener,
                         bool VerboseMode);

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         bool VerboseMode);

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         bool VerboseMode);

// Uses defaults from configuration
TestResult checkProperty(const Property &property, bool VerboseMode);


template <typename Testable, typename... Args>
TestResult checkTestable(Testable &&testable, Args &&... args) {
  return checkProperty(toProperty(std::forward<Testable>(testable)),
                       std::forward<Args>(args)...);
}

} // namespace detail

template <typename Testable>
bool check(Testable &&testable) {
  return check(
      std::string(), std::forward<Testable>(testable), /*Verbose Mode*/ false);
}

template <typename Testable>
bool check(const std::string &description, Testable &&testable) {
  return check(description, std::forward<Testable>(testable), /*Verbose Mode*/ false);
}

// Verbose 
template <typename Testable>
bool check(Testable &&testable, bool VerboseMode) {
  return check(std::string(), std::forward<Testable>(testable), /*Verbose Mode*/ VerboseMode);
}

template <typename Testable>
bool check(const std::string &description, Testable &&testable, bool VerboseMode) {
  using namespace rc::detail;

  // Force loading of the configuration so that message comes _before_ the
  // description
  configuration();

  if (!description.empty()) {
    std::cerr << std::endl << "- " << description << std::endl;
  }

  TestMetadata metadata;
  metadata.id = description;
  metadata.description = description;
  const auto result =
      detail::checkTestable(std::forward<Testable>(testable), metadata, VerboseMode);

  printResultMessage(result, std::cerr);
  std::cerr << std::endl;

  return result.template is<detail::SuccessResult>();
}


} // namespace rc
