#include "server.hpp"

using json = nlohmann::json;

int main()
{
	FileServer fs;
	fs.Start();
	//dbtest();
}

void dbtest()
{
	using namespace std::string_literals;
	const auto conn = tao::pq::connection::create("dbname=files");

	conn->execute(R"(DROP TABLE IF EXISTS files)");
	
	conn->execute(R"(CREATE TABLE IF NOT EXISTS files ( 
			id TEXT PRIMARY KEY,
			info jsonb,
			parent_id TEXT
	))");

	conn->prepare("import_file", "INSERT INTO files ( id, info, parent_id ) VALUES ( $1, $2::text::jsonb, $3 )");
	conn->prepare("update_size", update_size);
	conn->prepare("get_info_id", get_info_by_id);
	conn->prepare("add_to_children", add_to_children);
	conn->prepare("update_date", update_date);
	conn->prepare("delete_by_id", delete_by_id);
	conn->prepare("delete_by_id_from_children", delete_by_id_from_children);

	conn->execute("import_file", "1a"s, "{\"children\": [], \"type\": \"folder\", \"size\": 0, \"date\": 9}"s, "null"s);
	conn->execute("import_file", "3b"s, "{\"children\": [], \"type\": \"file\", \"size\": 40, \"date\": 10}"s, "2c"s);
	conn->execute("import_file", "2c"s, "{\"children\": [\"3b\"], \"type\": \"folder\", \"size\": 0, \"date\": 11}"s, "1a"s);
	conn->execute("update_date", "3b"s, 100);
	auto res = conn->execute("get_info_id", "3b"s);

	conn->execute("delete_by_id", "3b");
	conn->execute("delete_by_id_from_children", "3b");
}

