#pragma once

#include "public.h"

#include <yt/yt/client/api/rpc_proxy/public.h>

#include <yt/yt/client/table_client/public.h>

namespace NYT::NArrow {

////////////////////////////////////////////////////////////////////////////////

NApi::NRpcProxy::IRowStreamEncoderPtr CreateArrowRowStreamEncoder(
    NTableClient::TTableSchemaPtr schema,
    NTableClient::TNameTablePtr nameTable,
    NApi::NRpcProxy::IRowStreamEncoderPtr fallbackEncoder);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NArrow
