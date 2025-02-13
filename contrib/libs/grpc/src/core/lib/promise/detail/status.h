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

#ifndef GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H
#define GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H

#include <grpc/support/port_platform.h>

#include <utility>

#include "y_absl/status/status.h"
#include "y_absl/status/statusor.h"

// Helpers for dealing with y_absl::Status/StatusOr generically

namespace grpc_core {
namespace promise_detail {

// Convert with a move the input status to an y_absl::Status.
template <typename T>
y_absl::Status IntoStatus(y_absl::StatusOr<T>* status) {
  return std::move(status->status());
}

// Convert with a move the input status to an y_absl::Status.
inline y_absl::Status IntoStatus(y_absl::Status* status) {
  return std::move(*status);
}

}  // namespace promise_detail

// Return true if the status represented by the argument is ok, false if not.
// By implementing this function for other, non-y_absl::Status types, those types
// can participate in TrySeq as result types that affect control flow.
inline bool IsStatusOk(const y_absl::Status& status) { return status.ok(); }

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H
