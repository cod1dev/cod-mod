#include "pch.h"
#if 1
#include "ui.h"

namespace ui
{
	stock::cvar_t* branding;
	stock::cvar_t* cg_drawDisconnect;
	stock::cvar_t* cg_drawWeaponSelect;
	stock::cvar_t* cg_drawFPS_custom;

	utils::hook::detour hook_CG_DrawWeaponSelect;
	utils::hook::detour hook_CG_DrawFPS;
		
	static void stub_CG_DrawDisconnect()
	{
		if (!cg_drawDisconnect->integer)
			return;
		utils::hook::invoke<void>(ABSOLUTE_CGAME_MP(0x30015450));
	}

	static void stub_CG_DrawWeaponSelect()
	{
		if (!cg_drawWeaponSelect->integer)
			return;
		hook_CG_DrawWeaponSelect.invoke();
	}

	static void draw_branding()
	{
		if (!branding->integer)
			return;

		const auto x = 1;
		const auto y = 10;
		const auto fontID = 1;
		const auto scale = 0.21f;
		float color[4] = { 1.f, 1.f, 1.f, 0.80f };
		float color_shadow[4] = { 0.f, 0.f, 0.f, 0.80f };
		std::string text = std::string(MOD_NAME) + ".com";

		stock::SCR_DrawString(x + 1, y + 1, fontID, scale, color_shadow, text.c_str(), NULL, NULL, NULL); // Shadow first
		stock::SCR_DrawString(x, y, fontID, scale, color, text.c_str(), NULL, NULL, NULL);
	}
	
	static void stub_CG_DrawFPS(float y)
	{
		if (!cg_drawFPS_custom->integer)
		{
			hook_CG_DrawFPS.invoke(y);
			return;
		}
		
		// TODO: Try to let the original function do the calculation, to replace only the display
		static int previousTimes[stock::FPS_FRAMES];
		static int index;
		int i, total;
		int fps;
		static int previous;
		int t, frameTime;
		
		t = (int)(*stock::cgame_mp::syscall)(6); // CG_MILLISECONDS
		frameTime = t - previous;
		previous = t;

		previousTimes[index % stock::FPS_FRAMES] = frameTime;
		index++;
		if (index > stock::FPS_FRAMES)
		{
			total = 0;
			for (i = 0; i < stock::FPS_FRAMES; i++)
			{
				total += previousTimes[i];
			}
			if (!total)
			{
				total = 1;
			}
			fps = 1000 * stock::FPS_FRAMES / total;
			
			const auto x = 550;
			const auto _y = 11;
			const auto fontID = 1;
			const auto scale = 0.22f;
			float color[4] = { 1.f, 1.f, 1.f, 0.80f };
			float color_shadow[4] = { 0.f, 0.f, 0.f, 0.80f };
			std::string text = utils::string::va("FPS: %i", fps);

			stock::SCR_DrawString(x + 1, _y + 1, fontID, scale, color_shadow, text.c_str(), NULL, NULL, NULL); // Shadow first
			stock::SCR_DrawString(x, _y, fontID, scale, color, text.c_str(), NULL, NULL, NULL);
		}
	}
	
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			branding = stock::Cvar_Get("branding", "1", stock::CVAR_ARCHIVE);
			cg_drawWeaponSelect = stock::Cvar_Get("cg_drawWeaponSelect", "1", stock::CVAR_ARCHIVE);
			cg_drawDisconnect = stock::Cvar_Get("cg_drawDisconnect", "1", stock::CVAR_ARCHIVE);
			cg_drawFPS_custom = stock::Cvar_Get("cg_drawFPS_custom", "0", stock::CVAR_ARCHIVE);

			// Replace "k" by "KB" in SCR_DrawDemoRecording
			utils::hook::set(0x00416b82 + 1, "RECORDING %s: %iKB");

			scheduler::loop(draw_branding, scheduler::pipeline::renderer);
		}

		void post_cgame() override
		{
			hook_CG_DrawWeaponSelect.create(ABSOLUTE_CGAME_MP(0x30037790), stub_CG_DrawWeaponSelect);

			utils::hook::jump(ABSOLUTE_CGAME_MP(0x300159CC), stub_CG_DrawDisconnect);
			utils::hook::jump(ABSOLUTE_CGAME_MP(0x300159D4), stub_CG_DrawDisconnect);
			
			hook_CG_DrawFPS.create(ABSOLUTE_CGAME_MP(0x30014a00), stub_CG_DrawFPS);
		}
	};
}

REGISTER_COMPONENT(ui::component)
#endif