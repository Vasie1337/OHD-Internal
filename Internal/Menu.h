#pragma once

class c_menu {
public:
	static inline bool showMenu = false;
	union u_vars
	{
		static inline bool EspSkeleton = true;
		static inline bool ShowName = true;

		static inline bool SilentAim = true;
		static inline float FovRadius = 120.f;
		static inline bool VisCheck = true;

		static inline const char* BoneList[] = { "Head", "Body", "Feet" };
		static inline int AimBone = 0;


	};

	static void Tick()
	{
		if (showMenu)
		{
			ImGui::Begin("Vasie Internal", &showMenu);
			{
				ImGui::TextDisabled("Visuals");
				ImGui::Checkbox("EspSkeleton", &u_vars::EspSkeleton);
				ImGui::Checkbox("ShowName", &u_vars::ShowName);

				ImGui::TextDisabled("Aim");
				ImGui::Checkbox("SilentAim", &u_vars::SilentAim);
				ImGui::Checkbox("VisCheck", &u_vars::VisCheck);
				ImGui::DragFloat("FovRadius", &u_vars::FovRadius);

				ImGui::ListBox("AimBone", &u_vars::AimBone, u_vars::BoneList, sizeof(u_vars::BoneList) / sizeof(u_vars::BoneList[0]));
			}
			ImGui::End();
		}
	}
};