// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "MonsterAnimInstance.h"
#include "ActionHUD.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "CustomMonsterActor.generated.h"

UENUM()
enum EPartyType
{
	PlayerControlled,
	AI_Controlled
};

USTRUCT()
struct FBuff
{
	GENERATED_USTRUCT_BODY()

	FBuff()
	{

	}
	
public:
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
		int32 DamageOnEnd;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		TEnumAsByte<EStatusAbnormality> CurrentAbnormality;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 TurnsLifetime;

	UPROPERTY(EditAnywhere, Category = "Buff Code")
		int32 BuffID;
};

UENUM()
enum EMonsterType
{
	Spider,
	FlowerMonster,
	Orc,
	MagmaDemon,
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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EPartyType> Party;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
	UCameraComponent* MonsterCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
	UStaticMeshComponent* MonsterCameraRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Components")
	UWidgetComponent* MyWidgetComponent;

	bool AllDone;
	//***************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		TEnumAsByte<EMonsterType> MonsterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 MaxMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		int32 CurrentMana;

	void UpdateHealth(bool WillCrit, int32 Amount);
	void UpdateMana(bool WillCrit, int32 Amount);

	// **************************************************************************

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

	// **************************************************************************

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
	
	// **************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Pos Helper")
		AActor* LeInitPosActor;

	FVector InitPos;

	// **************************************************************************

	bool bIsDefending;

	

	void MonsterDeath();

	// **************************************************************************
	
	float TurningSpeed;

	FRotator InitialCameraRot;

	// **************************************************************************

	UMonsterAnimInstance* AnimInstance;
};
