
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//							         Inventory System - Kostas Dokos						               //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PointAndClick.h"
#include "InventoryUI.h"
#include "PointAndClickPlayerController.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//							Main Functionality (Add | Remove | Combine Items)						       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

FString UInventoryUI::CombineTheseItems(FString Name_A, FString Name_B)
{
	TArray<FString> Names;
	Names.Add(Name_A);
	Names.Add(Name_B);

	if (Names.Contains("Golden Key") && Names.Contains("Lockbox"))
	{
		RemoveItemByName(Name_A);
		RemoveItemByName(Name_B);

		APointAndClickPlayerController* TheController = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
		TheController->GlobalAudioComponent->Sound = TheController->CombinationSuccess;
		TheController->GlobalAudioComponent->Play();

		return "Big Ol' Wrench";
	}
	
	return "";
}

void UInventoryUI::RemoveItemByName(FString Name)
{
	int32 IndexToRemove = GetIndexOfItemInArrayByName(Name);
	if (IndexToRemove != -1)
	{
		ButtonsSelected[IndexToRemove] = false;
		CurrentItemEquipped = "";

		ItemsOnInventory[IndexToRemove].ItemID = -1;
		ItemsOnInventory[IndexToRemove].ItemName = "";
		ItemsOnInventory[IndexToRemove].ItemImage = nullptr;

		ShiftItems(IndexToRemove);
	}
}

bool UInventoryUI::AddItemByName(FString Name)
{
	APointAndClickPlayerController* Controller = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());

	for (size_t i = 0; i < Controller->ItemPool.Num(); i++)
	{
		if (Controller->ItemPool[i].ItemName == Name)
		{
			int32 IndexToAdd = GetIndexOfFirstEmptySpot();
			ItemsOnInventory[IndexToAdd] = Controller->ItemPool[i];
			GLog->Log("Added!");
			return true;
		}
	}


	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									User Interface Routed Button Clicks									   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UInventoryUI::ButtonClick(int32 ButtonID)
{
	// This Means that we selected the same item, so we toggle it off.
	if (ButtonsSelected[ButtonID] == true)
	{
		ButtonsSelected[ButtonID] = false;
		CurrentItemEquipped = "";
	}
	else
	{
		//If we have clicked on a new button, we want to check if we already have one selected:
		int32 OtherIndex = GetTheOtherClickedButtonIfItExists(ButtonID);
		
		//If we have, then we attempt to combine these items.
		if (OtherIndex != -1)
		{
			FString SuccessfulCombinedName = CombineTheseItems(GetItemNameByIndex(ButtonID), GetItemNameByIndex(OtherIndex));

			if (SuccessfulCombinedName != "")
			{
				//Successful combination! Remove both of these items, shut them down and add the new item.

				ButtonsSelected[ButtonID] = false;
				ButtonsSelected[OtherIndex] = false;

				APointAndClickPlayerController* Controller = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
				int32 Rand = FMath::RandRange(0, 2);
				Controller->EnqueueTextForTypewriter(Controller->DialogueSystem->DialogueBanches[1].ThingsToSay[Rand], Controller->DialogueSystem->DialogueBanches[1].SoundsToPlay[Rand]);


				CurrentItemEquipped = "";

				AddItemByName(SuccessfulCombinedName);
				
			}
			else
			{
				//If the combine was unsuccessful, say that it's impossible, and reset both buttons.
				
				APointAndClickPlayerController* Controller = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
				int32 Rand = FMath::RandRange(0, 6);
				Controller->EnqueueTextForTypewriter(Controller->DialogueSystem->DialogueBanches[0].ThingsToSay[Rand], Controller->DialogueSystem->DialogueBanches[0].SoundsToPlay[Rand]);

				ButtonsSelected[ButtonID] = false;
				ButtonsSelected[OtherIndex] = false;

				CurrentItemEquipped = "";
			}
		}
		else
		{
			ButtonsSelected[ButtonID] = true;
			CurrentItemEquipped = GetItemNameByIndex(ButtonID);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//										Helper Inventory Functionality									   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 UInventoryUI::GetIndexOfFirstEmptySpot()
{
	for (size_t i = 0; i < ItemsOnInventory.Num(); i++)
	{
		if (ItemsOnInventory[i].ItemID == -1)
		{
			return i;
		}
	}

	return -1;
}

int32 UInventoryUI::GetIndexOfItemByName(FString Name)
{
	for (size_t i = 0; i < ItemsOnInventory.Num(); i++)
	{
		if (ItemsOnInventory[i].ItemName == Name)
			return ItemsOnInventory[i].ItemID;
	}

	return -1;
}

int32 UInventoryUI::GetIndexOfItemInArrayByName(FString Name)
{
	for (size_t i = 0; i < ItemsOnInventory.Num(); i++)
	{
		if (ItemsOnInventory[i].ItemName == Name)
			return i;
	}

	return -1;
}

FString UInventoryUI::GetItemNameByIndex(int32 Index)
{
	return ItemsOnInventory[Index].ItemName;
}

int32 UInventoryUI::GetTheOtherClickedButtonIfItExists(int32 IgnoredButtonID)
{
	for (size_t i = 0; i < ButtonsSelected.Num(); i++)
	{
		if (ButtonsSelected[i])
		{
			if (i != IgnoredButtonID)
			{
				return i;
			}
		}
	}

	return -1;
}

bool UInventoryUI::DoIHaveThisItem(FString Name)
{
	for (size_t i = 0; i < ItemsOnInventory.Num(); i++)
	{
		if (ItemsOnInventory[i].ItemName == Name)
		{
			return true;
		}
	}

	return false;
}

void UInventoryUI::ShiftItems(int32 NewEmptySpot)
{
	for (size_t i = NewEmptySpot; i < 10; i++)
	{
		if (i<9)
			ItemsOnInventory[i] = ItemsOnInventory[i + 1];

		if (i == 8)
		{
			ItemsOnInventory[9].ItemID = -1;
			ItemsOnInventory[9].ItemName = "";
			ItemsOnInventory[9].ItemImage = nullptr;
		}
	}
}