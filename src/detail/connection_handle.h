/**
 * Copyright © 2014-2015, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLPP_POSTGRESQL_CONNECTION_HANDLE_H
#define SQLPP_POSTGRESQL_CONNECTION_HANDLE_H

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <libpq-fe.h>
#include <sqlpp11/postgresql/visibility.h>

namespace sqlpp
{
  namespace postgresql
  {
    // Forward declaration
    struct connection_config;

    namespace detail
    {
      struct DLL_LOCAL connection_handle
      {
        const std::shared_ptr<connection_config> config;
        PGconn* postgres{nullptr};
		std::set<std::string> prepared_statement_names;

        connection_handle(const std::shared_ptr<connection_config>& config);
        ~connection_handle();
        connection_handle(const connection_handle&) = delete;
        connection_handle(connection_handle&&) = delete;
        connection_handle& operator=(const connection_handle&) = delete;
        connection_handle& operator=(connection_handle&&) = delete;

        PGconn* native() const
        {
          return postgres;
        }

        void deallocate_prepared_statement(const std::string& name);

        std::string escape(const std::string& s) const
        {
          // Escape strings
          std::string result;
          result.resize((s.size() * 2) + 1);

          int err;
          size_t length = PQescapeStringConn(postgres, &result[0], s.c_str(), s.size(), &err);
          result.resize(length);
          return result;
        }

        std::vector<uint8_t> escape_bytes(const std::vector<uint8_t>& s) const
        {
          size_t length = 0;

          std::unique_ptr<unsigned char, void (*)(unsigned char*)> buf{
              PQescapeByteaConn(postgres, s.data(), s.size(), &length), [](unsigned char* p) { PQfreemem(p); }};
          if (buf.get() == nullptr)
            throw std::bad_alloc{};

          return {buf.get(), buf.get() + length};
        }

        std::string escape(const std::vector<uint8_t>& s) const
        {
          auto buf = escape_bytes(s);

          return {buf.begin(), buf.end()};
        }

        std::vector<uint8_t> unescape_bytes(const std::vector<uint8_t>& s) const
        {
          size_t length = s.size();
          const std::unique_ptr<unsigned char, void (*)(unsigned char*)> buf{PQunescapeBytea(s.data(), &length),
                                                                             [](unsigned char* p) { PQfreemem(p); }};

          return {buf.get(), buf.get() + length};
        }
      };
    }  // namespace detail
  }    // namespace postgresql
}  // namespace sqlpp

#endif
