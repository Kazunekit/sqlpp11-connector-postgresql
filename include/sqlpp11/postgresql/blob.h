#pragma once

#include <sqlpp11/postgresql/connection.h>

namespace sqlpp
{
  template <>
  struct serializer_t<postgresql::context_t, blob_operand>
  {
    using _serialize_check = consistent_t;
    using Operand = blob_operand;

    static postgresql::context_t& _(const Operand& t, postgresql::context_t& context)
    {
      context << "\'" << context.escape(context.escape(t._t)) << "\'";

      return context;
    }
  };

}  // namespace sqlpp
