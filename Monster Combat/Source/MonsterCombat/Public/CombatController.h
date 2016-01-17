// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "CustomMonsterActor.h"
#include "ActionHUD.h"
#include "CombatController.generated.h"

USTRUCT(BlueprintType)
struct FMonster
{
	GENERATED_USTRUCT_BODY()
	FMonster()
	{

	}

	UPROPERTY(EditAnywhere)
		TEnumAsByte<EMonsterType> MonsterType;

	UPROPERTY(EditAnywhere)
		TArray<FString> MonsterNames;

	UPROPERTY(EditAnywhere)
		int32 MaxHP;

	UPROPERTY(EditAnywhere)
		int32 MaxMP;

	UPROPERTY(EditAnywhere)
		int32 Speed;

	UPROPERTY(EditAnywhere)
		int32 Defence;

	UPROPERTY(EditAnywhere)
		int32 Attack;

	UPROPERTY(EditAnywhere)
		int32 SpecialDefence;

	UPROPERTY(EditAnywhere)
		int32 SpecialAttack;

};

UENUM()
enum ECameras
{
	Camera_Party_A,
	Camera_Party_B,
	MonsterCamA1,
	MonsterCamA2,
	MonsterCamA3,
	MonsterCamB1,
	MonsterCamB2,
	MonsterCamB3,
	FreeRoamCamera,
	NoneCam
};
/**
 * 
 */
UCLASS()
class MONSTERCOMBAT_API ACombatController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACombatController();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> UI_Holder;

	UActionHUD* MyActionHUD;

	// ********************************************************************************************

	virtual void SetupInputComponent() override;

	void LMB();
	void RMB();

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grouping")
		TArray<ACustomMonsterActor*> PartyA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grouping")
		TArray<ACustomMonsterActor*> PartyB;
	
	// ********************************************************************************************

	TArray<ACustomMonsterActor*> TurnOrder;

	int32 IndexOfCurrentPlayingMonster = -1;

	// ********************************************************************************************
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
		float TurningSpeed;

	AActor* CameraPartyA;
	AActor* CameraPartyB;

	bool bIsActorClickEnabled;

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Stats")
		TArray<FMonster> MonsterStats;

	// ********************************************************************************************
	
	FVector GetEnemyMeleeReceiverPosByID(int32 ID);
	AController* GetEnemyControllerByID(int32 ID);

	bool bIsWalkingTowardsDestination;
	FVector ReceiverPos;

	// ********************************************************************************************
	void ReferenceCameras();
	
	void ChangeCamera(ECameras CamType);

	void DetermineParties();
	void DetermineTurnOrder();
	void EndTurn();
};
