
////////////////////////////////////////////////////////////////////////////////////////////
//				Turn Based System (Including Combat, Cameras, Input and Phases)			  //
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameFramework/PlayerController.h"
#include "CustomMonsterActor.h"
#include "ObjectPooler.h"
#include "CombatController.generated.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The Game Phases Enums used in Phase control and Tick Events to determine available in-game actions and execution of gameplay. //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////
//				The Core Structure of a Monster (Stats, Portraits, Name)				  //
////////////////////////////////////////////////////////////////////////////////////////////

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
		TEnumAsByte<EAIArchetypes> AI_Archetype;

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

////////////////////////////////////////////////////////////////////////////////////////////
//					A List of All the Cameras used in gameplay.							  //
////////////////////////////////////////////////////////////////////////////////////////////

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

UCLASS()
class MONSTERCOMBAT_API ACombatController : public APlayerController
{
	GENERATED_BODY()
	
public:

	// ********************************************************************************************

	#pragma region Core Functions

	ACombatController();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	#pragma endregion

	// ********************************************************************************************

	#pragma region Object Pool

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Pool Object")
		AObjectPooler* GlobalPool;

	UPROPERTY(EditAnywhere, Category = "Global Pool Object")
		TArray<TEnumAsByte<ESoundEffectLibrary>> Sounds_TXT;

	UPROPERTY(EditAnywhere, Category = "Global Pool Object")
		TArray<USoundBase*> Sounds_SFX;

	UPROPERTY(EditAnywhere, Category = "Global Pool Object")
		TMap<TEnumAsByte<ESoundEffectLibrary>, USoundBase*> SFX_Dictionary;

	#pragma endregion

	// ********************************************************************************************

	#pragma region User Interface

	UPROPERTY(EditAnywhere, Category = "User Interface Control")
		TSubclassOf<class UUserWidget> UI_Holder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Interface Control")
	UActionHUD* MyActionHUD;

	#pragma endregion

	// ********************************************************************************************

	#pragma region Input

	virtual void SetupInputComponent() override;

	void LMB();
	void RMB();

	#pragma endregion

	// ********************************************************************************************

	#pragma region Monster Groups

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grouping")
		TArray<ACustomMonsterActor*> PartyA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grouping")
		TArray<ACustomMonsterActor*> PartyB;

	#pragma endregion
	
	// ********************************************************************************************

	#pragma region Turn Based System

	#pragma region Core

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Based System")
	TArray<ACustomMonsterActor*> TurnOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Based System")
	int32 IndexOfCurrentPlayingMonster = -1;

	void InitializerGrande();
	bool bIsInitialized;

	void SetBottomTooltip(bool bShouldReset, FString ActualText);

	void DetermineParties();
	void DetermineTurnOrder();
	void EndTurn();

	#pragma endregion

	#pragma region Winning Conditions

	bool bWillChangeAfterDeath;

	int32 WinningCondition = 0; // 0 = Noone has won yet. 1 = We have won. 2 = AI has won.
	bool bWinCountdown;

	void CheckForWinConditions();

#pragma endregion
	
	#pragma region Phase System

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

	#pragma endregion

	#pragma region Helper Tick Functions

	void Attacking(float DeltaTime);
	void UsingItems(float DeltaTime);
	void UsingAbilities(float DeltaTime);
	void Defending(float DeltaTime);

	#pragma endregion

	#pragma endregion

	// ******************************************************************************************** 
	
	#pragma region Camera System

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera System")
		float TurningSpeed;

	AActor* CameraPartyA;
	AActor* CameraPartyB;
	AActor* CameraOverhead;
	TArray<AActor*> MonsterStaticCams;

	bool bIsOverheadCamera;


	void ReferenceCameras();

	void ChangeCamera(ECameras CamType);
	void ChangeCamera(int32 MonsterID);

	#pragma endregion

	// ********************************************************************************************

	#pragma region Data Entry (For Designers)

	#pragma region Global Monster Stats

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Stats")
		TArray<FMonster> MonsterStats;

	#pragma endregion

	#pragma region Global Inventories

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
		FInventory GroupInventories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
		FInventory GroupAInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Management")
		FInventory GroupBInventory;

	void InitializeInventoryStock();

	#pragma endregion

	#pragma endregion

	// ********************************************************************************************

	#pragma region Combat System

	#pragma region Combat Main Functions

	void ApplyDamage(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim);

	void ApplyMagicalEffect(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim, FAbility Spell);
	
	void ApplyItem(FItem ItemToApply, ACustomMonsterActor* Target);
	
	float GetMultiplierBasedOnElements(EElementalPower Element_Spell, EElementalPower Element_Victim);

	#pragma endregion

	#pragma region Combat Helper Functions

	FVector GetEnemyMeleeReceiverPosByID(int32 ID);

	FVector GetEnemyPosByID(int32 ID);

	AController* GetEnemyControllerByID(int32 ID);

	AActor* GetEnemyActorByID(int32 ID);

	FVector ReceiverPos;

	FVector InitialPosition;

	FRotator FindLookAtRotation(FVector Start, FVector Target);

	#pragma endregion

	#pragma endregion

	// ********************************************************************************************

	#pragma region AI Related

	void AI_ExecuteAction();
	bool bIsAI_PseudoThink;

	bool bHasFinishedPath;

	#pragma endregion
	
	// ********************************************************************************************

};
