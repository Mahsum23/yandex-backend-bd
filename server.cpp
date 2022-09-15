#include "server.hpp"

using json = nlohmann::json;


FileServer::FileServer(const std::string& address, uint16_t port)
    : address_(address), port_(port)
{
    conn_ = tao::pq::connection::create("dbname=files");
    conn_->execute(R"(CREATE TABLE IF NOT EXISTS files ( 
			id TEXT,
			info jsonb,
			parent_id TEXT
	))");

    conn_->prepare("import_file", import_file);
	conn_->prepare("add_to_children", add_to_children);
	conn_->prepare("update_size", update_size);
	conn_->prepare("update_date", update_date);
	conn_->prepare("get_info_by_id", get_info_by_id);
	conn_->prepare("delete_by_id", delete_by_id);
	conn_->prepare("delete_by_id_from_children", delete_by_id_from_children);

    InitRouteDelete();
    InitRouteImport();
    InitRouteGetNodes();
}

void FileServer::Start()
{
    app_.bindaddr(address_).port(port_).multithreaded().run();
}

void FileServer::InitRouteImport()
{
    CROW_ROUTE(app_, "/imports").methods(crow::HTTPMethod::Post)([&](const crow::request &req)
	{
		try
		{
            auto json = json::parse(req.body);
			ImportFile(json);
		}
		catch (std::exception& exc)
		{
			std::cerr << exc.what() << std::endl;
			return crow::response(400, "Validation Failed");
		}
		return crow::response(200); 
	});
}
void FileServer::InitRouteDelete()
{
	CROW_ROUTE(app_, "/delete/<string>").methods(crow::HTTPMethod::Delete)([&](const crow::request &req, const std::string &id)
	{
		try
		{
			DeleteFile(id);
		}
		catch (std::exception& exc)
		{
            std::cerr << exc.what() << std::endl;
			return crow::response(404, "Item not found");
		}
		return crow::response(200, "Item Deleted"); 
	});
}


void FileServer::InitRouteGetNodes()
{
    CROW_ROUTE(app_, "/nodes/<string>").methods(crow::HTTPMethod::Get)([&](const std::string &id)
	{
		try
		{
            return crow::response(200, GetFileInfo(id)); 
		}
		catch (std::exception& exc)
		{
			std::cerr << exc.what() << std::endl;
			return crow::response(404, "Item not found");
		}
		
	});
}

void FileServer::ImportFile(json& json)
{
    for (auto& entry : json["items"])
    {
        entry["children"] = json::array();
        entry["date"] = json["updateDate"];
        if (entry["size"].is_null())
        {
            entry["size"] = 0;
        }
        entry["url"];
        conn_->execute("import_file",  entry["id"].dump(), entry.dump(), entry["parentId"].dump());

        // insert id to children of node parentId
        conn_->execute("add_to_children",  entry["id"].dump(), entry["parentId"].dump());

        // increment size and set data recursively
        if (entry["size"].dump() != "0")
        {
            conn_->execute("update_size",  entry["id"].dump(), std::stoi(entry["size"].dump()));
        }
        conn_->execute("update_date",  entry["id"].dump(), entry["date"].dump());
    }
}

void FileServer::DeleteFile(const std::string& id)
{
    conn_->execute("delete_by_id", "\"" + id + "\"");
    conn_->execute("delete_by_id_from_children", "\"" + id + "\"");	
}

std::string FileServer::GetFileInfo(const std::string &id)
{
    auto root_json = "{}"_json;
    auto root_info = conn_->execute("get_info_by_id", "\"" + id + "\"");
    root_json = json::parse(root_info.as<std::string>());
    std::stack<json*> nodes;
    nodes.push(&root_json);
    while (!nodes.empty())
    {
        auto cur_json = nodes.top();
        nodes.pop();
        if ((*cur_json)["children"].size() == 0)
        {
            (*cur_json)["children"] = json();
        }
        for (auto& id : (*cur_json)["children"])
        {
            auto info = conn_->execute(get_info_by_id, id.dump());
            auto child_json = json::parse(info.as<std::string>());
            *(&id) = child_json;
            nodes.push(&id);
        }
    }
    return root_json.dump();
}

