// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "HeroAnimInstance.h"
#include "DokController.h"
#include "AmmoBox.h"
#include "RoguelikeChar.generated.h"

UCLASS()
class ROGUELIKESURVIVAL_API ARoguelikeChar : public ACharacter
{
	GENERATED_BODY()

private:
	/*Spawns the Character in a random floor tile*/
	void SpawnCharInRandomTile();

	void SpawnZombiesSpawnPoints();

	void SpawnPowerUps();

	void SpawnHealthKits();

	/*Spawns the double damage powerups*/
	void SpawnDDs();

public:

#pragma region Functions

#pragma region Predefined Functions

	ARoguelikeChar();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

#pragma endregion

#pragma region Input

	void CharacterMouseTurn(float AxisValue);
	void CharacterMouseLookUpDown(float AxisValue);

	void CharacterMoveForward(float AxisValue);
	void CharacterMoveRight(float AxisValue);

	void CharacterHoldAim();
	void CharacterReleaseAim();

	void CharacterHoldSprint();
	void CharacterReleaseSprint();

	void CharacterShoot();
	void CharacterJump();

	void Reload();

	void Cycle();

#pragma endregion

#pragma region Gameplay Functions


	// ---------------- Initialization Functions ----------- //
	
	void InitializeGameplayStats();

	void GameplayComponentReferencing();

	// --------------- Stamina Related Functions ----------- //

	void ChangeStamina(float Amount);


	// --------- Shooting Related Functions -------------- //

	UFUNCTION(BlueprintCallable, Category = "C++ Functionality")
		void ShootABullet();

	UFUNCTION(BlueprintCallable, Category = "C++ Functionality")
		void RefillAmmo();

	// ------- Death Related Functions ------------------- //

	void DeathCameraMovement(float DeltaTime);


	// ------- Laser Related Functions ------------------- //

	void GenerateLaserBeam();

	void ToggleLaserVisibility(bool Visibility);

#pragma endregion


#pragma endregion

#pragma region Fields

#pragma region Components

	UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = "DeathMechanic")
	UCameraComponent* DeathCamera;

	USpringArmComponent* PlayerSpringArm;

	UPROPERTY(EditAnywhere, Category = "Gun Mesh")
		UStaticMeshComponent* PlayerMachineGun;


	UStaticMeshComponent* GunPositionHelperComponent;

	UParticleSystemComponent* PlayerLaser;

	UChildActorComponent* LaserSource;

	UPointLightComponent* LaserTarget;

	USkeletalMeshComponent* CharacterMesh;

#pragma endregion

#pragma region Gameplay

	int32 BulletsLeft_A;
	int32 BulletsLeft_B;
	int32 BulletsLeft_C;

	int32 BulletsLeft_A_Total;
	int32 BulletsLeft_B_Total;
	int32 BulletsLeft_C_Total;

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 MaxAmmoEquipped;

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 MaxAmmoHolderSize;

	float MaxHealth;

	float CurrentHealth;

	/*Increments the health of the character clamping to max health limit*/
	void IncreaseHealth(float HpToAdd);

	int32 Kills;

	int32 Wave;

	bool bIsDead;

	UPROPERTY(EditAnywhere, Category = "Shaking")
	TSubclassOf<UCameraShake> OurShake;

	UPROPERTY(EditAnywhere, Category = "Shaking")
	float ShakePower;

#pragma endregion

#pragma region Gun Mesh

	UPROPERTY(EditAnywhere, Category = "Gun Mesh")
		UStaticMesh* MachineGunMesh;

#pragma endregion

#pragma region Movement Controls

	UPROPERTY(EditAnywhere, Category = "Character Movement")
		float TurningSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Movement")
		float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Movement")
		float SprintMultiplier;

	UPROPERTY(EditAnywhere, Category = "Character Movement")
		float ClampingAngle;

#pragma endregion

#pragma region Ammunition

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 User_BulletsLeft_A;

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 User_BulletsLeft_B;

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 User_BulletsLeft_C;

	UPROPERTY(EditAnywhere, Category = "Ammunition")
		int32 CurrentBulletEquipped;

#pragma endregion

#pragma region Sprint

	float CurrentStamina;
	float MaxStamina;

	bool bIsOnCooldown;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float CooldownResetLim;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float SprintStaminaRequiredMinimum;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float StaminaRate;

#pragma endregion

#pragma region Damage

	void TakeDamageFromZombie(float Damage);

	void DiePainfully();

	UPROPERTY(EditAnywhere, Category = "Damage")
		float MinBulletDamageOrdinary;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float MaxBulletDamageOrdinary;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float MinBulletDamageIncinerary;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float MaxBulletDamageIncinerary;

	/*The duration of the dot damage*/
	UPROPERTY(EditAnywhere, Category = "Damage")
		float DotDuration;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float MinBulletDamageFrost;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float MaxBulletDamageFrost;

	bool bIsDoubleDamageEnabled;

	float CalculateBulletDamage(int32 BulletID);

	void IncreaseBulletsLeft(EAmmoType BulletsType,int32 Bullets);

#pragma endregion

#pragma region Death

	UPROPERTY(EditAnywhere, Category = "Death Mechanic")
		float DeathCamSpinRate;

	UPROPERTY(EditAnywhere, Category = "Death Mechanic")
		float DeathCamTranslateRate;

#pragma endregion

#pragma region Wave System

	UPROPERTY(EditAnywhere, Category = "Wave System")
		float SecondsToRestBetweenWaves;

	bool bIsWaveCountingDown;

	int32 WaveCountdown;

#pragma endregion

#pragma region Powerup

	FString PowerupName;

#pragma endregion   <-------------- Orfeas this is your spot

#pragma region Referencing

	UHeroAnimInstance* PlayerAnimationInstance;

	ADokController* PlayerController;

#pragma endregion

#pragma region Helper Fields

#pragma region Movement

	float InitialMovementSpeed;

	bool bIsSprinting;

	bool bIsAiming;

#pragma endregion

#pragma region Animation

	float MoveDirX;

	float MoveDirY;

#pragma endregion

#pragma region Aiming

	UPROPERTY(EditAnywhere, Category = "Aim Effects")
		float FOVDropRate;

	UPROPERTY(EditAnywhere, Category = "Aim Effects")
		float FOVLowLimit;

	UPROPERTY(EditAnywhere, Category = "Aim Effects")
		float FOVHighLimit;

	FVector LaserTargetLocation;

#pragma endregion

#pragma endregion

#pragma region Debugging

	int32 TestHits;

#pragma endregion

#pragma endregion


};
