#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<void ()> Com_Frame{ 0, 0x00437f40 };
	WEAK symbol<void (int msec)> SV_Frame{ 0, 0x0045b1d0 };
	WEAK symbol<void (float x, float y, int font, float scale, float* color, const char* text, float spacing, int limit, int flags)> SCR_DrawString{ 0x0, 0x4DF570 };
	WEAK symbol<cvar_t* (const char* name)> Cvar_FindVar{ 0, 0x00439280 };
	WEAK symbol<cvar_t* (const char* name, const char* value, int flags)> Cvar_Get{ 0, 0x00439350 };
	WEAK symbol<cvar_t* (const char* name, const char* value)> Cvar_Set{ 0, 0x00439650 };
	WEAK symbol<int ()> Sys_Milliseconds{ 0, 0x004659d0 };
	WEAK symbol<LRESULT CALLBACK (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> MainWndProc{ 0, 0x466BE0 };
	WEAK symbol<void ()> IN_DeactivateMouse{ 0, 0x4616b0 };
	WEAK symbol<void ()> IN_ActivateMouse{ 0, 0x461730 };
	WEAK symbol<void (const char* msg, ...)> Com_Printf{ 0, 0x004357b0 };
	WEAK symbol<void ()> CG_ServerCommand{ 0, 0x3002e0d0, OFFSET_CGAME_MP };
	WEAK symbol<void(const char* cmd_name, xcommand_t function)> Cmd_AddCommand{ 0, 0x00428840 };






	/*WEAK symbol<void(const char* pakSums, const char* pakNames)> FS_PureServerSetLoadedPaks{ 0, 0x0043c290 };
	WEAK symbol <void(const char* pakNames)> __fastcall FS_PureServerSetReferencedPaks{ 0, 0x0043c530 };*/



}