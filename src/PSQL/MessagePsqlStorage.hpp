#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <pqxx/pqxx>
#include "Message.hpp"

// Class to store Message objects in PostgreSQL, with a table per message type (created on demand)
class MessagePsqlStorage {
public:
    MessagePsqlStorage(const std::string& dbName,
                        const std::string& user,
                        const std::string& passwd,
                        const std::string& hostaddr,
                        uint16_t port,
                        std::ostream& error_strm = std::cerr,
                        std::ostream& warn_strm = std::cerr,
                        std::ostream& info_strm = std::cerr,
                        std::ostream& dbg_strm = std::cerr);
    ~MessagePsqlStorage();

    // Store a message in the appropriate table (creates table if needed)
    void StoreMessage(const Message& msg);

    // Check if table for a message type exists
    bool TableExists(const std::string& typeName);

    // Create table for a message type (if not exists)
    void CreateTableIfNotExists(const std::string& table, MessageType type);

private:
    std::unique_ptr<pqxx::connection> conn;
    std::string SanitizeTypeName(const std::string& typeName) const;


        
    std::ostream& error_strm;
    std::ostream& warn_strm;
    std::ostream& info_strm;
    std::ostream& dbg_strm;
};
