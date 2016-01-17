// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
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
		int32 CurrentMonsterID;

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<int32> ItemQuantities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<FString> ItemNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item UI Properties")
		TArray<FString> ItemTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> ItemTooltips;

	// *****************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<int32> AbilityMPCosts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability UI Properties")
		TArray<FString> AbilityTooltips;

	// *****************************

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Attack();
	
	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Ability();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void AbilitySelected(int32 AbilitySelected, FString Target);

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Item();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void ItemSelected();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Defend();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Cancel();

	UFUNCTION(BlueprintCallable, Category = "UI Stuff")
		void Confirm();

	// ******************************


	
};
