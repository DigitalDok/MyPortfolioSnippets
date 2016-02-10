// Fill out your copyright notice in the Description page of Project Settings.

#include "CavemanTestField.h"
#include "CavemanController.h"
#include "MyCharacter_FirstTry.h"


void ACavemanController::BeginPlay()
{

	Super::BeginPlay();

	if (UIMan) // -- Check if the Asset is assigned in the blueprint. -- //
	{
		// -- Create the widget and store it. This is exclusive to PlayerController classes -- //
		UberWidget = Cast<UGameHUD_UMG>(CreateWidget<UUserWidget>(this, UIMan));
		InventoryWidget = Cast<UInventoryHUD_UMG>(CreateWidget<UUserWidget>(this, UI_Inv));
		InventoryWidget->AddToViewport(1);
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		MyInventory.AddZeroed(40);
		
		CraftWidget = Cast<UCraftingHUD_UMG>(CreateWidget<UUserWidget>(this, UI_Craft));
		CraftWidget->AddToViewport(3);
		CraftWidget->SetVisibility(ESlateVisibility::Collapsed);
		

		TimeWidget = Cast<UTimeOverlayHUD_UMG>(CreateWidget<UUserWidget>(this, UI_Time));
		TimeWidget->AddToViewport(10);
		TimeWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		TimeWidget->Hour = 0;
		TimeWidget->Minute = 0;
		TimeWidget->Second = 0;
		TimeWidget->Day = 1;

		LastLootTable.AddZeroed(16);

		CurrentInventorySlot = 0;
		//InventoryWidget->TheController = this;


		if (UberWidget)
		{
			// -- Add the Widget to the Viewport and render it! -- //
			UberWidget->AddToViewport();
		}

		// -- I will need this true so I can click the button. Else, my cursor is hidden and I can't do anything. -- //
		bShowMouseCursor = false;
	}
}

#pragma region Inventory Functions

void ACavemanController::OpenInventory()
{
	bIsInventoryOpen = true;
	InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	bShowMouseCursor = true;
	InventoryWidget->OnInteractionDokFinished.Broadcast();
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(InventoryWidget->GetCachedWidget());
	SetInputMode(Mode);
}

void ACavemanController::CloseInventory()
{
	bIsInventoryOpen = false;
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	bShowMouseCursor = false;
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
}

#pragma endregion

#pragma region Crafting Functions

void ACavemanController::OpenCrafts(bool WillOpenMainCraft, bool IsMainFire)
{
	
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	LeCaveman->bIsCraftingWindowOpen = true;
	CraftWidget->SetVisibility(ESlateVisibility::Visible);


	CraftWidget->OnInventoryOpened.Broadcast();

	bShowMouseCursor = true;
	CraftWidget->OnInteractionDokFinished.Broadcast();
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(CraftWidget->GetCachedWidget());
	SetInputMode(Mode);

	if (WillOpenMainCraft)
	{
		CraftWidget->ShowCraftingDel.Broadcast();
		CraftWidget->HideStartingFire.Broadcast();
		CraftWidget->HideMainFire.Broadcast();
	}
	else
	{
		CraftWidget->HideCraftingDel.Broadcast();

		if (IsMainFire)
		{
			CraftWidget->ShowMainFire.Broadcast();
			CraftWidget->HideStartingFire.Broadcast();
		}
		else
		{
			CraftWidget->HideMainFire.Broadcast();
			CraftWidget->ShowStartingFire.Broadcast();
		}
	}
}

void ACavemanController::CloseCrafts()
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());

	LeCaveman->bIsCraftingWindowOpen = false;
	LeCaveman->bIsFireMainMenuEnabled = false;
	LeCaveman->bIsFireStartingWindowEnabled = false;

	CraftWidget->SetVisibility(ESlateVisibility::Collapsed);
	bShowMouseCursor = false;
	FInputModeGameOnly Mode;
	SetInputMode(Mode);

	if (bIsCookingFireEnabled)
	{
		bIsCookingFireEnabled = false;
	}
}

void ACavemanController::CraftsGoUp()
{
	if (CurrentlySelectedCraftableItemSlot > 0)
	{
		CurrentlySelectedCraftableItemSlot--;
		CraftWidget->OnGamepadControllerUpDown.Broadcast();
	}
}

void ACavemanController::CraftsGoDown()
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	
	int32 Stock = 0;
	for (int32 i = 0; i < LeCaveman->RecipeList->Recipe_Knowledge.Num(); i++)
	{
		if (LeCaveman->RecipeList->Recipe_Knowledge[i] == true)
		{
			Stock++;
		}
	}

	GLog->Log("Stock equals " + FString::FromInt(Stock));
	
	if (CurrentlySelectedCraftableItemSlot < Stock-1)
	{
		CurrentlySelectedCraftableItemSlot++;
		CraftWidget->OnGamepadControllerUpDown.Broadcast();
	}

	
}

