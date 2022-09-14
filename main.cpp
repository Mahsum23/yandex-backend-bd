#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <list>
#include <stack>

#include "asio.hpp"
#include "crow.h"
#include "json.hpp"
#include "file_json.h"
#include "tao/pq.hpp"

using json = nlohmann::json;

int main()
{
	crow::SimpleApp app;
	FileJson current_data;
	current_data.LoadFromFile("data.json");

	const auto conn = tao::pq::connection::create("host=localhost user=postgres password=2323rm dbname=files");
	conn->execute(R"(CREATE TABLE IF NOT EXISTS users ( 
			id TEXT PRIMARY KEY,
			info jsonb,
			parent_path TEXT
	))" );

	conn->prepare( "import_file", "INSERT INTO files ( id, info, parent_path ) VALUES ( $1, $2, $3 )" );
	

	// $1 - string (child) $2 - string (parent)
	std::string add_to_children = R"(UPDATE files SET info = jsonb_insert(info, '{children, -1}', $1, true) WHERE id = $2)"; 


	// update date in the same query??? 
	// $1 - string (added id), $2 - int (size of added)
	std::string update_size = R"(WITH RECURSIVE node AS 
	( 
		SELECT * FROM files WHERE id = $1 
		UNION ALL 
		SELECT files.id, files.info, files.parent_path 
		FROM files, node 
		WHERE node.parent_path = files.id 
	) 
	UPDATE files 
	SET info = jsonb_set(files.info, '{size}', ((files.info->>'size')::int + $2)::text::jsonb) 
	FROM node WHERE node.parent_path = files.id )";

	// $1 - string $2 - string
	std::string update_date = R"(WITH RECURSIVE node AS 
	( 
		SELECT * FROM files WHERE id = $1 
		UNION ALL 
		SELECT files.id, files.info, files.parent_path 
		FROM files, node 
		WHERE node.parent_path = files.id 
	) 
	UPDATE files 
	SET info = jsonb_set(files.info, '{date}', $2) 
	FROM node WHERE node.parent_path = files.id )";

	conn->prepare( "add_to_children",  add_to_children);
	conn->prepare( "update_size",  update_size);
	conn->prepare( "update_date",  update_date);

	CROW_ROUTE(app, "/health")([]()
		{
			return crow::response(200, "working fine...");
		});
	CROW_ROUTE(app, "/imports").methods(crow::HTTPMethod::Post)([&](const crow::request& req)
		{
			try
			{
				auto json = json::parse(req.body);
				for (auto& entry : json["items"])
				{
					entry["children"] = json::array();
					entry["date"] = json["updateDate"];
					conn->execute("import_file",  entry["id"].dump(), entry.dump(), entry["parentId"].dump());
					// insert id to children of node parentId
					conn->execute("add_to_children",  entry["id"].dump(), entry["parentId"].dump());
					// increment size and set data recursively
					conn->execute("update_size",  entry["id"].dump(), entry["size"].dump());
					conn->execute("update_date",  entry["id"].dump(), entry["date"].dump());
				}
				
			}
			catch (...)
			{
				return crow::response(400, "Validation Failed");
			}
			return crow::response(200);
		});

	CROW_ROUTE(app, "/nodes/<string>").methods(crow::HTTPMethod::Get)([&](const std::string& id)
		{
			auto target = "{}"_json;
			try
			{
				target = ConvertNestedChildrenMapToArray(current_data.FindIdLocation(id)->at(id));
			}
			catch (...)
			{
				return crow::response::response(404, "Item not found");
			}
			//std::cout << target.dump() << std::endl << std::endl;
			return crow::response(200, target.dump());
		});

	CROW_ROUTE(app, "/delete/<string>").methods(crow::HTTPMethod::Delete)([&](const crow::request& req, const std::string& id)
		{
			try
			{
				current_data.Delete(id, req.url_params.get("date"));
			}
			catch (...)
			{
				return crow::response(404, "Item not found");
			}
			return crow::response(200, "Item Deleted");
		});

	app.bindaddr("127.0.0.1").port(8080).multithreaded().run();
}