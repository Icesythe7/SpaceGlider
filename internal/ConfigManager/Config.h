#pragma once
#include "../XorStr.h"
#include <Windows.h>
#include <string>
#include "json.hpp"
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>

class ConfigManager {
public:
	void load(HMODULE hModule) {
		auto path = getModulePath(hModule);
		auto settingsPath = path.parent_path() / xorstr("settings.json");
		std::ifstream inSettings(settingsPath);
		json = nlohmann::ordered_json::parse(inSettings, NULL, false);
		json.at(xorstr("ShowMenu")).get_to(this->ShowMenu);
		inSettings.close();
	}
	void save(HMODULE hModule) {
		auto path = getModulePath(hModule);
		auto settingsPath = path.parent_path() / xorstr("settings.json");
		std::ofstream outSettings(settingsPath);
		json = nlohmann::ordered_json{
			{xorstr("ShowMenu"), this->ShowMenu},
		};
		outSettings << std::setw(4) << json << std::endl;
		outSettings.close();
	}

	bool ShowMenu;

private:
	std::experimental::filesystem::path getModulePath(HMODULE hModule) {
		TCHAR buff[MAX_PATH];
		GetModuleFileName(hModule, buff, MAX_PATH);
		return std::experimental::filesystem::path(buff);
	}
	nlohmann::ordered_json json;
};
inline ConfigManager config;