void ACavemanController::CraftsConfirmCraft()
{
	CraftWidget->OnCraftItem.Broadcast();
}

bool ACavemanController::SelectAnItemForCrafting(FString Item,bool IsCooking)
{
	SelectedItemForCrafting = Item;
	return DoWeKnowThisRecipe(SelectedItemForCrafting,IsCooking);
}

bool ACavemanController::DoWeKnowThisRecipe(FString Item, bool isCooking)
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	int32 Index = -1;

	if (isCooking)
		Index = LeCaveman->RecipeList->Cook_Items_Recipes.IndexOfByKey(Item);
	else
		Index = LeCaveman->RecipeList->Items_Recipes.IndexOfByKey(Item);


	return LeCaveman->RecipeList->Recipe_Knowledge[Index];
}

void ACavemanController::DoStuffWithSelectedItem(FString ItemName, UTexture2D* ItemImg,
	FString Mat1, int32 Mat1Count, FString Mat2, int32 Mat2Count, FString Mat3, int32 Mat3Count,
	FString Mat4, int32 Mat4Count, FString Mat5, int32 Mat5Count, FString Mat6, int32 Mat6Count)
{
	CItemName = ItemName;
	CItemImg = ItemImg;

	CMat1 = Mat1;
	CMat2 = Mat2;
	CMat3 = Mat3;
	CMat4 = Mat4;
	CMat5 = Mat5;
	CMat6 = Mat6;

	CMat1Count = Mat1Count;
	CMat2Count = Mat2Count;
	CMat3Count = Mat3Count;
	CMat4Count = Mat4Count;
	CMat5Count = Mat5Count;
	CMat6Count = Mat6Count;

	PendingChange = true;
}

void ACavemanController::DoStuffWithSelectedItem2(FCraftingItemsStruct LeStruct)
{

	CItemName = LeStruct.ItemName;
	CItemImg = LeStruct.ItemImg;

	CMat1 = LeStruct.Mat1;
	CMat2 = LeStruct.Mat2;
	CMat3 = LeStruct.Mat3;
	CMat4 = LeStruct.Mat4;
	CMat5 = LeStruct.Mat5;
	CMat6 = LeStruct.Mat6;

	CMat1Count = LeStruct.Mat1_Count;
	CMat2Count = LeStruct.Mat2_Count;
	CMat3Count = LeStruct.Mat3_Count;
	CMat4Count = LeStruct.Mat4_Count;
	CMat5Count = LeStruct.Mat5_Count;
	CMat6Count = LeStruct.Mat6_Count;

	PendingChange = true;
}

void  ACavemanController::Craft(FString ItemToCraft,
	FString Mat1, int32 Mat1Count, FString Mat2, int32 Mat2Count, FString Mat3, int32 Mat3Count,
	FString Mat4, int32 Mat4Count, FString Mat5, int32 Mat5Count, FString Mat6, int32 Mat6Count)
{
	if (Mat1Count > 0)
	{
		for (int32 i = 0; i < Mat1Count; i++)
		{
			RemoveItemByName(Mat1);
		}
	}
	if (Mat2Count > 0)
	{
		for (int32 i = 0; i < Mat2Count; i++)
		{
			RemoveItemByName(Mat2);
		}
	}
	if (Mat3Count > 0)
	{
		for (int32 i = 0; i < Mat3Count; i++)
		{
			RemoveItemByName(Mat3);
		}
	}
	if (Mat4Count > 0)
	{
		for (int32 i = 0; i < Mat4Count; i++)
		{
			RemoveItemByName(Mat4);
		}
	}
	if (Mat5Count > 0)
	{
		for (int32 i = 0; i < Mat5Count; i++)
		{
			RemoveItemByName(Mat5);
		}
	}
	if (Mat6Count > 0)
	{
		for (int32 i = 0; i < Mat6Count; i++)
		{
			RemoveItemByName(Mat6);
		}
	}

	AddItemToInventory(ItemToCraft);
	Cast<AMyCharacter_FirstTry>(GetCharacter())->SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_ItemCrafted);
}

void ACavemanController::HideMats()
{
	CraftWidget->Hide();
}

#pragma endregion

#pragma region Inventory Management Functions


FName ACavemanController::GetRowNameOfItem(FString Item)
{
	return FName("");
}

void ACavemanController::AddItemToInventory(FString LeItem)
{
	if (GetFirstAvailableEmptySlot() != -1)
	{
		int32 IndexToAdd = GetFirstAvailableEmptySlot();
		MyInventory[IndexToAdd] = LeItem;

		InventoryWidget->ItemName_Update = LeItem;
		InventoryWidget->ItemSlot_Update = IndexToAdd;

		InventoryWidget->OnAwesomeness.Broadcast();

	}
	else
	{
		GLog->Log("Inventory is full!");
	}
}

