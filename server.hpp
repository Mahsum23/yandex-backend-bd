#pragma once

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <list>
#include <stack>
#include <memory>

#include "asio.hpp"
#include "crow.h"
#include "json.hpp"
#include "file_json.h"
#include "tao/pq.hpp"
#include "db_queries.hpp"
#include "date/date.h"

class FileServer
{
    using json = nlohmann::json;
public:
    FileServer(const std::string& address="127.0.0.1", uint16_t port=8080);
    void Start();
    void ImportFile(json& json);
    void DeleteFile(const std::string &id);
    std::string GetFileInfo(const std::string &id);
private:

    void InitRouteImport();
    void InitRouteDelete();
    void InitRouteGetNodes();
    std::shared_ptr<tao::pq::connection> conn_;
    crow::SimpleApp app_;
    uint16_t port_ = 0;
    std::string address_ = "";
};