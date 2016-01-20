// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "MonsterAnimInstance.h"

#include "CustomMonsterActor.generated.h"

UENUM()
enum EPartyType
{
	PlayerControlled,
	AI_Controlled
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Base Properties")
		UTexture2D* MonsterPortrait;
	
	// **************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Pos Helper")
		AActor* LeInitPosActor;

	FVector InitPos;

	// **************************************************************************

	bool bIsDead;

	void MonsterDeath();

	// **************************************************************************
	
	float TurningSpeed;

	FRotator InitialCameraRot;

	// **************************************************************************

	UMonsterAnimInstance* AnimInstance;
};