void ACavemanController::RemoveItemFromInventory(int32 SlotToRemove)
{
	FString ItemName = MyInventory[SlotToRemove];
	if (ItemName != "")
	{
		if (IsTheItemSlotted(ItemName))
		{
			if (GetNumberOfItemsInInventory(ItemName) > 1)
			{
				MyInventory[SlotToRemove] = "";
				InventoryWidget->ItemName_Update = "";
				InventoryWidget->ItemSlot_Update = SlotToRemove;
				InventoryWidget->OnAwesomeness.Broadcast();
				InventoryWidget->OnInteractionDokFinished.Broadcast();
			}
			else if (GetNumberOfItemsInInventory(ItemName) == 1)
			{
				int32 Slot = GetTheSlotThatTheItemIsAssignedTo(ItemName);
				AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());

				LeCaveman->AssignNameToSlot("", Slot);
				
				MyInventory[SlotToRemove] = "";
				InventoryWidget->ItemName_Update = "";
				InventoryWidget->ItemSlot_Update = SlotToRemove;
				InventoryWidget->OnAwesomeness.Broadcast();
				InventoryWidget->OnInteractionDokFinished.Broadcast();
			}
		}
		else
		{
			MyInventory[SlotToRemove] = "";
			InventoryWidget->ItemName_Update = "";
			InventoryWidget->ItemSlot_Update = SlotToRemove;
			InventoryWidget->OnAwesomeness.Broadcast();
			InventoryWidget->OnInteractionDokFinished.Broadcast();
		}
	}

	
}


int32 ACavemanController::GetFirstAvailableEmptySlot()
{
	int32 Index = -1;

	for (int32 i  = 0;  i  < 40; i ++)
	{
		if (MyInventory[i] == "")
		{
			Index = i;
			break;
		}
	}

	return Index;
}

void ACavemanController::UseItem(FString ItemName)
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	LeCaveman->UseItem(ItemName);
}

void ACavemanController::RemoveItemByName(FString ItemName)
{
	int32 IndexToRemove = FindFirstOccurenceOfItemInInventory(ItemName);
	if ( IndexToRemove != -1)
	{
		MyInventory[IndexToRemove] = "";
		InventoryWidget->ItemName_Update = "";
		InventoryWidget->ItemSlot_Update = IndexToRemove;
		InventoryWidget->OnAwesomeness.Broadcast();
		InventoryWidget->OnInteractionDokFinished.Broadcast();
	}
}

int32 ACavemanController::FindFirstOccurenceOfItemInInventory(FString ItemName)
{
	int32 Index = -1;

	for (int32 i = 0; i < 40; i++)
	{
		if (MyInventory[i] == ItemName)
		{
			Index =  i;
			break;
		}
	}

	return Index;
}

int32 ACavemanController::GetNumberOfItemsInInventory(FString ItemName)
{
	int32 Stock = 0;
	for (int32 i = 0; i < 40; i++)
	{
		if (MyInventory[i] == ItemName)
		{
			Stock++;
		}
	}
	return Stock;
}

bool ACavemanController::IsTheItemSlotted(FString ItemName)
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	if (LeCaveman->ItemInSlot1 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot2 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot3 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot4 == ItemName)
	{
		return true;
	}

	return false;
}

bool ACavemanController::IsTheItemSlotted(int32 ItemSlot)
{
	FString ItemName = GetNameBySlot(ItemSlot);
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	if (LeCaveman->ItemInSlot1 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot2 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot3 == ItemName)
	{
		return true;
	}
	if (LeCaveman->ItemInSlot4 == ItemName)
	{
		return true;
	}

	return false;

}

FString ACavemanController::GetNameBySlot(int32 ItemSlot)
{
	FString Name = "";

	for (int32 i = 0; i < 40; i++)
	{
		if (i == ItemSlot)
		{
			return MyInventory[ItemSlot];
		}
	}

	return Name;
}

void ACavemanController::GetStructBasedOnIndex(int32 index, bool IsCooking)
{

	FCraftingItemsStruct* Row = nullptr;
	if (IsCooking)
	{
		Row = CookData->FindRow<FCraftingItemsStruct>(FName(*FString::FromInt(index)), TEXT("ItemName"));
	}
	else
	{
		Row = Data->FindRow<FCraftingItemsStruct>(FName(*FString::FromInt(index)), TEXT("ItemName"));
	}

	if (Row)
	{
		CItemName = Row->ItemName;
		CItemImg = Row->ItemImg;

		CMat1 = Row->Mat1;
		CMat2 = Row->Mat2;
		CMat3 = Row->Mat3;
		CMat4 = Row->Mat4;
		CMat5 = Row->Mat5;
		CMat6 = Row->Mat6;

		CMat1Count = Row->Mat1_Count;
		CMat2Count = Row->Mat2_Count;
		CMat3Count = Row->Mat3_Count;
		CMat4Count = Row->Mat4_Count;
		CMat5Count = Row->Mat5_Count;
		CMat6Count = Row->Mat6_Count;

		PendingChange = true;
	}
}

