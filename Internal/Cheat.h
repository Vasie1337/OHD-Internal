#pragma once

class c_cheat {
	static SDK::FVector2D BoneToScreenLocation(SDK::APlayerController* Controller, SDK::AHDPlayerCharacter* Player, int BoneId)
	{
		if (!Controller || !Player || !Player->Mesh)
			return { 0, 0 };

		SDK::USkeletalMeshComponent* Mesh = Player->Mesh;

		SDK::FVector BoneLocation = Mesh->GetSocketLocation(Mesh->GetBoneName(BoneId));
		SDK::FVector2D BoneScreenLocation;

		if (Controller->ProjectWorldLocationToScreen(BoneLocation, &BoneScreenLocation, false))
			return BoneScreenLocation;

		return { 0, 0 };
	}

	static float Distance2D(SDK::FVector2D main, SDK::FVector2D other)
	{
		float dx = main.X - other.X;
		float dy = main.Y - other.Y;
		return std::sqrt(dx * dx + dy * dy);
	}

public:
	static inline SDK::FVector2D Screen = {
			static_cast<float>(GetSystemMetrics(SM_CXSCREEN)),
			static_cast<float>(GetSystemMetrics(SM_CYSCREEN))
	};

	static inline SDK::FVector2D ScreenCenter = {
		Screen.X / 2.f,
		Screen.Y / 2.f
	};

