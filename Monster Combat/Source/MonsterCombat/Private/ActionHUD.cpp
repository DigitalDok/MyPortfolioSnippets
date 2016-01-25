// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCombat.h"
#include "ActionHUD.h"
#include "Widget.h"
#include "CombatController.h"


void UActionHUD::Attack()
{
	DepthLevel = 1;
	CurrentMenu = EActiveMenus::SelectHostileTargetMenu;

	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_B);
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Attack;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentAttackPhase = EActionPhasesAttack::TargetSelection;
}

void UActionHUD::Item()
{
	DepthLevel = 1;
	CurrentTarget = "";
	CurrentMenu = EActiveMenus::ItemsMenu;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Item;
	PopulateInventory();
}

void UActionHUD::Ability()
{
	DepthLevel = 1;
	CurrentMenu = EActiveMenus::AbilitiesMenu;
	CurrentTarget = "";
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Ability;
	PopulateSpellbook();
}

void UActionHUD::ItemSelected(FItem AssociatedItem)
{
	LatestItem = AssociatedItem;
	CurrentTarget = LatestItem.Target;
	
	bIsCustomTooltip = (CurrentTarget == "All") ? true : false;
	BottomTooltip = (CurrentTarget == "All") ? "Click Anywhere to confirm!" : "";
	
	DepthLevel = 2;
	CurrentMenu = EActiveMenus::SelectFriendlyTargetMenu;
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_A);
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentItemPhase = EActionPhasesItem::TargetSelection_Friendly;
}

void UActionHUD::PopulateInventory()
{
	GroupAInventory = Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->GroupAInventory;
	GroupBInventory = Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->GroupBInventory;
	ACombatController* Controller = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());

	ItemBox->ClearChildren();
	QuantityBox->ClearChildren();
	TargetBox->ClearChildren();

	FInventory OurInventory;
	if (Controller->IndexOfCurrentPlayingMonster > 2)
	{
		OurInventory = GroupBInventory;
	}
	else
	{
		OurInventory = GroupAInventory;
	}

	bIsHoveringFromButtons.Empty();
	bIsHoveringFromButtons.AddDefaulted(OurInventory.Items.Num());
	for (size_t i = 0; i < OurInventory.Items.Num(); i++)
	{
		UItemPanel_Button* Button = CreateWidget<UItemPanel_Button>(GetWorld(), MyInventoryButtonTemplate);
		UItemPanel_Targets* Target = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryTargetsTemplate);
		UItemPanel_Targets* Quantity = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryQuantitiesTemplate);

		Button->AddToViewport();
		Target->AddToViewport();
		Quantity->AddToViewport();

		Button->MyText = OurInventory.Items[i].Name;
		Button->MyTooltip = OurInventory.Items[i].Tooltip;
		Button->MyID = i;
		Button->MyItemPower = OurInventory.Items[i];

		Quantity->MyText = FString::FromInt(OurInventory.Items[i].Quantity);
		Target->MyText = OurInventory.Items[i].Target;

		Quantity->AssociatedButton = Button;
		Target->AssociatedButton = Button;

		ItemBox->AddChild(Button);
		QuantityBox->AddChild(Quantity);
		TargetBox->AddChild(Target);
	}

}

void UActionHUD::PopulateSpellbook()
{
	
	ACombatController* Controller = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());

	AbilityBox->ClearChildren();
	TargetAbilityBox->ClearChildren();
	MPCostBox->ClearChildren();
	ElementalBox->ClearChildren();
	PowerBox->ClearChildren();

	FSpellBook OurSpellbook;
	
	for (size_t i = 0; i < Controller->MonsterStats.Num(); i++)
	{
		if (Controller->TurnOrder[Controller->IndexOfCurrentPlayingMonster]->MonsterType == Controller->MonsterStats[i].MonsterType)
		{
			OurSpellbook = Controller->MonsterStats[i].MonsterSpellbook;
		}
	}

	bIsHoveringFromButtons.Empty();
	bIsHoveringFromButtons.AddDefaulted(OurSpellbook.Abilities.Num());
	for (size_t i = 0; i < OurSpellbook.Abilities.Num(); i++)
	{
		UItemPanel_Button* Button = CreateWidget<UItemPanel_Button>(GetWorld(), MyInventoryButtonTemplate);
		UItemPanel_Targets* Target = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryTargetsTemplate);
		UItemPanel_Targets* Mana = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryQuantitiesTemplate);
		UItemPanel_Targets* Elemental = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryQuantitiesTemplate);
		UItemPanel_Targets* PowerTxt = CreateWidget<UItemPanel_Targets>(GetWorld(), MyInventoryQuantitiesTemplate);

		Button->AddToViewport();
		Target->AddToViewport();
		Mana->AddToViewport();
		Elemental->AddToViewport();
		PowerTxt->AddToViewport();

		Button->MyText = OurSpellbook.Abilities[i].Name;
		Button->MyTooltip = OurSpellbook.Abilities[i].Tooltip;
		Button->MyID = i;
		Button->MySpell = OurSpellbook.Abilities[i];

		Mana->MyText = FString::FromInt(OurSpellbook.Abilities[i].ManaCost);
		Target->MyText = OurSpellbook.Abilities[i].Target;

		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EElementalPower"), true);
		FString String = EnumPtr->GetEnumName(OurSpellbook.Abilities[i].ElementalPower); 

		Elemental->MyText = String;
		PowerTxt->MyText = FString::FromInt(OurSpellbook.Abilities[i].BaseSpellPowerHealth);

		Mana->AssociatedButton = Button;
		Target->AssociatedButton = Button;
		Elemental->AssociatedButton = Button;
		PowerTxt->AssociatedButton = Button;

		AbilityBox->AddChild(Button);
		TargetAbilityBox->AddChild(Target);
		MPCostBox->AddChild(Mana);
		ElementalBox->AddChild(Elemental);
		PowerBox->AddChild(PowerTxt);
	}

}

