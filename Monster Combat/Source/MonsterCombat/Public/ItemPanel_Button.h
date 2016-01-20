// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ItemPanel_Button.generated.h"

UENUM(BlueprintType)
enum EStatusAbnormality
{
	NoAbnormality,
	Poison,
	Stun,
	Paralyze,
	Silence
};

UENUM(BlueprintType)
enum EElementalPower
{
	Normal,
	Fire,
	Frost,
	Nature,
	Thunder,
	Air,
	Darkness
};

USTRUCT(BlueprintType)
struct FAbility
{
	GENERATED_USTRUCT_BODY()
		FAbility()
	{

	}

	// --- Base Properties of the Ability --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		FString Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		FString Tooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 ManaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		USoundWave* SpellShout; // What will the monster yell should it cast the spell?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		USoundWave* SpellStartSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		USoundWave* SpellImpactSFX;

	// --- Whether this spell is a Damage or a Heal. --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		bool bIsDamagingForHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		bool bIsDamagingForMana;

	// --- The Power of this spell, adjusted by an extra Bonus --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerBonusHealth_Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerBonusHealth_Max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerBonusMana_Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 BaseSpellPowerBonusMana_Max;

	// --- If this is ZERO, then it won't cause any damage or heal --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 TurnsForDamage;

	// --- The Elemental Power that comes along with this ability, if any --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		TEnumAsByte<EElementalPower> ElementalPower;

	// --- Which Status Abnormality Can be applied? --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		TEnumAsByte<EStatusAbnormality> StatusAbnormality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 TurnsForStatusAbnormality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 ChanceForStatusAbnormality;

	// --- Alter a specific Stat for a set amount of Turns --- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 AlterationOfAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 AlterationOfDefence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 AlterationOfMagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 AlterationOfMagicDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 TurnsForAlteration;

	// --- Apply Extra Damage or Heal After X Turns ---- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 SpellDamageAfterXTurns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Ability Stuff")
		int32 TurnsForExtraDamage;

};

USTRUCT(BlueprintType)
struct FSpellBook
{
	GENERATED_USTRUCT_BODY()
		FSpellBook()
	{

	}

	UPROPERTY(EditAnywhere)
		TArray<FAbility> Abilities;
};

USTRUCT(BlueprintType)
struct FItem
{
	GENERATED_USTRUCT_BODY()
		FItem()
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="F Item Stuff")
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		FString Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		FString Tooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		int32 HP_Restored_Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		int32 HP_Restored_Percentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		int32 MP_Restored_Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		int32 MP_Restored_Percentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		bool Status_Recovery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "F Item Stuff")
		bool Resurrection;
};

USTRUCT(BlueprintType)
struct FInventory
{
	GENERATED_USTRUCT_BODY()
		FInventory()
	{

	}

	UPROPERTY(EditAnywhere)
		TArray<FItem> Items;
};

UCLASS()
class MONSTERCOMBAT_API UItemPanel_Button : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		FString MyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		FString MyTooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		int32 MyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		FItem MyItemPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		FAbility MySpell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		bool bIsHovered;
	
	
};
