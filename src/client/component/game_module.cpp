#include <std_include.hpp>
#include "game/game.hpp"
#include "loader/component_loader.hpp"
#include "game_module.hpp"

#include <utils/hook.hpp>

DWORD cgame_mp;
DWORD ui_mp;

namespace game_module
{
	utils::hook::detour nt_LoadLibraryA_hook;
	utils::hook::detour nt_GetModuleFileNameA_hook;
	utils::hook::detour nt_GetModuleFileNameW_hook;
	utils::hook::detour CG_ServerCommand_hook;

	utils::nt::library get_client_module()
	{
		static utils::nt::library client{ HMODULE(0x400000) };
		return client;
	}

	utils::nt::library get_host_module()
	{
		static utils::nt::library host{};
		return host;
	}








	// TODO: move to appropriate component
	void CG_ServerCommand_stub()
	{
		CG_ServerCommand_hook.invoke();
	}
	void hook_dll_cg_mp()
	{
		CG_ServerCommand_hook.create(cgame_mp_offset(0x3002e0d0), CG_ServerCommand_stub);
	}








	
	HMODULE WINAPI nt_LoadLibraryA_stub(LPCSTR lpLibFileName)
	{
		auto* orig = static_cast<decltype(LoadLibraryA)*>(nt_LoadLibraryA_hook.get_original());
		auto ret = orig(lpLibFileName);

		auto hModule = (DWORD)GetModuleHandleA(lpLibFileName);

		if (lpLibFileName != NULL)
		{
			auto fileName = PathFindFileNameA(lpLibFileName);
			if (!strcmp(fileName, "cgame_mp_x86.dll"))
			{
				cgame_mp = hModule;
				hook_dll_cg_mp();
			}
			else if (!strcmp(fileName, "ui_mp_x86.dll"))
			{
				ui_mp = hModule;
			}
		}

		return ret;
	}

	/*
	* Return original client filename, so GPU driver knows what game it is,
	* so if it has a profile for it, it will get enabled
	* (this prevents buffer overrun when glGetString(GL_EXTENSIONS) gets called)
	*/
	// For AMD and Intel HD Graphics
	DWORD WINAPI nt_GetModuleFileNameA_stub(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
	{
		auto* orig = static_cast<decltype(GetModuleFileNameA)*>(nt_GetModuleFileNameA_hook.get_original());
		auto ret = orig(hModule, lpFilename, nSize);
		
		if (!strcmp(PathFindFileNameA(lpFilename), "cod-mod.exe"))
		{
			std::filesystem::path path = lpFilename;
			auto binary = game::environment::get_client_filename();
			path.replace_filename(binary);
			std::string pathStr = path.string();
			std::copy(pathStr.begin(), pathStr.end(), lpFilename);
			lpFilename[pathStr.size()] = '\0';
		}

		return ret;
	}
	// For Nvidia
	DWORD WINAPI nt_GetModuleFileNameW_stub(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
	{
		auto* orig = static_cast<decltype(GetModuleFileNameW)*>(nt_GetModuleFileNameW_hook.get_original());
		auto ret = orig(hModule, lpFilename, nSize);

		int required_size = WideCharToMultiByte(CP_UTF8, 0, lpFilename, -1, nullptr, 0, nullptr, nullptr);
		std::string pathStr(required_size - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, lpFilename, -1, pathStr.data(), required_size, nullptr, nullptr);

		if (!strcmp(PathFindFileNameA(pathStr.c_str()), "cod-mod.exe"))
		{
			std::filesystem::path pathFs = pathStr;

			auto client_filename = game::environment::get_client_filename();
			pathFs.replace_filename(client_filename);
			pathStr = pathFs.string();

			required_size = MultiByteToWideChar(CP_UTF8, 0, pathStr.c_str(), -1, nullptr, 0);
			MultiByteToWideChar(CP_UTF8, 0, pathStr.c_str(), -1, lpFilename, required_size);
		}

		return ret;
	}
	
	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			get_host_module();
		}

		void post_load() override
		{
			assert(get_host_module() == get_client_module());
			
			const utils::nt::library kernel32("kernel32.dll");
			
			nt_LoadLibraryA_hook.create(kernel32.get_proc<HMODULE(WINAPI*)(LPCSTR)>("LoadLibraryA"), nt_LoadLibraryA_stub);
			nt_GetModuleFileNameA_hook.create(kernel32.get_proc<DWORD(WINAPI*)(HMODULE, LPSTR, DWORD)>("GetModuleFileNameA"), nt_GetModuleFileNameA_stub);
			nt_GetModuleFileNameW_hook.create(kernel32.get_proc<DWORD(WINAPI*)(HMODULE, LPWSTR, DWORD)>("GetModuleFileNameW"), nt_GetModuleFileNameW_stub);
		}
	};
}

REGISTER_COMPONENT(game_module::component)