void UActionHUD::AbilitySelected(FAbility AssociatedAbility)
{
	LatestAbility = AssociatedAbility;
	CurrentTarget = LatestAbility.Target;

	bIsCustomTooltip = (CurrentTarget == "All Enemies" || CurrentTarget == "All Allies" || CurrentTarget == "Self") ? true : false;
	BottomTooltip = (CurrentTarget == "All Enemies" || CurrentTarget == "All Allies" || CurrentTarget == "Self") ? "Click Anywhere to confirm!" : "";

	DepthLevel = 2;

	if (CurrentTarget == "All Enemies" || CurrentTarget == "Single Enemy")
	{
		CurrentMenu = EActiveMenus::SelectHostileTargetMenu;
		Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_B);
	}
	else if (CurrentTarget == "All Allies" || CurrentTarget == "Single Ally" || CurrentTarget == "Self")
	{
		CurrentMenu = EActiveMenus::SelectFriendlyTargetMenu;
		Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::Camera_Party_A);
	}
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentAbilityPhase = EActionPhasesAbility::TargetSelectionAbility;
}

const FString UActionHUD::GetTextFromActor(int32 ID)
{
	ACombatController* Controller = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());

	for (size_t i = 0; i < Controller->TurnOrder.Num(); i++)
	{
		if (Controller->TurnOrder[i]->MonsterID == ID)
		{
			return Controller->TurnOrder[i]->StatusAbnormalitiesTextSweep;
		}
	}

	return "";
}

void UActionHUD::Defend()
{
	CurrentMenu = EActiveMenus::None;
	DepthLevel = 0;

	ACombatController* Controller = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());

	Controller->CurrentPhase = EActionPhasesType::Defend;
	Controller->CurrentDefendPhase = EActionPhasesDefend::DefendingParticle;

	bIsFadingOut = false;

	Controller->SetBottomTooltip(true, Controller->TurnOrder[Controller->IndexOfCurrentPlayingMonster]->MonsterName + " Defends! (+50% Defense until next turn!)");

	CentralTooltip = "Defend";
}

void UActionHUD::Cancel()
{
	if (DepthLevel == 2)
	{
		if (CurrentMenu == EActiveMenus::SelectFriendlyTargetMenu)
		{
			if (Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase == EActionPhasesType::Item)
			{
				CurrentMenu = EActiveMenus::ItemsMenu;
				DepthLevel = 1;
				CurrentTarget = "";
				bIsCustomTooltip = false;
				BottomTooltip = "";
				bIsHoveringFromButtons.Empty();

				Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			}
			if (Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase == EActionPhasesType::Ability)
			{
				CurrentMenu = EActiveMenus::AbilitiesMenu;
				DepthLevel = 1;
				CurrentTarget = "";
				bIsCustomTooltip = false;
				BottomTooltip = "";
				bIsHoveringFromButtons.Empty();

				Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			}
		}
		else if (CurrentMenu == EActiveMenus::SelectHostileTargetMenu)
		{
			CurrentMenu = EActiveMenus::AbilitiesMenu;
			DepthLevel = 1;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			
			CurrentTarget = "";
			bIsCustomTooltip = false;
			BottomTooltip = "";
			bIsHoveringFromButtons.Empty();
		}
	}
	else if (DepthLevel == 1)
	{
		if (CurrentMenu == EActiveMenus::SelectHostileTargetMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Undefined;
		}
		else if (CurrentMenu == EActiveMenus::AbilitiesMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Undefined;

			AbilityBox->ClearChildren();
			TargetAbilityBox->ClearChildren();
			MPCostBox->ClearChildren();
			ElementalBox->ClearChildren();
			PowerBox->ClearChildren();
		}
		else if (CurrentMenu == EActiveMenus::ItemsMenu)
		{
			CurrentMenu = EActiveMenus::MainMenu;
			DepthLevel = 0;
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->ChangeCamera(ECameras::NoneCam);
			Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Undefined;
		}
	}
}

void UActionHUD::Confirm()
{
	CurrentMenu = EActiveMenus::None;
	DepthLevel = 0;
}

