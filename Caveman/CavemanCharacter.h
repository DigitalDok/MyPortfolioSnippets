// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "CavemanController.h"
#include "GameHUD_UMG.h"
#include "CavemanAnimInstance.h"
#include "CavemanArrow.h"
#include "CavemanRecipeListComponent.h"
#include "Trap.h"
#include "SoundEffectManager.h"
#include "MyCharacter_FirstTry.generated.h"

UCLASS()
class CAVEMANTESTFIELD_API ACavemanCharacter : public ACharacter
{
	GENERATED_BODY()
private:

#pragma region Trap Mechanic Properties

	//If caveman is on trap mode movement should be disabled in order to place trap first
	bool bIsInTrapMode = false;
	//Move the trap on the ground (used only while "InTrapMode")
	void SetNewTrapLocation();
	//Reference for the trap we are trying to place
	ATrap* TrapToBePlaced;

#pragma endregion

public:

#pragma region Functions

#pragma region Core Functions
	
	ACavemanCharacter();

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

#pragma endregion

#pragma region Combat System

	void DamageEnemy(AActor* HitEnemy);

	void DamageGivenEnemy(AActor* HitEnemy, APawn* PawnWhoPerformedTheHit);

	void TakeDamageFromEnemy(float Damage);

	float CauseDamageByMelee();
	float CauseDamageByRanged();

	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float BaseDamageMelee;
	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float BaseDamageRanged;
	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float BaseDef;

	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float MinDamageRanged;
	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float MinDamageMelee;

	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float MaxDamageMelee;
	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float MaxDamageRanged;

	UPROPERTY(EditAnywhere, Category = "Character Combat")
		float CombatResetSeconds;

	float CombatStartSeconds;

	/*True if you want to enable combat and false otherwise.*/
	void SetCombatStatus(bool IsInCombat);

#pragma endregion

#pragma region Input Related Functionality

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	//Input Methods
	void MoveXAxis(float value);
	void MoveYAxis(float value);
	void MouseTurn(float value);
	void MouseTurnUp(float value);
	
	void MyCustomAction(); // Used for testing
	
	void TogglePOVFunc();
	
	void SprintingON();
	void SprintingOFF();

	void InventoryUp();
	void InventoryDown();
	void InventoryRight();
	void InventoryLeft();
	
	void InventoryUse();
	void InventoryDrop();
	void InventoryAssign();
	
	void ToggleInventoryUI();
	void CraftingWindowToggle();

	void UseQuickSlot1();
	void UseQuickSlot2();
	void UseQuickSlot3();
	void UseQuickSlot4();

	void CancelFromPC();
	
#pragma endregion

#pragma region Animation Control

	/*UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* Idle;
	UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* Walk;
	UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* Run;
	UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimationAsset* JumpAnim;
	UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* FallAnimLoop;
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* FallAnimHard;
	/*UPROPERTY(EditDefaultsOnly, Category = "Custom Animations")
		UAnimMontage* BashAnim;*/

	void CavemanAnimationHandling();

	bool bShouldCountFallingMeters;
	float Meters;

	float NormalWalkSpeed;

#pragma endregion

#pragma region Trap Mechanic Functions

	/*Used in order to Place Traps*/
	UFUNCTION(BlueprintCallable, Category = "Trap")
		void PlaceTrap();

	/*Blueprint of trap to spawn*/
	UPROPERTY(EditDefaultsOnly, Category = "Trap")
		TSubclassOf<ATrap> TrapToSpawn;

	/*Max Spawn distance from caveman*/
	UPROPERTY(EditDefaultsOnly, Category = "Trap")
		float SpawnDistance;

#pragma endregion

#pragma region Trait System Functions

	UFUNCTION(BlueprintCallable, Category = "Fires")
		void Health_Up(float Heal);

	void Health_Down(float Damage);
	void Hunger_Up(float HungerPts);

	UFUNCTION(BlueprintCallable, Category = "Fires")
		void Hunger_Down(float HungerPts);

	UFUNCTION(BlueprintCallable, Category = "Fires")
		void Heat_Up(float HeatPts);
	void Heat_Down(float HeatPts);


