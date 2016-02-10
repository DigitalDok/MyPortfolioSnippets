// Fill out your copyright notice in the Description page of Project Settings.

#include "CavemanTestField.h"
#include "CavemanController.h"
#include "EnemyStatsComponent.h"
#include "MyCharacter_FirstTry.h"
#include "InventoryHUD_UMG.h"


#pragma region Inventory Helpers

int32 UInventoryHUD_UMG::UpdateInventory(int32 ID, FString Name)
{
	//ItemSlot_Update = ID;
	ItemName_Update = Name;

	return GetSpriteByName(Name);
}

int32 UInventoryHUD_UMG::GetSpriteByName(FString Name)
{
	int32 Index = -1;

	if (ItemNamesUMG.Contains(Name))
	{
		Index = ItemNamesUMG.IndexOfByKey(Name);
	}

	return Index;
}

FString UInventoryHUD_UMG::GetNameBySlot(ACavemanController* Controller, int32 ItemSlot)
{
	FString Name = "";

	for (int32 i = 0; i < 40; i++)
	{
		if (i == ItemSlot)
		{
			return Controller->MyInventory[ItemSlot];
		}
	}

	return Name;
}

FString UInventoryHUD_UMG::GetNameBySlot_Loot(ACavemanController* Controller, int32 ItemSlot)
{
	FString Name = "";

	for (int32 i = 0; i < 40; i++)
	{
		if (i == ItemSlot)
		{
			return Controller->LastLootTable[ItemSlot];
		}
	}

	return Name;
}

#pragma endregion

#pragma region Inventory Functionality

void UInventoryHUD_UMG::RemoveItemFromInventory(ACavemanController* Controller, int32 SlotToRemove)
{
	Controller->RemoveItemFromInventory(SlotToRemove);
}

void UInventoryHUD_UMG::UseItemFromInventory(ACavemanController* Controller, int32 SlotToUse)
{
	Controller->UseItem(GetNameBySlot(Controller,CurrentlySelectedSlot));
}

#pragma endregion

// --- Single Function that Binds an Item to a specific Item Slot --- //
void UInventoryHUD_UMG::AssignInvSlotToActionSlot(ACavemanController* Controller, int32 InvSlot, int32 ActionSlot)
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(Controller->GetCharacter());

	if (LeCaveman)
	{
		LeCaveman->AssignNameToSlot(GetNameBySlot(Controller,InvSlot), ActionSlot);
	}
}

#pragma region Loot Table Functionality

void UInventoryHUD_UMG::LootEverything(ACavemanController* Controller)
{
	for (int32 i = 0; i < 16; i++)
	{
		if (Controller->GetFirstAvailableEmptySlot() != -1)
		{
			Controller->AddItemToInventory(GetNameBySlot_Loot(Controller, i));
			RemoveItemFromLootTable(Controller, i);
		}
		else
		{
			GLog->Log("Inventory is full");
			return;
		}
	}
	
	
	TArray<UEnemyStatsComponent*> CamComps;
	Controller->LastDiedEnemyThatWeLooted->GetComponents<UEnemyStatsComponent>(CamComps);
	for (int32 i = 0; i < CamComps.Num(); i++)
	{
		bool WillClose = true;

		for (int32 i = 0; i < Controller->LastLootTable.Num(); i++)
		{
			if (Controller->LastLootTable[i] != "")
			{
				WillClose = false;
			}
		}

		if (WillClose)
		{
			CamComps[0]->CloseLootWindow();

			CamComps[0]->LootTable.Empty();
			CamComps[0]->LootTable.AddZeroed(16);
		}
	}
	
	
}

void UInventoryHUD_UMG::LootSingleItem(ACavemanController* Controller, int32 InvSlot)
{
	Controller->AddItemToInventory(GetNameBySlot_Loot(Controller, InvSlot));
	RemoveItemFromLootTable(Controller, InvSlot);

	TArray<UEnemyStatsComponent*> CamComps;
	Controller->LastDiedEnemyThatWeLooted->GetComponents<UEnemyStatsComponent>(CamComps);
	for (int32 i = 0; i < CamComps.Num(); i++)
	{
		bool WillClose = true;

		for (int32 i = 0; i < Controller->LastLootTable.Num(); i++)
		{
			if (Controller->LastLootTable[i] != "")
			{
				WillClose = false;
			}
		}

		if (WillClose)
		{
			CamComps[0]->CloseLootWindow();
			CamComps[0]->LootTable.Empty();
			CamComps[0]->LootTable.AddZeroed(16);
		}
	}
}

void UInventoryHUD_UMG::RemoveItemFromLootTable(ACavemanController* Controller, int32 SlotToRemove)
{
	FString ItemName = Controller->LastLootTable[SlotToRemove];
	if (ItemName != "")
	{
		Controller->LastLootTable[SlotToRemove] = "";
		ItemName_Update = "";
		ItemSlot_Update = SlotToRemove;
		OnAwesomeness.Broadcast();
		OnInteractionDokFinished.Broadcast();

	}
}

#pragma endregion