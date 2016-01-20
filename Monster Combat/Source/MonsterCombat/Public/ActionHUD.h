// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ItemPanel_Button.h"
#include "ItemPanel_Targets.h"

#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Runtime/UMG/Public/Components/VerticalBox.h"

#include "ActionHUD.generated.h"

UENUM(BlueprintType)
enum EActiveMenus
{
	MainMenu,
	SelectHostileTargetMenu,
	SelectFriendlyTargetMenu,
	AbilitiesMenu,
	ItemsMenu,
	None
};





/**
 * 
 */
UCLASS()
class MONSTERCOMBAT_API UActionHUD : public UUserWidget
{
	GENERATED_BODY()
public:

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General UI Properties")
		TArray<UTexture2D*> Portraits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General UI Properties")
		FString BottomTooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General UI Properties")
		TEnumAsByte<EActiveMenus> CurrentMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General UI Properties")
		int32 DepthLevel = 0;

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<FString> MonsterNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<int32> MonsterHPs_Current;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<int32> MonsterHPs_Max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<int32> MonsterMPs_Current;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<int32> MonsterMPs_Max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<FString> SelectionTooltips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		TArray<UTexture2D*> MonsterPortraits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		int32 CurrentMonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		FString CentralTooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		float OpacityOfCentralTooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		bool bIsFadingOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat UI Properties")
		float FadeSpeed;

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<int32> ItemQuantities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<FString> ItemNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<FString> ItemTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<FString> ItemTooltips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
	FInventory GroupAInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		FInventory GroupBInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TSubclassOf<UItemPanel_Button> MyInventoryButtonTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TSubclassOf<UItemPanel_Targets> MyInventoryTargetsTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TSubclassOf<UItemPanel_Targets> MyInventoryQuantitiesTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		UVerticalBox* ItemBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		UVerticalBox* TargetBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		UVerticalBox* QuantityBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<bool> bIsHoveringFromButtons;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		FItem LatestItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		FString CurrentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		bool bIsCustomTooltip;

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<int32> AbilityMPCosts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityTooltips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		UVerticalBox* AbilityBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		UVerticalBox* TargetAbilityBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		UVerticalBox* MPCostBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		UVerticalBox* ElementalBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		FAbility LatestAbility;

	// *****************************

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Attack();
	
	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Ability();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void AbilitySelected(FAbility AssociatedAbility);

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Item();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void ItemSelected(FItem AssociatedItem);

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void PopulateInventory();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void PopulateSpellbook();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Defend();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Cancel();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Confirm();

	// ******************************


	
};