	void Stamina_Up(float StaPts);
	void Stamina_Down(float StaPts);

	void HungerOverTime(float DeltaTime);
	void HeatOverTime(float DeltaTime);
	void StarvationOverTime(float DeltaTime);

	bool bStarvationHasStarted;

#pragma endregion

#pragma endregion

#pragma region Properties

#pragma region State Controller Properties (Melee / Ranged | First / Third Person)

	UPROPERTY(EditDefaultsOnly, Category = "Custom Movement")
		bool bIs1stPerson;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Movement")
		bool bIsMelee;

#pragma endregion

#pragma region Core Character Component Properties

	UPrimitiveComponent* MyPrimitiveComponent;
	UCameraComponent* FirstPersonCamera;
	UCameraComponent* ThirdPersonCamera;
	USpringArmComponent* SpringArmComponent;
	USkeletalMeshComponent* TheCavemanSkeletalMesh;

	USkeletalMeshComponent* HandSkeletalMesh; // extra hand mesh for melee
	USkeletalMeshComponent* HandSkeletalMeshBow; // extra hand mesh for bow

	UStaticMeshComponent* OurGreatBowComponent3rdPerson; // 3rd person bow
	UStaticMeshComponent* OurGreatBowComponent1stPerson; // 1st person bow
	UStaticMeshComponent* OurGreatClubComponent3rdPerson; // 3rd person club
	UStaticMeshComponent* OurGreatClubComponent1stPerson; // 1st person club

	UAudioComponent* Firebrand; // Battle Theme
	USoundEffectManager* SFXManager;

	ACharacter* LastAccessedCharacter; // Used for looting system 
	UPawnNoiseEmitterComponent* NoiseEmitterComp; // Component responsible for noise

#pragma endregion

#pragma region Movement and Input Properties


	UPROPERTY(EditDefaultsOnly, Category = "Custom Movement")
		float MovementSpeed;

	FVector CurrentVelocity;
	FVector ModelRotatorHelper;

	float X_AxisValue;
	float Y_AxisValue;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Movement")
		float TurningSpeed;

	FRotator LastKnownRotation;

#pragma endregion

#pragma region Primary Trait System (Health)

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Temperature;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character Traits")
	float Health_Max;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character Traits")
	float Health_Current;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
		float Health_DropRate;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
		float Health_Drop_CD;

	float Health_Drop_Timer;
	
#pragma endregion

#pragma region Hunger System

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character Traits")
	float Hunger_Max;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Character Traits")
	float Hunger_Current;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
		float Hunger_Drop_CD;

		float Hunger_Drop_Timer;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Hunger_DropRate;

#pragma endregion

#pragma region Stamina System

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Stamina_Max;
	float Stamina_Current;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Stamina_DropRate;
	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Stamina_IncreaseRate;
	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Stamina_Threshold;

#pragma endregion

#pragma region Heat System

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Heat_Max;
	float Heat_Current;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
		float Heat_Drop_CD;

		float Heat_Drop_Timer;

	UPROPERTY(EditDefaultsOnly, Category = "Character Traits")
	float Heat_DropRate;

#pragma endregion

#pragma region Character Death System

	void KillTheCharacter();

	bool bHasDied;

#pragma endregion

#pragma region Useful References (Properties)

	ACavemanController* Controller;
	UCavemanAnimInstance* MyAnimInstance;

#pragma endregion

#pragma region Weapon Oriented Properties (Melee)

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		UStaticMesh* OurGreatClub;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		UStaticMesh* OurGreatBow;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		TSubclassOf<ACavemanArrow> OurGreatArrow;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FRotator MyClubRotator;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyClubLocation;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyClubScale;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FRotator MyClubRotator1st;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyClubLocation1st;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyClubScale1st;

#pragma endregion

#pragma region Weapon Oriented Properties (Ranged)

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FRotator MyBowRotator1st;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyBowLocation1st;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyBowScale1st;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FRotator MyBowRotator3rd;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyBowLocation3rd;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		FVector MyBowScale3rd;

	

#pragma endregion

#pragma region First Person Animation Assets for my Hands (Extra Mesh to simulate FPS Camera)

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Animations (Melee)")
		UAnimationAsset* FirstPersonIdle;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Animations (Melee)")
		UAnimationAsset* FirstPersonAttack;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Animations (Ranged)")
		UAnimationAsset* FirstPersonIdle_Bow;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Animations (Ranged)")
		UAnimationAsset* FirstPersonPull_Bow;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Animations (Ranged)")
		UAnimationAsset* FirstPersonRelease_Bow;

