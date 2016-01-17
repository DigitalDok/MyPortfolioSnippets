// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCombat.h"
#include "ActionHUD.h"
#include "CombatController.h"

void UActionHUD::Attack()
{
	DepthLevel = 1;
	CurrentMenu = EActiveMenus::SelectHostileTargetMenu;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_B);
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->bIsActorClickEnabled = true;
}

void UActionHUD::Item()
{
	DepthLevel = 1;
	CurrentMenu = EActiveMenus::ItemsMenu;
}

void UActionHUD::Ability()
{
	DepthLevel = 1;
	CurrentMenu = EActiveMenus::AbilitiesMenu;
}

void UActionHUD::ItemSelected()
{
	DepthLevel = 2;
	CurrentMenu = EActiveMenus::SelectFriendlyTargetMenu;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_A);
}

void UActionHUD::AbilitySelected(int32 AbilitySelected, FString Target)
{
	DepthLevel = 2;
	if (Target == "All")
	{
		CurrentMenu = EActiveMenus::None;
		DepthLevel = 0;
		return;
	}

	CurrentMenu = EActiveMenus::SelectHostileTargetMenu;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_B);
}

void UActionHUD::Defend()
{
	CurrentMenu = EActiveMenus::None;
	DepthLevel = 0;
}

void UActionHUD::Cancel()
{
	if (DepthLevel == 2)
	{
		if (CurrentMenu == EActiveMenus::SelectFriendlyTargetMenu)
		{
			CurrentMenu = EActiveMenus::ItemsMenu;
			DepthLevel = 1;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->bIsActorClickEnabled = false;
		}
		else if (CurrentMenu == EActiveMenus::SelectHostileTargetMenu)
		{
			CurrentMenu = EActiveMenus::AbilitiesMenu;
			DepthLevel = 1;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->bIsActorClickEnabled = false;
		}
	}
	else if (DepthLevel == 1)
	{
		if (CurrentMenu == EActiveMenus::SelectHostileTargetMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
		}
		else if (CurrentMenu == EActiveMenus::AbilitiesMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
		}
		else if (CurrentMenu == EActiveMenus::ItemsMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
		}
	}
}

void UActionHUD::Confirm()
{
	CurrentMenu = EActiveMenus::None;
	DepthLevel = 0;
}

