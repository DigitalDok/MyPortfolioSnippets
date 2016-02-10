// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GameHUD_UMG.h"
#include "InventoryHUD_UMG.h"
#include "CraftingHUD_UMG.h"
#include "TimeOverlayHUD_UMG.h"
#include "CavemanController.generated.h"

/**
 * 
 */


UCLASS()
class CAVEMANTESTFIELD_API ACavemanController : public APlayerController
{
	GENERATED_BODY()

private:
	void DetermineNewTemperatureLimits(float& UpperLimit, float& FloorLimit, float CurrentTemperature, float MaxTemp, float MinTemp);
	
public:
	// -- Our own definition of the BeginPlay Function -- //
	virtual void BeginPlay() override;

#pragma region Widget Templates
		// -- We are going to need access to the User Widget so as to drag and drop it at the Editor -- //
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> UIMan;
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> UI_Inv;
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> UI_Craft;
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> UI_Time;

#pragma endregion

#pragma region Widget Instances
	// -- If we are going to communicate with the Extended UserWidget, we are going to need a property of that type -- //
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		UGameHUD_UMG* UberWidget;
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		UInventoryHUD_UMG* InventoryWidget;
	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		UCraftingHUD_UMG* CraftWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Interface Stuff")
		UTimeOverlayHUD_UMG* TimeWidget;

#pragma endregion

#pragma region Open any Widget
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void OpenCrafts(bool WillOpenMainCraft, bool IsMainFire);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CloseCrafts();

	void OpenInventory();

	void CloseInventory();

#pragma endregion

#pragma region Inventory System

	int CurrentInventorySlot;

	void AddItemToInventory(FString ItemToAdd);

	void RemoveItemFromInventory(int32 SlotToRemove);
	void RemoveItemFromLootTable(int32 SlotToRemove);

	int32 GetFirstAvailableEmptySlot();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exposed Vars")
	bool bIsInventoryOpen;

	// Reference to our inventory in FString format.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FString> MyInventory;

	// Reference to the last loot table we interacted with, in a FString format.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FString> LastLootTable;

	// The Item Names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FString> ItemNames;

	// Sprites to display when these items exist in our inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FSlateBrush> ItemSprites;

	// Descriptions to be shown when we select the item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<FText> ItemDescs; 

	// boolean list that displays which item is usable and which is not. (Used to disable "USE" and "ASSIGN" Buttons).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
		TArray<bool> ItemUsableProps; 

#pragma endregion

#pragma region Inventory Helper Properties

	bool DoWeHaveThisItem(FString ItemName);

	void UseItem(FString ItemName);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RemoveItemByName(FString Name);

	int32 FindFirstOccurenceOfItemInInventory(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory Items")
	int32 GetNumberOfItemsInInventory(FString Name);

	bool IsTheItemSlotted(FString Name);
	bool IsTheItemSlotted(int32 SlotID);

	FString GetNameBySlot(int32 ItemSlot);

	int32 GetTheSlotThatTheItemIsAssignedTo(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory Items")
		int32 GetTheSlotThatTheItemIsAssignedTo(int32 SlotID);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Items")
	bool bIsAssignmentMenuOpen;
	
#pragma endregion

#pragma region Looting system 

	bool bIsLootWindowOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot Table")
		TArray<FString> LootTable;

	void PopulateLootTable(TArray<FString> DroppedItems);

	AActor* LastDiedEnemyThatWeLooted;

#pragma endregion

#pragma region Crafting Helpers

	void CraftsGoUp();

	void CraftsGoDown();

	void CraftsConfirmCraft();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
		FString SelectedItemForCrafting;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		bool SelectAnItemForCrafting(FString Item, bool IsCooking);

	bool DoWeKnowThisRecipe(FString Item, bool IsCooking);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		FName GetRowNameOfItem(FString Item);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		void DoStuffWithSelectedItem(FString ItemName, UTexture2D* ItemImg,
		FString Mat1, int32 Mat1Count, FString Mat2, int32 Mat2Count, FString Mat3, int32 Mat3Count,
		FString Mat4, int32 Mat4Count, FString Mat5, int32 Mat5Count, FString Mat6, int32 Mat6Count);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		void DoStuffWithSelectedItem2(FCraftingItemsStruct LeStruct);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		void GetStructBasedOnIndex(int32 index, bool IsCooking);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UDataTable* Data;

#pragma endregion

#pragma region Crafting System

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool PendingChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CItemName; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	UTexture2D* CItemImg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat1; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat1Count; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat2; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat2Count; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat3; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat3Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat4Count; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat5; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat5Count; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FString CMat6; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 CMat6Count;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		FSlateBrush GetImageByName(FString Item);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		void Craft(FString ItemToCraft,
		FString Mat1, int32 Mat1Count, FString Mat2, int32 Mat2Count, FString Mat3, int32 Mat3Count,
		FString Mat4, int32 Mat4Count, FString Mat5, int32 Mat5Count, FString Mat6, int32 Mat6Count);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
		int32 CurrentlySelectedCraftableItemSlot;

	UFUNCTION(BlueprintCallable, Category = "Dok Custom C++ ")
		void HideMats();

#pragma endregion

#pragma region Fire and Cooking Systems
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UDataTable* CookData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		bool bIsCookingFireEnabled;

	UFUNCTION(BlueprintCallable, Category = "Dok Custom C++ ")
		void ShowStartingFires();

	UFUNCTION(BlueprintCallable, Category = "Dok Custom C++ ")
		void HideStartingFires();

	UFUNCTION(BlueprintCallable, Category = "Dok Custom C++ ")
		void ShowMainMenuFires();

	UFUNCTION(BlueprintCallable, Category = "Dok Custom C++ ")
		void HideMainMenuFires();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
		bool DoWeHaveTheMatsForStartingFire();

#pragma endregion

#pragma region Temperature Mechanics

	/*Min temperature between 06:00-18:00*/
	UPROPERTY(EditAnywhere, Category = "Temperature")
	float MinDayTemperature = 20;

	/*Max temperature between 06:00-18:00*/
	UPROPERTY(EditAnywhere, Category = "Temperature")
	float MaxDayTemperature = 30;

	/*Min temperature between 18:00-05:00*/
	UPROPERTY(EditAnywhere, Category = "Temperature")
	float MinNightTemperature = 5;

	/*Max temperature between 18:00-05:00*/
	UPROPERTY(EditAnywhere, Category = "Temperature")
	float MaxNightTemperature = 15;

	/*Clamp value for temperature (ie a value of 2 means that the temp will rise or fall max 2 degrees*/
	UPROPERTY(EditDefaultsOnly, Category = "Temperature")
	float ClampValue=2;

	/*Decides the temperature based on max and mins temperatures given*/
	UFUNCTION(BlueprintCallable, Category = "Temperature")
	void SetNewTemperature();

#pragma endregion

};




