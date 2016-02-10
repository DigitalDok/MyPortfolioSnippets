
////////////////////////////////////////////////////////////////////////////////////////////
//						Custom Monster Actor - By Kostas Dokos							  //
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/Character.h"
#include "MonsterAnimInstance.h"
#include "ActionHUD.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "CustomMonsterActor.generated.h"

/////////////////////////////////////////////////////////////////////////////////
// Monsters that are on the opposite side (Party B) are controlled by the AI.  //
// Monsters on the left side (Party A) are controlled by the Human Player.	   //
/////////////////////////////////////////////////////////////////////////////////

UENUM()
enum EPartyType
{
	PlayerControlled,
	AI_Controlled
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Each monster has a specific archetype. The archetype determines the AI Behavior.  										   //
//																															   //
// WARRIOR - Favors Melee Attacks, Focuses on Targets with Low HP or Low P.Def. Defends if HP is Low.						   //
// MAGE - Favors Offensive Magic, Focuses on Targets with Low HP or Low S.Def.												   //
// HEALER - Favors Support Magic and Item Usage and then on Offensive Magic. Same rules as Mage when no Support is required.   //
//																															   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM()
enum EAIArchetypes
{
	Warrior,
	Mage,
	Healer
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The buff Structure that any monster can have at any possible time.				  										   //
//																															   //
// Every Buff has a duration (lifetime), and special effects that are applied to the monster who currently has that buff.	   //
// Examples include Stat Alteration, DOTs and HOTs, Damage After X Turns and/or any Status Abnormality, such as Poison.		   //
//																															   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FBuff
{
	GENERATED_USTRUCT_BODY()

	FBuff()
	{

	}
	
public:
	UPROPERTY(EditAnywhere, Category = "Buff Code")
		FString OriginatingSpellName;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 AttackTempChange;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 DefenseTempChange;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 MagicDefenseTempChange;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 MagicAttackTempChange;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 DamagePerTurn;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 HealPerTurn;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 DamageOnEnd;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		TEnumAsByte<EStatusAbnormality> CurrentAbnormality;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 TurnsLifetime;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 BuffID;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// For tracking purposes, each monster is of a specific type. This determines the available information which is passed to the UI.	//
// This information includes Name, Portrait, HP/MP Values and Stats.																//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM()
enum EMonsterType
{
	Werewolf,
	Wyvern,
	Treant,
	Hydra,
	Troll,
	Mummy
};

UCLASS()
class MONSTERCOMBAT_API ACustomMonsterActor : public ACharacter
{
	GENERATED_BODY()
	
public:	
	
	ACustomMonsterActor();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	
	bool AllDone; //When this is true on all monsters, the Game Controller can begin Initialization.

	#pragma region Core Properties and Components

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EPartyType> Party;

	UMonsterAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
		UCameraComponent* MonsterCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
		UStaticMeshComponent* MonsterCameraRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
		UWidgetComponent* MyHealthWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
		UWidgetComponent* MyManaWidgetComponent;

	#pragma endregion

	//***************************************************************************

	#pragma region Main Monster Traits

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		FString MonsterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EAIArchetypes> AI_Archetype;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TArray<FAbility> Spells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EMonsterType> MonsterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float MaxMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float CurrentMana;

	void UpdateHealth(bool WillCrit, int32 Amount);
	void UpdateMana(bool WillCrit, int32 Amount);

#pragma endregion

	// **************************************************************************

	#pragma region Buff System

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TArray<FBuff> ActiveBuffs;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TArray<TEnumAsByte<EStatusAbnormality>> ActiveAbnormalities;

	void ApplyBuff(FBuff Buff)
	{
		ActiveBuffs.Add(Buff);

		Attack_Temp += Buff.AttackTempChange;
		Defense_Temp += Buff.DefenseTempChange;
		MagicAttack_Temp += Buff.MagicAttackTempChange;
		MagicDefense_Temp += Buff.MagicDefenseTempChange;

		AbnormalityIndex = 0;

		if (Buff.CurrentAbnormality != EStatusAbnormality::NoAbnormality)
			ActiveAbnormalities.Add(Buff.CurrentAbnormality);
	}

	void RemoveBuff(FBuff Buff)
	{
		ActiveBuffs.RemoveAt(Buff.BuffID);

		for (size_t i = Buff.BuffID; i < ActiveBuffs.Num(); i++)
		{
			ActiveBuffs[i].BuffID--;
		}

		AbnormalityIndex = 0;

		Attack_Temp -= Buff.AttackTempChange;
		Defense_Temp -= Buff.DefenseTempChange;
		MagicAttack_Temp -= Buff.MagicAttackTempChange;
		MagicDefense_Temp -= Buff.MagicDefenseTempChange;

		if(Buff.DamageOnEnd!=0)
		UpdateHealth(false,-Buff.DamageOnEnd);

		if (Buff.CurrentAbnormality != EStatusAbnormality::NoAbnormality)
			ActiveAbnormalities.Remove(Buff.CurrentAbnormality);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 AbnormalityIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float AbnormalityTimer = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		FString StatusAbnormalitiesTextSweep;

#pragma endregion

	// **************************************************************************

	#pragma region Combat Monster Traits


	int32 GetAttack()
	{
		GLog->Log(FString::FromInt(Attack) + " -- " + FString::FromInt(Attack_Temp));
		return (Attack + Attack_Temp>0) ? Attack + Attack_Temp : 0;
	}

	int32 GetDefense()
	{
		return (Defense + Defense_Temp>0) ? Defense + Defense_Temp : 0;
	}

	int32 GetMagicAttack()
	{
		return (MagicAttack + MagicAttack_Temp>0) ? MagicAttack + MagicAttack_Temp : 0;
	}

	int32 GetMagicDefense()
	{
		return (MagicDefense + MagicDefense_Temp>0) ? MagicDefense + MagicDefense_Temp : 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MagicDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Attack_Temp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Defense_Temp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MagicAttack_Temp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MagicDefense_Temp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float AttackBonus_Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float AttackBonus_Max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 HitChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 AttackCritChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		float AttackCritMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 AlterationTurns;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EElementalPower> MonsterAffinityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		UTexture2D* MonsterPortrait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		bool bIsDead;

	bool bIsDefending;

#pragma endregion
	
	// **************************************************************************

	#pragma region Camera & Positioning Properties for Combat and Action Selection

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Pos Helper")
		AActor* LeInitPosActor;

	FVector InitPos;

	float TurningSpeed;

	FRotator InitialCameraRot;

	#pragma endregion

	// **************************************************************************

	#pragma region Monster Life and Death

	void MonsterDeath();
	void MonsterRez();

#pragma endregion

};
