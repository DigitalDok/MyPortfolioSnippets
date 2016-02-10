// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "InventoryHUD_UMG.generated.h"

/**
 * 
 */
UCLASS()
class CAVEMANTESTFIELD_API UInventoryHUD_UMG : public UUserWidget
{
	GENERATED_BODY()

public:
	
#pragma region Global Variables for Inventory Management (Descriptions, Names, Images)

	// A Global list containing all kinds of useful info for our inventory to be databound to the UMG Widgets.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FString> ItemNamesUMG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FSlateBrush> ItemSpritesUMG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FText> ItemDescsUMG;

#pragma endregion

#pragma region Delegates Used on UMG

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyBindableEvent);
	UPROPERTY(BlueprintAssignable)
		FMyBindableEvent OnAwesomeness;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyInteractionInventoryDelegate3);
	UPROPERTY(BlueprintAssignable)
		FMyInteractionInventoryDelegate3 OnInteractionDokFinished;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegate);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegate OnAssignmentViaGamepad;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegateCancel);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegateCancel OnAssignmentViaGamepadCancel;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegateSlot1);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegateSlot1 OnAssignmentViaGamepadSlot1;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegateSlot2);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegateSlot2 OnAssignmentViaGamepadSlot2;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegateSlot3);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegateSlot3 OnAssignmentViaGamepadSlot3;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAssignmentDelegateSlot4);
	UPROPERTY(BlueprintAssignable)
		FAssignmentDelegateSlot4 OnAssignmentViaGamepadSlot4;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLootSingleDelegate);
	UPROPERTY(BlueprintAssignable)
		FLootSingleDelegate LootSingleDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLootAllDelegate);
	UPROPERTY(BlueprintAssignable)
		FLootAllDelegate LootAllDelegate;


#pragma endregion

#pragma region Helper Properties

	// When this is true, the inventory widget is updated.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Updater")
		bool ShouldUpdateInventory;

	// The Slot and Name that needs updating.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Updater")
		int32 ItemSlot_Update;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Updater")
		FString ItemName_Update;

	// This is the currently selected slot in the inventory, used primarily for Glowing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		int32 CurrentlySelectedSlot;

#pragma endregion

#pragma region Inventory Functionality

	// Updates Inventory in UMG.
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		int32 UpdateInventory(int32 ItemSlot, FString ItemName);

	// Removes Item from Inventory (By Slot Name)
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void RemoveItemFromInventory(ACavemanController* Controller, int32 SlotToRemove);

	// Uses Item from Inventory (If it has a usage)
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void UseItemFromInventory(ACavemanController* Controller, int32 SlotToUse);

#pragma endregion

#pragma region Inventory Helper Functionality

	int32 GetSpriteByName(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		FString GetNameBySlot(ACavemanController* Controller, int32 ID_Slot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		FString GetNameBySlot_Loot(ACavemanController* Controller, int32 ID_Slot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void AssignInvSlotToActionSlot(ACavemanController* Controller, int32 InvSlot, int32 ActionSlot);

#pragma endregion

#pragma region Looting Functionality

	// Empties the Loot Window, Giving it all to the user.
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void LootEverything(ACavemanController* Controller);

	// Loots a single item, transfering it from the loot window to the inventory.
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void LootSingleItem(ACavemanController* Controller, int32 InvSlot);

	// Makes sure that the item is removed from the loot window.
	UFUNCTION(BlueprintCallable, Category = "Inventory Custom C++")
		void RemoveItemFromLootTable(ACavemanController* Controller, int32 SlotToRemove);

#pragma endregion
	


	
};
