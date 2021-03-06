#pragma once
#include <fstream>
#include "json.hpp"

inline void InitConfig()
{
	std::ifstream file(ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/PlayerList/config.json");
	if (!file.is_open())
	{
		Log::GetLog()->error("Failed to load config.json");
		throw;
	}

	nlohmann::json Config;

	try
	{
		file >> Config;
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error(error.what());
		throw;
	}

	file.close();
	int i = 0;

	auto MessagesConfig = Config["Messages"];
	for (nlohmann::json::iterator it = MessagesConfig.begin(); it != MessagesConfig.end(); ++it)
	{
		Messages[i++] = ArkApi::Tools::Utf8Decode(*it);
	}
}