	static void Tick()
	{
		ImGui::GetForegroundDrawList()->AddCircleFilled({ ScreenCenter.X, ScreenCenter.Y }, c_menu::u_vars::FovRadius, ImColor(30, 30, 30, 70));
		ImGui::GetForegroundDrawList()->AddCircle({ ScreenCenter.X, ScreenCenter.Y }, c_menu::u_vars::FovRadius, ImColor(30, 30, 30, 240));
		
		SDK::UWorld* Gworld = SDK::UWorld::GetWorld();
		if (!Gworld)
			return;

		SDK::UGameInstance* GameInstance = Gworld->OwningGameInstance;
		if (!GameInstance)
			return;

		SDK::ULocalPlayer* LocalPlayer = GameInstance->LocalPlayers[0];
		if (!LocalPlayer)
			return;

		SDK::APlayerController* PlayerController = LocalPlayer->PlayerController;
		if (!PlayerController)
			return;

		SDK::APlayerState* SelfPlayerState = PlayerController->PlayerState;
		if (!SelfPlayerState)
			return;

		SDK::APlayerController* SelfPlayerController = LocalPlayer->PlayerController;
		if (!SelfPlayerController)
			return;

		SDK::AHDPlayerCharacter* SelfPlayerCharacter = static_cast<SDK::AHDPlayerCharacter*>(PlayerController->AcknowledgedPawn);
		if (!SelfPlayerCharacter)
			return;

		SDK::ADFBaseItem* EquippedItem = SelfPlayerCharacter->EquippedItem;
		if (EquippedItem && EquippedItem->IsA(SDK::AHDBaseWeapon::StaticClass()))
		{
			SDK::AHDBaseWeapon* CurrWeapon = (SDK::AHDBaseWeapon*)EquippedItem;
			if (CurrWeapon)
			{
				CurrWeapon->MuzzleFX = nullptr;
				CurrWeapon->RecoilHandler = nullptr;
				CurrWeapon->MuzzlePSC = nullptr;
				CurrWeapon->ShellEjectFX = nullptr;
				CurrWeapon->WeaponMesh = nullptr;
			}
		}

		SDK::AGameStateBase* GameState = Gworld->GameState;
		if (!GameState)
			return;

		SDK::ULevel* PersistentLevel = Gworld->PersistentLevel;
		if (!PersistentLevel)
			return;

		uintptr_t Actors = *(uintptr_t*)((uintptr_t)PersistentLevel + 0x98);
		if (!Actors)
			return;

		int32_t ActorCount = *(int32_t*)((uintptr_t)PersistentLevel + 0xa0);
		if (!ActorCount)
			return;

		float ClosestDistance = FLT_MAX;
		SDK::AHDPlayerCharacter* Target = nullptr;
		SDK::FVector AimLocation;
		ImColor Color{};

		for (std::int32_t i = 0; i < ActorCount; i++)
		{
			SDK::AActor* Actor = *(SDK::AActor**)(Actors + i * sizeof(uintptr_t));
			if (!Actor)
				continue;

			if (Actor->IsA(SDK::AHDPlayerCharacter::StaticClass()))
			{
				SDK::AHDPlayerCharacter* PlayerCharacter = (SDK::AHDPlayerCharacter*)Actor;
				if (!PlayerCharacter)
					continue;

				SDK::APlayerState* PlayerState = PlayerCharacter->PlayerState;
				if (!PlayerState || SelfPlayerState == PlayerState)
					continue;

				std::string Name = PlayerState->GetPlayerName().ToString();
				bool EntityVisible = PlayerController->LineOfSightTo(PlayerCharacter, { 0.f,0.f,0.f }, false);

				SDK::FVector2D Feet = BoneToScreenLocation(PlayerController, PlayerCharacter, 61);
				SDK::FVector2D Head = BoneToScreenLocation(PlayerController, PlayerCharacter, 48);

				if (!Head.X || !Head.Y)
					continue;

				float height = Head.Y - Feet.Y;
				float width = height / 4.f;

				Color = ImColor(255, 255, 20);

				if (EntityVisible)
					Color = ImColor(255, 20, 20);

				if (PlayerCharacter->TeamNum == SelfPlayerCharacter->TeamNum)
					Color = ImColor(20, 255, 20);

				if (c_menu::u_vars::EspSkeleton)
				{
					std::vector<int> LeftArmtoRightArm = { 24, 6, 5, 27, 45 };
					std::vector<int> HeadtoLeftFoot = { 5, 5, 1, 49, 50, 52 };
					std::vector<int> PelvistoRightFoot = { 1, 55, 56, 58 };
					std::vector<std::vector<int>> SkeletonVec{ LeftArmtoRightArm, HeadtoLeftFoot, PelvistoRightFoot };

					for (std::vector<int>& BodyPart : SkeletonVec)
					{
						SDK::FVector2D PrevScreenPosition;

						for (size_t i = 0; i < BodyPart.size(); i++)
						{
							int BoneId = BodyPart.at(i);

							SDK::FVector2D BoneScreenPosition = BoneToScreenLocation(PlayerController, PlayerCharacter, BoneId);

							if (i != 0)
								ImGui::GetForegroundDrawList()->AddLine({ PrevScreenPosition.X, PrevScreenPosition.Y }, { BoneScreenPosition.X, BoneScreenPosition.Y }, Color);

							PrevScreenPosition = BoneScreenPosition;
						}
					}
					ImGui::GetForegroundDrawList()->AddCircle({ Head.X, Head.Y }, 2.f, Color);
				}

				if (c_menu::u_vars::ShowName)
					ImGui::GetForegroundDrawList()->AddText({ Feet.X, Feet.Y }, Color, Name.c_str());

				if (PlayerCharacter->TeamNum == SelfPlayerCharacter->TeamNum)
					continue;

				if (c_menu::u_vars::SilentAim)
				{
					if (!c_menu::u_vars::VisCheck || (c_menu::u_vars::VisCheck && EntityVisible))
					{
						float Distance = Distance2D(ScreenCenter, Head);
						if (Distance < ClosestDistance && Distance < c_menu::u_vars::FovRadius)
						{
							static int boneId = 48;

							switch (c_menu::u_vars::AimBone)
							{
							case 0:
								boneId = 48;
								break;
							case 1:
								boneId = 5;
								break;
							case 2:
								boneId = 58;
								break;
							default:
								break;
							}

							ClosestDistance = Distance;
							Target = PlayerCharacter;
							AimLocation = PlayerCharacter->Mesh->GetSocketLocation(PlayerCharacter->Mesh->GetBoneName(boneId));
						}
					}
				}
			}
			else if (c_menu::u_vars::SilentAim && Actor->IsA(SDK::AHDProj_Bullet::StaticClass()))
			{
				SDK::AHDProj_Bullet* Bullet = (SDK::AHDProj_Bullet*)Actor;
				if (!Bullet)
					continue;

				if (Bullet->InstigatingController == PlayerController)
				{
					if (AimLocation.X && AimLocation.Y && AimLocation.Z)
					{
						Bullet->K2_SetActorLocation(AimLocation, false, nullptr, true);
					}
				}
			}
		}
	}
};