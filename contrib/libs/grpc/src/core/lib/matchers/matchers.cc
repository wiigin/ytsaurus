// Copyright 2021 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <grpc/support/port_platform.h>

#include "src/core/lib/matchers/matchers.h"

#include <utility>

#include "y_absl/memory/memory.h"
#include "y_absl/status/status.h"
#include "y_absl/strings/ascii.h"
#include "y_absl/strings/match.h"
#include "y_absl/strings/numbers.h"
#include "y_absl/strings/str_format.h"

namespace grpc_core {

//
// StringMatcher
//

y_absl::StatusOr<StringMatcher> StringMatcher::Create(Type type,
                                                    y_absl::string_view matcher,
                                                    bool case_sensitive) {
  if (type == Type::kSafeRegex) {
    auto regex_matcher = y_absl::make_unique<RE2>(TString(matcher));
    if (!regex_matcher->ok()) {
      return y_absl::InvalidArgumentError(
          "Invalid regex string specified in matcher.");
    }
    return StringMatcher(std::move(regex_matcher));
  } else {
    return StringMatcher(type, matcher, case_sensitive);
  }
}

StringMatcher::StringMatcher(Type type, y_absl::string_view matcher,
                             bool case_sensitive)
    : type_(type), string_matcher_(matcher), case_sensitive_(case_sensitive) {}

StringMatcher::StringMatcher(std::unique_ptr<RE2> regex_matcher)
    : type_(Type::kSafeRegex), regex_matcher_(std::move(regex_matcher)) {}

StringMatcher::StringMatcher(const StringMatcher& other)
    : type_(other.type_), case_sensitive_(other.case_sensitive_) {
  if (type_ == Type::kSafeRegex) {
    regex_matcher_ = y_absl::make_unique<RE2>(other.regex_matcher_->pattern());
  } else {
    string_matcher_ = other.string_matcher_;
  }
}

StringMatcher& StringMatcher::operator=(const StringMatcher& other) {
  type_ = other.type_;
  if (type_ == Type::kSafeRegex) {
    regex_matcher_ = y_absl::make_unique<RE2>(other.regex_matcher_->pattern());
  } else {
    string_matcher_ = other.string_matcher_;
  }
  case_sensitive_ = other.case_sensitive_;
  return *this;
}

StringMatcher::StringMatcher(StringMatcher&& other) noexcept
    : type_(other.type_), case_sensitive_(other.case_sensitive_) {
  if (type_ == Type::kSafeRegex) {
    regex_matcher_ = std::move(other.regex_matcher_);
  } else {
    string_matcher_ = std::move(other.string_matcher_);
  }
}

StringMatcher& StringMatcher::operator=(StringMatcher&& other) noexcept {
  type_ = other.type_;
  if (type_ == Type::kSafeRegex) {
    regex_matcher_ = std::move(other.regex_matcher_);
  } else {
    string_matcher_ = std::move(other.string_matcher_);
  }
  case_sensitive_ = other.case_sensitive_;
  return *this;
}

bool StringMatcher::operator==(const StringMatcher& other) const {
  if (type_ != other.type_ || case_sensitive_ != other.case_sensitive_) {
    return false;
  }
  if (type_ == Type::kSafeRegex) {
    return regex_matcher_->pattern() == other.regex_matcher_->pattern();
  } else {
    return string_matcher_ == other.string_matcher_;
  }
}

bool StringMatcher::Match(y_absl::string_view value) const {
  switch (type_) {
    case Type::kExact:
      return case_sensitive_ ? value == string_matcher_
                             : y_absl::EqualsIgnoreCase(value, string_matcher_);
    case StringMatcher::Type::kPrefix:
      return case_sensitive_
                 ? y_absl::StartsWith(value, string_matcher_)
                 : y_absl::StartsWithIgnoreCase(value, string_matcher_);
    case StringMatcher::Type::kSuffix:
      return case_sensitive_ ? y_absl::EndsWith(value, string_matcher_)
                             : y_absl::EndsWithIgnoreCase(value, string_matcher_);
    case StringMatcher::Type::kContains:
      return case_sensitive_
                 ? y_absl::StrContains(value, string_matcher_)
                 : y_absl::StrContains(y_absl::AsciiStrToLower(value),
                                     y_absl::AsciiStrToLower(string_matcher_));
    case StringMatcher::Type::kSafeRegex:
      return RE2::FullMatch(TString(value), *regex_matcher_);
    default:
      return false;
  }
}

TString StringMatcher::ToString() const {
  switch (type_) {
    case Type::kExact:
      return y_absl::StrFormat("StringMatcher{exact=%s%s}", string_matcher_,
                             case_sensitive_ ? "" : ", case_sensitive=false");
    case Type::kPrefix:
      return y_absl::StrFormat("StringMatcher{prefix=%s%s}", string_matcher_,
                             case_sensitive_ ? "" : ", case_sensitive=false");
    case Type::kSuffix:
      return y_absl::StrFormat("StringMatcher{suffix=%s%s}", string_matcher_,
                             case_sensitive_ ? "" : ", case_sensitive=false");
    case Type::kContains:
      return y_absl::StrFormat("StringMatcher{contains=%s%s}", string_matcher_,
                             case_sensitive_ ? "" : ", case_sensitive=false");
    case Type::kSafeRegex:
      return y_absl::StrFormat("StringMatcher{safe_regex=%s}",
                             regex_matcher_->pattern());
    default:
      return "";
  }
}

//
// HeaderMatcher
//

y_absl::StatusOr<HeaderMatcher> HeaderMatcher::Create(
    y_absl::string_view name, Type type, y_absl::string_view matcher,
    int64_t range_start, int64_t range_end, bool present_match,
    bool invert_match) {
  if (static_cast<int>(type) < 5) {
    // Only for EXACT, PREFIX, SUFFIX, SAFE_REGEX and CONTAINS.
    y_absl::StatusOr<StringMatcher> string_matcher =
        StringMatcher::Create(static_cast<StringMatcher::Type>(type), matcher,
                              /*case_sensitive=*/true);
    if (!string_matcher.ok()) {
      return string_matcher.status();
    }
    return HeaderMatcher(name, type, std::move(string_matcher.value()),
                         invert_match);
  } else if (type == Type::kRange) {
    if (range_start > range_end) {
      return y_absl::InvalidArgumentError(
          "Invalid range specifier specified: end cannot be smaller than "
          "start.");
    }
    return HeaderMatcher(name, range_start, range_end, invert_match);
  } else {
    return HeaderMatcher(name, present_match, invert_match);
  }
}

HeaderMatcher::HeaderMatcher(y_absl::string_view name, Type type,
                             StringMatcher string_matcher, bool invert_match)
    : name_(name),
      type_(type),
      matcher_(std::move(string_matcher)),
      invert_match_(invert_match) {}

HeaderMatcher::HeaderMatcher(y_absl::string_view name, int64_t range_start,
                             int64_t range_end, bool invert_match)
    : name_(name),
      type_(Type::kRange),
      range_start_(range_start),
      range_end_(range_end),
      invert_match_(invert_match) {}

HeaderMatcher::HeaderMatcher(y_absl::string_view name, bool present_match,
                             bool invert_match)
    : name_(name),
      type_(Type::kPresent),
      present_match_(present_match),
      invert_match_(invert_match) {}

HeaderMatcher::HeaderMatcher(const HeaderMatcher& other)
    : name_(other.name_),
      type_(other.type_),
      invert_match_(other.invert_match_) {
  switch (type_) {
    case Type::kRange:
      range_start_ = other.range_start_;
      range_end_ = other.range_end_;
      break;
    case Type::kPresent:
      present_match_ = other.present_match_;
      break;
    default:
      matcher_ = other.matcher_;
  }
}

HeaderMatcher& HeaderMatcher::operator=(const HeaderMatcher& other) {
  name_ = other.name_;
  type_ = other.type_;
  invert_match_ = other.invert_match_;
  switch (type_) {
    case Type::kRange:
      range_start_ = other.range_start_;
      range_end_ = other.range_end_;
      break;
    case Type::kPresent:
      present_match_ = other.present_match_;
      break;
    default:
      matcher_ = other.matcher_;
  }
  return *this;
}

HeaderMatcher::HeaderMatcher(HeaderMatcher&& other) noexcept
    : name_(std::move(other.name_)),
      type_(other.type_),
      invert_match_(other.invert_match_) {
  switch (type_) {
    case Type::kRange:
      range_start_ = other.range_start_;
      range_end_ = other.range_end_;
      break;
    case Type::kPresent:
      present_match_ = other.present_match_;
      break;
    default:
      matcher_ = std::move(other.matcher_);
  }
}

HeaderMatcher& HeaderMatcher::operator=(HeaderMatcher&& other) noexcept {
  name_ = std::move(other.name_);
  type_ = other.type_;
  invert_match_ = other.invert_match_;
  switch (type_) {
    case Type::kRange:
      range_start_ = other.range_start_;
      range_end_ = other.range_end_;
      break;
    case Type::kPresent:
      present_match_ = other.present_match_;
      break;
    default:
      matcher_ = std::move(other.matcher_);
  }
  return *this;
}

bool HeaderMatcher::operator==(const HeaderMatcher& other) const {
  if (name_ != other.name_) return false;
  if (type_ != other.type_) return false;
  if (invert_match_ != other.invert_match_) return false;
  switch (type_) {
    case Type::kRange:
      return range_start_ == other.range_start_ &&
             range_end_ == other.range_end_;
    case Type::kPresent:
      return present_match_ == other.present_match_;
    default:
      return matcher_ == other.matcher_;
  }
}

bool HeaderMatcher::Match(
    const y_absl::optional<y_absl::string_view>& value) const {
  bool match;
  if (type_ == Type::kPresent) {
    match = value.has_value() == present_match_;
  } else if (!value.has_value()) {
    // All other types fail to match if field is not present.
    return false;
  } else if (type_ == Type::kRange) {
    int64_t int_value;
    match = y_absl::SimpleAtoi(value.value(), &int_value) &&
            int_value >= range_start_ && int_value < range_end_;
  } else {
    match = matcher_.Match(value.value());
  }
  return match != invert_match_;
}

TString HeaderMatcher::ToString() const {
  switch (type_) {
    case Type::kRange:
      return y_absl::StrFormat("HeaderMatcher{%s %srange=[%d, %d]}", name_,
                             invert_match_ ? "not " : "", range_start_,
                             range_end_);
    case Type::kPresent:
      return y_absl::StrFormat("HeaderMatcher{%s %spresent=%s}", name_,
                             invert_match_ ? "not " : "",
                             present_match_ ? "true" : "false");
    case Type::kExact:
    case Type::kPrefix:
    case Type::kSuffix:
    case Type::kSafeRegex:
    case Type::kContains:
      return y_absl::StrFormat("HeaderMatcher{%s %s%s}", name_,
                             invert_match_ ? "not " : "", matcher_.ToString());
    default:
      return "";
  }
}

}  // namespace grpc_core
