#include "pch.h"
#if 1
#include "ui.h"

namespace ui
{
	stock::cvar_t* branding;
	stock::cvar_t* cg_drawDisconnect;
	stock::cvar_t* cg_drawWeaponSelect;

	utils::hook::detour CG_DrawWeaponSelect_hook;
		
	static void CG_DrawDisconnect_stub()
	{
		if (!cg_drawDisconnect->integer)
			return;
		utils::hook::invoke<void>(ABSOLUTE_CGAME_MP(0x30015450));
	}

	static void CG_DrawWeaponSelect_stub()
	{
		if (!cg_drawWeaponSelect->integer)
			return;
		CG_DrawWeaponSelect_hook.invoke();
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
	
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			branding = stock::Cvar_Get("branding", "1", stock::CVAR_ARCHIVE);
			cg_drawWeaponSelect = stock::Cvar_Get("cg_drawWeaponSelect", "1", stock::CVAR_ARCHIVE);
			cg_drawDisconnect = stock::Cvar_Get("cg_drawDisconnect", "1", stock::CVAR_ARCHIVE);

			// Replace "k" by "KB" in SCR_DrawDemoRecording
			utils::hook::set(0x00416b82 + 1, "RECORDING %s: %iKB");

			scheduler::loop(draw_branding, scheduler::pipeline::renderer);
		}

		void post_cgame() override
		{
			CG_DrawWeaponSelect_hook.create(ABSOLUTE_CGAME_MP(0x30037790), CG_DrawWeaponSelect_stub);

			utils::hook::jump(ABSOLUTE_CGAME_MP(0x300159CC), CG_DrawDisconnect_stub);
			utils::hook::jump(ABSOLUTE_CGAME_MP(0x300159D4), CG_DrawDisconnect_stub);
		}
	};
}

REGISTER_COMPONENT(ui::component)
#endif