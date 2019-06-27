#include "BlobSample.h"
#include <sqlpp11/postgresql/postgresql.h>
#include <sqlpp11/sqlpp11.h>

#include <functional>
#include <iostream>
#include <random>

namespace
{
  template <class Db>
  void prepare_table(Db&& db)
  {
    // prepare test with timezone
    db.execute("DROP TABLE IF EXISTS blob_sample");
    db.execute(R"(CREATE TABLE blob_sample (
                 id SERIAL,
                 data bytea
               ))");
  }

}  // namespace

namespace sql = sqlpp::postgresql;
const auto blob = BlobSample{};
/*
 * 500,000,000 seems to be max,
 * since it doubles when being escaped
 * Could also depend on pgsql server config
 */
constexpr size_t blob_size = 1000 * 1000ul;
constexpr size_t blob_small_size = 999;

void verify_blob(sql::connection& db, const std::vector<uint8_t>& data, uint64_t id)
{
  auto result = db(select(blob.data).from(blob).where(blob.id == id));
  const auto& result_row = result.front();
  auto result_blob = db.unescape_bytes({result_row.data.blob, result_row.data.blob + result_row.data.len});
  std::cerr << "Checking id: " << id << std::endl;
  std::cerr << "Insert size: " << data.size() << std::endl;
  std::cerr << "Select size: " << result_blob.size() << std::endl;
  if (data.size() != result_blob.size())
  {
    std::cerr << "Size mismatch" << std::endl;

    throw std::runtime_error("Size mismatch " + std::to_string(data.size()) +
                             " != " + std::to_string(result_blob.size()));
  }
  std::cerr << "Verifying content" << std::endl;
  if (data != result_blob)
  {
    std::cout << "Content mismatch ([row] original -> received)" << std::endl;

    for (size_t i = 0; i < data.size(); i++)
    {
      if (data[i] != result_blob[i])
      {
        std::cerr << "[" << i << "] " << static_cast<int>(data.at(i)) << " -> " << static_cast<int>(result_blob.at(i))
                  << std::endl;
      }
    }
    throw std::runtime_error("Content mismatch");
  }
}

int BlobTest(int, char*[])
{
  auto config = std::make_shared<sql::connection_config>();

#ifdef WIN32
  config->dbname = "test";
  config->user = "test";
  config->password = "test";
  config->debug = true;
#else
  // TODO: assume there is a DB with the "username" as a name and the current user has "peer" access rights
  config->dbname = getenv("USER");
  config->user = config->dbname;
  config->debug = true;
#endif

  sql::connection db(config);
  prepare_table(db);

  std::cerr << "Generating data " << blob_size << std::endl;
  std::vector<uint8_t> data(blob_size);
  std::uniform_int_distribution<unsigned short> distribution(0, 255);
  std::mt19937 engine;
  auto generator = std::bind(distribution, engine);
  std::generate_n(data.begin(), blob_size, generator);

  std::vector<uint8_t> data_smaller(blob_small_size);
  std::generate_n(data_smaller.begin(), blob_small_size, generator);

  // If we use the bigger blob it will trigger SQLITE_TOOBIG for the query
  //  auto prep = db.prepare(sql::insert_into(blob).set(blob.data = parameter(blob.data)).returning(blob.id));
  //  prep.params.data = data_smaller;
  //  auto id = db(prep).front().id;
  auto id = db(sql::insert_into(blob).set(blob.data = data_smaller).returning(blob.id)).front().id;

  auto prepared_insert = db.prepare(sql::insert_into(blob).set(blob.data = parameter(blob.data)).returning(blob.id));
  prepared_insert.params.data = data;
  auto prep_id = db(prepared_insert).front().id;
  prepared_insert.params.data.set_null();
  auto null_id = db(prepared_insert).front().id;

  verify_blob(db, data_smaller, id);
  verify_blob(db, data, prep_id);
  {
    auto result = db(select(blob.data).from(blob).where(blob.id == null_id));
    const auto& result_row = result.front();
    std::cerr << "Null blob is_null:\t" << std::boolalpha << result_row.data.is_null() << std::endl;
    std::cerr << "Null blob len == 0:\t" << std::boolalpha << (result_row.data.len == 0) << std::endl;
    std::cerr << "Null blob blob == nullptr:\t" << std::boolalpha << (result_row.data.blob == nullptr) << std::endl;
    if (!result_row.data.is_null() || result_row.data.len != 0 || result_row.data.blob != nullptr)
    {
      throw std::runtime_error("Null blob has incorrect values");
    }
  }
  return 0;
}
