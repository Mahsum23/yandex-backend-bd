#pragma once

#include <unordered_map>
#include <string>
#include <iostream>

#include "json.hpp"

using json = nlohmann::json;

class FileJson
{
public:
	void LoadFromFile(const std::string& filename);
	void Import(json info);
	const json* FindIdLocation(const std::string& id) const;
	void Delete(const std::string& id, const std::string& date);
	void SaveToFile(const std::string& filename);
private:

	void UpdateSizeDate(const std::string& id, const std::string& date, int size_change);

	json root_;
	std::unordered_map<std::string, std::string> child_parent_;
};

json ConvertNestedChildrenMapToArray(const json& node);
