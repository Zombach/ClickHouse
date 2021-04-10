#pragma once

#include <string>
#include <Core/Block.h>
#include <DataStreams/IBlockInputStream.h>
#include <mysqlxx/PoolWithFailover.h>
#include <mysqlxx/Query.h>
#include <Core/ExternalResultDescription.h>
#include <Core/Settings.h>

namespace DB
{

struct StreamSettings
{
    size_t max_read_mysql_rows;
    size_t max_read_bytes_size;
    bool auto_close = false;
    bool fetch_by_name = false;
    size_t default_num_tries_on_connection_loss = 5;

    StreamSettings(const Settings & settings);
    StreamSettings(const Settings & settings, bool auto_close_, bool fetch_by_name_);
    StreamSettings(const Settings & settings, bool auto_close_, bool fetch_by_name_, size_t max_retry_);

};

/// Allows processing results of a MySQL query as a sequence of Blocks, simplifies chaining
class MySQLBlockInputStream : public IBlockInputStream
{
public:
    MySQLBlockInputStream(
        const mysqlxx::PoolWithFailover::Entry & entry,
        const std::string & query_str,
        const Block & sample_block,
        const StreamSettings & settings_);

    String getName() const override { return "MySQL"; }

    Block getHeader() const override { return description.sample_block.cloneEmpty(); }

protected:
    MySQLBlockInputStream(const Block & sample_block_, const struct StreamSettings & settings);
    Block readImpl() override;
    void initPositionMappingFromQueryResultStructure();

    struct Connection
    {
        Connection(const mysqlxx::PoolWithFailover::Entry & entry_, const std::string & query_str);

        mysqlxx::PoolWithFailover::Entry entry;
        mysqlxx::Query query;
        mysqlxx::UseQueryResult result;
    };

    Poco::Logger * log;
    std::unique_ptr<Connection> connection;

    const std::unique_ptr<StreamSettings> settings;
    std::vector<size_t> position_mapping;
    ExternalResultDescription description;
};

/// Like MySQLBlockInputStream, but allocates connection only when reading is starting.
/// It allows to create a lot of stream objects without occupation of all connection pool.
/// Also makes attempts to reconnect in case of connection failures.
class MySQLWithFailoverBlockInputStream final : public MySQLBlockInputStream
{
public:

    MySQLWithFailoverBlockInputStream(
        mysqlxx::PoolWithFailoverPtr pool_,
        const std::string & query_str_,
        const Block & sample_block_,
        const StreamSettings & settings_);

private:
    void readPrefix() override;

    mysqlxx::PoolWithFailoverPtr pool;
    std::string query_str;
};

}
