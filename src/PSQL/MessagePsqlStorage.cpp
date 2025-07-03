// MessagePsqlStorage.cpp
// Refactored: one table per *signal name* with schema matching MessageType
// * MT_SIGNAL   : no payload column
// * MT_BOOL     : value BOOLEAN
// * MT_SIGNED_INT: value INT
// * MT_STRING   : value TEXT
//
// NOTE
// -----
// • MessagePsqlStorage.hpp must be updated accordingly:
//     - CreateTableIfNotExists now takes (const std::string& table, MessageType type)
//     - TableExists signature simplified to TableExists(const std::string& table)
// • Consider converting the INSERT logic to prepared‑statements for extra safety/perf.
//

#include <pqxx/pqxx>      // pqxx::connection, pqxx::work
#include <cctype>         // std::isalnum
#include <sstream>
#include <stdexcept>

#include "MessagePsqlStorage.hpp"
#include "Message.hpp"

//--------------------------------- ctor / dtor --------------------------------
MessagePsqlStorage::MessagePsqlStorage(const std::string& dbName,
                                       const std::string& user,
                                       const std::string& passwd,
                                       const std::string& hostaddr,
                                       uint16_t           port,
                                       std::ostream&      error_strm, 
                                       std::ostream&      warn_strm, 
                                       std::ostream&      info_strm, 
                                       std::ostream&      dbg_strm)
    : error_strm(error_strm), warn_strm(warn_strm), info_strm(info_strm), dbg_strm(dbg_strm)
{
    std::ostringstream conninfo;
    conninfo << "dbname=" << dbName
             << " user=" << user
             << " password=" << passwd
             << " hostaddr=" << hostaddr
             << " port=" << port;

    conn = std::make_unique<pqxx::connection>(conninfo.str());
    if (!conn->is_open()) {
        error_strm << "Cannot open PostgreSQL connection to dbname=" << dbName
                   << ", user=" << user 
                   << ", hostaddr=" << hostaddr 
                   << ", port=" << port 
                   << std::endl;
    } else {
        info_strm << "PostgreSQL connection opened successfully to dbname=" << dbName
                  << ", user=" << user 
                  << ", hostaddr=" << hostaddr 
                  << ", port=" << port 
                  << std::endl;
    }
}

MessagePsqlStorage::~MessagePsqlStorage() = default;

//----------------------------------- helpers ----------------------------------
std::string MessagePsqlStorage::SanitizeTypeName(const std::string& typeName) const
{
    std::string sanitized = typeName;
    for (auto &c : sanitized) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            c = '_';
        }
    }
    return sanitized;
}

bool MessagePsqlStorage::TableExists(const std::string& table)
{
    pqxx::work txn(*conn);
    pqxx::result r = txn.exec("SELECT to_regclass('public." + table + "')");
    txn.commit();
    return !r.empty() && !r[0][0].is_null();
}

// Build the correct CREATE TABLE statement for the given MessageType.
void MessagePsqlStorage::CreateTableIfNotExists(const std::string& table, MessageType type)
{
    if (TableExists(table)) return;

    pqxx::work txn(*conn);
    std::ostringstream sql;
    sql << "CREATE TABLE IF NOT EXISTS \"" << table << "\" ("
        << "timestamp TIMESTAMPTZ DEFAULT NOW(), "
        << "idSender INT, "
        << "idTarget INT";

    switch (type) {
        case MT_BOOL:
            sql << ", value BOOLEAN";
            break;
        case MT_SIGNED_INT:
            sql << ", value INT";
            break;
        case MT_STRING:
            sql << ", value TEXT";
            break;
        case MT_SIGNAL:     // NOP – no payload column
        default:
            break;
    }
    sql << ");";

    txn.exec(sql.str());
    txn.commit();
}

//-------------------------------- message I/O ----------------------------------
void MessagePsqlStorage::StoreMessage(const Message &msg)
{
    const std::string table = SanitizeTypeName(Message::MsgCode2Name(msg.msgCode));
    const MessageType type  = msg.GetType();

    // Ensure table exists with proper schema for this type
    CreateTableIfNotExists(table, type);

    pqxx::work txn(*conn);
    const bool hasPayload = (type != MT_SIGNAL);

    std::string valueStr;
    if (hasPayload) {
        switch (type) {
            case MT_BOOL: {
                bool val; msg.GetValue(val);
                valueStr = val ? "true" : "false";
                break;
            }
            case MT_SIGNED_INT: {
                int val; msg.GetValue(val);
                valueStr = std::to_string(val);
                break;
            }
            case MT_STRING: {
                std::string val; msg.GetValue(val);
                valueStr = txn.esc(val);
                break;
            }
            default:
                break;
        }
    }

    // Build INSERT
    std::ostringstream sql;
    sql << "INSERT INTO \"" << table << "\" (idSender, idTarget";
    if (hasPayload) sql << ", value";
    sql << ") VALUES ("
        << static_cast<int>(msg.idSender) << ", "
        << static_cast<int>(msg.idTarget);

    if (hasPayload) {
        if (type == MT_STRING) {
            sql << ", '" << valueStr << "'";   // already escaped
        } else {
            sql << ", " << valueStr;            // bool/int – no quotes
        }
    }
    sql << ");";

    txn.exec(sql.str());
    txn.commit();
}