FSlateBrush ACavemanController::GetImageByName(FString Item)
{
	int32 Index = -1;
	if (ItemNames.Contains(Item))
	{
		Index = ItemNames.IndexOfByKey(Item);
	}

	return ItemSprites[Index];
}

#pragma endregion

#pragma region Assignment-Specific Functions

int32 ACavemanController::GetTheSlotThatTheItemIsAssignedTo(FString ItemName)
{
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	if (LeCaveman->ItemInSlot1 == ItemName)
	{
		return 1;
	}
	if (LeCaveman->ItemInSlot2 == ItemName)
	{
		return 2;
	}
	if (LeCaveman->ItemInSlot3 == ItemName)
	{
		return 3;
	}
	if (LeCaveman->ItemInSlot4 == ItemName)
	{
		return 4;
	}

	return 0;
}

int32 ACavemanController::GetTheSlotThatTheItemIsAssignedTo(int32 SlotID)
{
	FString ItemName = GetNameBySlot(SlotID);
	AMyCharacter_FirstTry* LeCaveman = Cast<AMyCharacter_FirstTry>(GetCharacter());
	if (LeCaveman->ItemInSlot1 == ItemName)
	{
		return 1;
	}
	if (LeCaveman->ItemInSlot2 == ItemName)
	{
		return 2;
	}
	if (LeCaveman->ItemInSlot3 == ItemName)
	{
		return 3;
	}
	if (LeCaveman->ItemInSlot4 == ItemName)
	{
		return 4;
	}

	return 0;
}

#pragma endregion

#pragma region Fire Starting Functions

void ACavemanController::ShowStartingFires()
{
	CraftWidget->ShowStartingFire.Broadcast();

}

void ACavemanController::HideStartingFires()
{
	CraftWidget->HideStartingFire.Broadcast();
}

void ACavemanController::ShowMainMenuFires()
{
	CraftWidget->ShowMainFire.Broadcast();
}

void ACavemanController::HideMainMenuFires()
{
	CraftWidget->HideMainFire.Broadcast();
}

bool ACavemanController::DoWeHaveTheMatsForStartingFire()
{
	bool DoWe = false;
	if (FindFirstOccurenceOfItemInInventory("Flint") != -1)
	{
		if (FindFirstOccurenceOfItemInInventory("Tinder") != -1)
		{
			DoWe = true;
		}
	}
	return DoWe;
}

#pragma endregion

#pragma region Temperature Functions

void ACavemanController::SetNewTemperature()
{
	float NewTemperature;
	float CurrentTemperature = (Cast<AMyCharacter_FirstTry>(GetPawn()))->Temperature;
	int32 Hours = TimeWidget->Hour;

	//Fixed min and max for new temperature
	float UpperLimitOfNewTemperature = 0;
	float FloorLimitOfNewTemperature = 0;

	//Decide the upper and floor limits
	if ((Hours>=6) && (Hours<=18))
	{
		DetermineNewTemperatureLimits(UpperLimitOfNewTemperature, FloorLimitOfNewTemperature, CurrentTemperature, MaxDayTemperature, MinDayTemperature);
	}
	else
	{
		DetermineNewTemperatureLimits(UpperLimitOfNewTemperature, FloorLimitOfNewTemperature, CurrentTemperature, MaxNightTemperature, MinDayTemperature);
	}

	//Get random based on the upper and floor limits decided above
	NewTemperature = FMath::FRandRange(FloorLimitOfNewTemperature, UpperLimitOfNewTemperature);

	//Set the new temperature
	(Cast<AMyCharacter_FirstTry>(GetPawn()))->Temperature = NewTemperature;
}

void ACavemanController::DetermineNewTemperatureLimits(float& UpperLimit, float& FloorLimit,float CurrentTemperature,float MaxTemp,float MinTemp)
{
	//Determine the new upper limit
	if (CurrentTemperature<=MaxTemp-ClampValue)
	{
		UpperLimit = CurrentTemperature + ClampValue;
	}
	else
	{
		UpperLimit = MaxTemp;
	}
	
	//Determine the lower limit
	if (CurrentTemperature>=MinTemp+ClampValue)
	{
		FloorLimit = CurrentTemperature - ClampValue;
	}
	else
	{
		FloorLimit = MinTemp;
	}
}

#pragma endregion