	void AttackRaycasting();

	void ToggleWeaponMode();
	

	void Attack();
	void AttackRelease();
	bool bBowReleased;
	bool bCanNowAttackRanged;

	bool bIsAttacking;
	bool bCanInflictDamageOnFoes;
	FVector prevBase, prevTip;
	

	
	UPROPERTY(EditAnywhere, Category = "Character Traits")
		float ArrowForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
		int32 ArrowsLeft;

	bool bCannotReAttack;
	void ShootTheArrow();
	
	UStaticMeshComponent* TheArrowThatWeHoldBeforeWeFire;

	void PlayHitAnimation();
	
#pragma endregion

#pragma region Lock On System

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	bool TryToLockOn();
	
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void ClearLockOn();

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Lock On")
	bool bHasLockedTarget;

	UPROPERTY(VisibleAnywhere, Category = "Lock On")
	AActor* EnemyTarget;

	UPROPERTY(EditAnywhere, Category = "Lock On")
	float MaxLockOnRange;

#pragma endregion

#pragma region Control Booleans

	bool bIsInCombat;
	bool bWillLoot;
	bool bLootWindowOpen;
	/*Used for smooth camera switch*/
	bool bIsChangingCameras;
	bool isSprinting;
	bool bIsCraftingWindowOpen;
	
#pragma endregion

#pragma region Inventory Quickslot System

	
	void UseItem(FString ItemName);
	void UseItemThroughSlot(FString ItemName);

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Slots")
	FString ItemInSlot1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Slots")
	FString ItemInSlot2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Slots")
	FString ItemInSlot3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Slots")
	FString ItemInSlot4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stuff")
		FString WeaponName;

	float CurCDTimerQuickSlot;

	UPROPERTY(EditAnywhere, Category = "Character Slots")
	float CDTimerQuickSlot;

	void AssignNameToSlot(FString Name, int32 SlotID);
	void RemoveAnyPossibleSlotConflicts(FString Name, int32 SlotID);

	void UpdateUIQuickSlots(FString Slot1, FString Slot2, FString Slot3, FString Slot4);

#pragma endregion

#pragma region Interaction System

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		FString InteractionTargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		float InteractionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		AActor* InteractionTarget;

	void InteractionHandling(); // Runs all the time
	void Interact(); // Runs only when I press "Interact" (E)

#pragma endregion

#pragma region Data Oriented Properties	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UCavemanRecipeListComponent* RecipeList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UDataTable* Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UCavemanRecipeListComponent*  CookRecipeList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
		UDataTable* CookData;

#pragma endregion

#pragma region Fire Starting Menu

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsFireMainMenuEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsFireStartingWindowEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		FString InteractionMode;

	UFUNCTION(BlueprintCallable, Category = "Fires")
		void EnableTheFireOnThisCampfire(bool BigFire);

	UFUNCTION(BlueprintCallable, Category = "Fires")
		void DisableTheFireOnThisCampfire(bool BigFire);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsInFireRadius;

#pragma endregion

#pragma region Collision Properties

	UFUNCTION()
		void OnCavemanBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
		void OnCavemanEndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma endregion

#pragma region Resting Menu Properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsResting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsRestingWindowOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsDiseased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsStartingFireAtTheMoment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dok Custom C++ ")
		bool bIsCraftingAtTheMoment;

#pragma endregion
	
#pragma region AI Noise Properties

	/*Used for Noise in order for AI to detect the fucking caveman*/
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void CavemanMakeNoise(float Loudness);

#pragma endregion

#pragma endregion

};
