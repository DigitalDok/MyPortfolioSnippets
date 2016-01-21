// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "CustomMonsterActor.h"

#include "CombatController.generated.h"

UENUM()
enum EActionPhasesAttack
{
	TargetSelection,
	CameraOnActor,
	MovingTowardsEnemy,
	RestAfterMoveFinished,
	MeleeAttack,
	RestAfterAttack,
	MoveTowardsInitialPosition,
	ResetTurn
};

UENUM()
enum EActionPhasesItem
{
	TargetSelection_Friendly,
	CameraOnActor_Item,
	MovingTowardsFriend,
	AppliedItemRest,
	MoveTowardsInitialPosition_Item,
	ResetTurnItem,
	GlobalItemUsageWait,
	GlobalItemUsedWait
};

UENUM()
enum EActionPhasesAbility
{
	TargetSelectionAbility,
	CameraOnActorAbility,
	ActorCastingSpell,
	ActorSpellCastRest,
	TargetActorCameraRest,
	TargetActorCameraEffect,
	SpellFinalizing,
	SpellFinalized,
	ResetTurnAbility
};

UENUM()
enum EActionPhasesDefend
{
	DefendingParticle,
	ResetTurnDef
};

UENUM()
enum EActionPhasesType
{
	Attack,
	Ability,
	Item,
	Defend,
	Undefined
};

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
		float DmgBonus_Min;

	UPROPERTY(EditAnywhere)
		float DmgBonus_Max;

	UPROPERTY(EditAnywhere)
		int32 SpecialDefence;

	UPROPERTY(EditAnywhere)
		int32 SpecialAttack;

	UPROPERTY(EditAnywhere)
		int32 CritChance;

	UPROPERTY(EditAnywhere)
		float CritMultiplier;

	UPROPERTY(EditAnywhere)
		int32 HitChance;

	UPROPERTY(EditAnywhere)
		UTexture2D* MonsterPortrait;

	UPROPERTY(EditAnywhere)
		TEnumAsByte<EElementalPower> MonsterAffinityType;

	UPROPERTY(EditAnywhere)
		FSpellBook MonsterSpellbook;

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

	UPROPERTY(EditAnywhere, Category = "User Interface Control")
		TSubclassOf<class UUserWidget> UI_Holder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Interface Control")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Based")
	TArray<ACustomMonsterActor*> TurnOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Based")
	int32 IndexOfCurrentPlayingMonster = -1;

	bool bWillChangeAfterDeath;

	// ********************************************************************************************
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
		float TurningSpeed;

	AActor* CameraPartyA;
	AActor* CameraPartyB;
	AActor* CameraOverhead;
	TArray<AActor*> MonsterStaticCams;

	bool bIsOverheadCamera;

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Stats")
		TArray<FMonster> MonsterStats;

	// ********************************************************************************************
	
	FVector GetEnemyMeleeReceiverPosByID(int32 ID);

	FVector GetEnemyPosByID(int32 ID);

	AController* GetEnemyControllerByID(int32 ID);

	AActor* GetEnemyActorByID(int32 ID);

	FVector ReceiverPos;

	FVector InitialPosition;

	FRotator FindLookAtRotation(FVector Start, FVector Target);

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
		TEnumAsByte<EActionPhasesAttack> CurrentAttackPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
		TEnumAsByte<EActionPhasesDefend> CurrentDefendPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
		TEnumAsByte<EActionPhasesItem> CurrentItemPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
		TEnumAsByte<EActionPhasesAbility> CurrentAbilityPhase;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
	TEnumAsByte<EActionPhasesType> CurrentPhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
	float HelperPhaseTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Control")
	float GlobalWaitingTimer;

	// ********************************************************************************************

	void ApplyDamage(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim);

	void ApplyMagicalEffect(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim, FAbility Spell);
	
	void ApplyItem(FItem ItemToApply, ACustomMonsterActor* Target);
	
	float GetMultiplierBasedOnElements(EElementalPower Element_Spell, EElementalPower Element_Victim);

	// ********************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
		FInventory GroupInventories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
	FInventory GroupAInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
	FInventory GroupBInventory;

	void InitializeInventoryStock();

	// ********************************************************************************************

	void PlaySound(USoundWave* SoundEffect);

	TArray<UAudioComponent*> SoundEffects;

	TScriptDelegate<FWeakObjectPtr> AudioFinishDelegate;

	UFUNCTION()
	void DestroyFinishedAudio();
	
	// ********************************************************************************************

	void Attacking(float DeltaTime);
	void UsingItems(float DeltaTime);
	void UsingAbilities(float DeltaTime);
	void Defending(float DeltaTime);

	void CheckForWinConditions();

	// ********************************************************************************************

	void InitializerGrande();
	bool bIsInitialized;

	void ReferenceCameras();
	
	void ChangeCamera(ECameras CamType);
	void ChangeCamera(int32 MonsterID);

	void DetermineParties();
	void DetermineTurnOrder();
	void EndTurn();
};
