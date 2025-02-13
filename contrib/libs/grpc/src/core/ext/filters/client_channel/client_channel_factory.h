//
// Copyright 2015 gRPC authors.
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
//

#ifndef GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_CLIENT_CHANNEL_FACTORY_H
#define GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_CLIENT_CHANNEL_FACTORY_H

#include <grpc/support/port_platform.h>

#include "y_absl/strings/string_view.h"

#include "src/core/ext/filters/client_channel/subchannel.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/resolved_address.h"

namespace grpc_core {

class ClientChannelFactory {
 public:
  struct RawPointerChannelArgTag {};

  virtual ~ClientChannelFactory() = default;

  // Creates a subchannel with the specified args.
  virtual RefCountedPtr<Subchannel> CreateSubchannel(
      const grpc_resolved_address& address, const ChannelArgs& args) = 0;

  static y_absl::string_view ChannelArgName();
};

}  // namespace grpc_core

#endif  // GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_CLIENT_CHANNEL_FACTORY_H
