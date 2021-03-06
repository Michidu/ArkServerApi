#include "PluginChecker.h"
#include "PluginCheckerCommands.h"
#pragma comment(lib, "ArkApi.lib")

void Init()
{
	Log::Get().Init("PluginChecker");
	InitCommands();
	CheckLatestPluginsVersions(nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_PROCESS_DETACH:
		RemoveCommands();
		break;
	}
	return TRUE;
}