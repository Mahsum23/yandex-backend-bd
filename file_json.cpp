#include <fstream>
#include <stack>

#include "file_json.h"

void FileJson::LoadFromFile(const std::string& filename)
{
	std::ifstream infile("data.json");
	if (!infile || (infile.peek() == -1))
	{
		std::ofstream out("data.json");
		out << "{ }";
	}
	infile.open("data.json");
	root_ = json::parse(infile);
	infile.close();
}

void FileJson::SaveToFile(const std::string& filename)
{
	std::ofstream outfile(filename);
	outfile << root_;
	outfile.flush();
	outfile.close();
}

// returns map containing an element with id
const json* FileJson::FindIdLocation(const std::string& id) const
{
	if (child_parent_.count(id) == 0)
		return &root_;
	std::string cur_id = child_parent_.at(id);
	std::list<std::string> dir_list;
	dir_list.push_front(cur_id);

	// at the end parent_id is root
	while (child_parent_.count(cur_id) != 0)
	{
		cur_id = child_parent_.at(cur_id);
		dir_list.push_front(cur_id);
	}

	std::cout << root_ << std::endl;

	auto location = &(root_.at(*dir_list.begin()));
	for (auto it = std::next(dir_list.begin()); it != dir_list.end(); ++it)
	{
		location = &(*location)["children"].at(*it);
	}
	return location;
}

void FileJson::Import(json info)
{
	for (auto& entry : info["items"])
	{
		entry["date"] = info["updateDate"];
		entry["url"];
		//entry["children"] = json::array();
		if (entry["parentId"].is_null())
		{
			root_[entry["id"]] = entry;
			root_[entry["id"]]["size"] = 0;
		}
		else
		{

			child_parent_[entry["id"]] = entry["parentId"];
			std::string parent_id = entry["parentId"];

			std::list<std::string> dir_list;
			dir_list.push_front(parent_id);

			// at the end parent_id is root
			while (child_parent_.count(parent_id) != 0)
			{
				parent_id = child_parent_[parent_id];
				dir_list.push_front(parent_id);
			}


			auto location = &(root_[*dir_list.begin()]["children"]);

			// update size and date
			root_[*dir_list.begin()]["size"] = root_[*dir_list.begin()]["size"].get<int>() +
				(entry["size"].is_null() ? 0 : entry["size"].get<int>());
			root_[*dir_list.begin()]["date"] = entry["date"];

			for (auto it = std::next(dir_list.begin()); it != dir_list.end(); ++it)
			{
				(*location)[*it]["size"] = (*location)[*it]["size"].get<int>() +
					(entry["size"].is_null() ? 0 : entry["size"].get<int>());
				(*location)[*it]["date"] = entry["date"];
				location = &(*location)[*it]["children"];
			}

			if (entry["size"].is_null())
				entry["size"] = 0;
			(*location)[entry["id"]] = entry;
		}

	}
}

void FileJson::Delete(const std::string& id, const std::string& date)
{
	auto ptr = FindIdLocation(id);
	if (ptr != &root_)
	{
		UpdateSizeDate(id, date, ptr->at("size"));
	}
	*(const_cast<json*>(ptr)) = json();
}

void FileJson::UpdateSizeDate(const std::string& id, const std::string& date, int size_change)
{
	std::string cur_id = id;
	std::list<std::string> dir_list;
	dir_list.push_front(cur_id);

	// at the end parent_id is root
	while (child_parent_.count(cur_id) != 0)
	{
		cur_id = child_parent_[cur_id];
		dir_list.push_front(cur_id);
	}


	auto location = &(root_[*dir_list.begin()]["children"]);

	// update size and date
	root_[*dir_list.begin()]["size"] = root_[*dir_list.begin()]["size"].get<int>() + size_change;
	root_[*dir_list.begin()]["date"] = date;

	for (auto it = std::next(dir_list.begin()); it != dir_list.end(); ++it)
	{
		(*location)[*it]["size"] = (*location)[*it]["size"].get<int>() + size_change;
		(*location)[*it]["date"] = date;
		location = &(*location)[*it]["children"];
	}
}

json ConvertNestedChildrenMapToArray(const json& node)
{

	if (node.is_null())
		return {};

	json res = node;
	std::stack<json*> st;

	auto arr = json::array();
	for (auto& j : res["children"])
	{
		arr.push_back(j);
	}

	res["children"] = (arr.empty() ? json() : move(arr));
	st.push(&res);
	while (!st.empty())
	{
		json& cur = *st.top();
		st.pop();
		if (cur["children"].is_null())
			continue;
		auto arr = json::array();
		for (auto& j : cur["children"])
		{
			arr.push_back(move(j));
		}
		cur["children"] = (arr.empty() ? json() : move(arr));
		for (auto& j : cur["children"])
		{
			st.push(&j);
		}
	}
	return res;
}
