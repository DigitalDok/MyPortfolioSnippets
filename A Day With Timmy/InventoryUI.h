// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "InventoryUI.generated.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Inventory Item Structure.																										  //
// Each Item is supposed to have a specific ID (Referencing), name, icon, and some text/voiceovers should the character interact with it. //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_USTRUCT_BODY()

	FInventoryItem()
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ItemImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ThingsToSay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<USoundWave*> SoundsToPlay;
	
};

/**
 * 
 */
UCLASS()
class POINTANDCLICK_API UInventoryUI : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FInventoryItem> ItemsOnInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<bool> ButtonsSelected;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString CurrentItemEquipped;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//							Main Functionality (Add | Remove | Combine Items)						       //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FString CombineTheseItems(FString Name_A, FString Name_B);

	void RemoveItemByName(FString Name);

	bool AddItemByName(FString Name);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//												Generic UI											       //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "Inventory Functions")
	void ButtonClick(int32 ButtonID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Functions")
		void RMB();
	//The only variable that has to do with game flow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool GameWon;
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//							Helper Functions (Mainly Getters for Inventory) 						       //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int32 GetIndexOfFirstEmptySpot();

	int32 GetIndexOfItemByName(FString Name);

	int32 GetIndexOfItemInArrayByName(FString Name);

	FString GetItemNameByIndex(int32 Index);

	int32 GetTheOtherClickedButtonIfItExists(int32 IgnoredButtonID);

	void ShiftItems(int32 DeletedIndex);

	bool DoIHaveThisItem(FString Name);
};
