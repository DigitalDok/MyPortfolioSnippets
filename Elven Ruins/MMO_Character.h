
/////////////////////////////////////////////////////
// Main Character of our Online RPG - Kostas Dokos //
/////////////////////////////////////////////////////

#pragma once

#include "MMO_Player_Controller.h"
#include "ObjectPool.h"
#include "MMO_Character.generated.h"


UCLASS()
class PIRATELAND_API AMMO_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AMMO_Character();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
	#pragma region Tick Helpers

	void LockedTargetMechanics();
	void ArcherFocusMode(float DeltaTime);
	void HealingPotionMechanic(float DeltaTime);
	void GetOtherCharacterInfo();
	void UI_Updater();

#pragma endregion

	#pragma region Components

	UPROPERTY(Replicated)
	USkeletalMeshComponent* Player_SkeletalMeshComponent;

	USpringArmComponent* Player_SpringArmComponent;

	UPROPERTY(Replicated)
	UCameraComponent* LockOnCam;

	UCameraComponent* MainCam;

	//This Refers to the Component that holds the OverheadUI.
	UWidgetComponent* Player_Widget; 

	UPROPERTY(BlueprintReadWrite, Category="UI",Replicated)
		UMMO_OverheadUI_HUD* OverheadUI;

	// This is the arrow that our archer holds when preparing for a shot.
	UStaticMeshComponent* HandArrow;

	//This is the location of the arrow, when it spawns.
	UStaticMeshComponent* ShootingLocation;

	//This is the *VIRTUAL* target of our arrow, refering to an invisible sphere in front of the camera, in order to determine direction.
	UStaticMeshComponent* ShootTarget;

	AObjectPool*	      GlobalPool;
	
#pragma endregion

	#pragma region Character Stats

	UPROPERTY(EditAnywhere, Category = "Character Traits", Replicated)
		int32 DamageOutput;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		int32 BaseAttack;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		int32 AttackBonusMin;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		int32 AttackBonusMax;

	// Must be a float between 0.0 and 1.0, representing Percetage of damage reduced.
	UPROPERTY(EditAnywhere, Category = "Character Traits")
		float BaseDamageReduction;

	UPROPERTY(EditAnywhere, Category = "Character Traits")
		bool bIsWarrior;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
		FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Traits", Replicated)
		float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits", Replicated)
		float MaxHealth;

#pragma endregion

	#pragma region Gameplay

	#pragma region Healing Potion Mechanic

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing Potion")
		int32 HealingPotions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing Potion")
		int32 HealingPotionPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing Potion")
		float HealingPotionCooldown;

	float HealingPotionCurrentTimer;

	bool bIsHealingPotInCD;

	void DrinkPotion();

#pragma endregion

	#pragma region Damaging / Death Mechanics


	UPROPERTY(Replicated)
	bool bIsDead;

	UPROPERTY(Replicated)
	FRotator DeathRotation;

	void TakeDamageFromEnemy(int32 Damage, AActor* Monster);

	void UpdateHealth(int32 Damage);

	void Die();

#pragma endregion
	
	#pragma region Lock-On Mechanic

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Lock On")
		int32 LockOnID;
	
	void AttemptToLockOn();

	UPROPERTY(EditAnywhere, Category = "Lock On")
		float PingAreaForLockOn;

	UPROPERTY(EditAnywhere, Category = "Lock On")
		AActor* LockedTarget;

	#pragma endregion
	
	#pragma region Input Related 

	UPROPERTY(EditAnywhere, Category="Input Related")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Input Related")
	float TurningSpeed;

	void MoveRightInput(float AxisValue);
	void MoveForwardInput(float AxisValue);
	
	void TurnUp(float AxisValue); 
	void TurnRight(float AxisValue);

	void JumpAction();

	#pragma endregion

	#pragma region Combat Sytem

#pragma region Ranged Combat System

		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
		int32 ArrowsLeft;

		UPROPERTY(EditAnywhere, Category = "Ranged Combat", Replicated)
			bool bIsHoldingBow;

		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
			TSubclassOf<AActor> MyArrowObject;

		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
			float ArrowForce;

		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
			float FocusSpringLength;
		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
			float NormalSpringLength;
		UPROPERTY(EditAnywhere, Category = "Ranged Combat")
			float SpringArmLengthChangeRate;

		FVector LastKnownLocationOfTarget;

		bool bIsInFocusMode;

		void FocusModeOn();
		void FocusModeOff();

		void BowAttackOn();
		void BowAttackOff();

		void ShootArrow();

		UPROPERTY(Replicated)
			int32 FinalArrowDamage;

#pragma endregion

#pragma region Melee Combat System

	FVector prevBase;
	FVector prevTip;

	UPROPERTY(EditAnywhere, Category = "Melee Combat", Replicated)
		bool bAttackNow;

	UPROPERTY(EditAnywhere, Category = "Melee Combat", Replicated)
		bool bIsAttacking;

	void MeleeAttackRaycasts();

	void LocalAttackMelee();

	void ShieldBlockOn();
	void ShieldBlockOff();

	UPROPERTY(Replicated)
		bool bIsShieldBlocking;

#pragma endregion

#pragma endregion

#pragma endregion

	#pragma region Visuals

	#pragma region Rotation Fix 

	UPROPERTY(EditAnywhere, Category = "Rotation Fix", Replicated)
	float MoveX;

	UPROPERTY(EditAnywhere, Category = "Rotation Fix", Replicated)
	float MoveY;

	UPROPERTY(EditAnywhere, Category = "Rotation Fix", Replicated)
	FRotator LookingDir;

	UPROPERTY(EditAnywhere, Category = "Rotation Fix", Replicated)
	FRotator LastKnownRotation;

	void RotationFixer();

	#pragma endregion

	#pragma region Animation Related

	UPROPERTY(EditAnywhere, Category = "Custom Montages")
		UAnimMontage* GetHurtMontage;

	UPROPERTY(EditAnywhere, Category = "Custom Montages")
		UAnimMontage* GetHurtMontageShield;

	UPROPERTY(EditAnywhere, Category = "Custom Montages", Replicated)
		UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Custom Montages", Replicated)
		UAnimMontage* SecondAttackMontage;

	void AnimationHandler();

	#pragma endregion

#pragma endregion

	#pragma region Helpers

	#pragma region References

	UPROPERTY(Replicated)
		UMMO_Char_AnimInstance* AnimInstance;

	AMMO_Player_Controller* MyController;

	AMMO_Character* OtherCharacter;

	ABoss* TheBAWS;

	#pragma endregion

	#pragma region Networking Function Declarations

	UFUNCTION(Server, Reliable, WithValidation)
		void MoveForward(float AxisValue);
		void MoveForward_Implementation(float AxisValue);
		bool MoveForward_Validate(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation)
		void MoveRight(float AxisValue);
		void MoveRight_Implementation(float AxisValue);
		bool MoveRight_Validate(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation)
		void AttackMelee();
		void AttackMelee_Implementation();
		bool AttackMelee_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);
		void Server_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target);
		bool Server_AggroTarget_Validate(AMMO_Mob_Character* Monster, AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);
		void Multicast_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_DrinkPotion();
		void Server_DrinkPotion_Implementation();
		bool Server_DrinkPotion_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_DrinkPotion();
		void Multicast_DrinkPotion_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void UpdateHealth(float Damage);
		void UpdateHealth_Implementation(float Damage);
		bool UpdateHealth_Validate(float Damage);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerTakeDamageFromEnemy(int32 Damage, AActor* Monster);
		bool ServerTakeDamageFromEnemy_Validate(int32 Damage, AActor* Monster);
		void ServerTakeDamageFromEnemy_Implementation(int32 Damage, AActor* Monster);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastTakeDamageFromEnemy(int32 Damage, AActor* Monster);
		void MulticastTakeDamageFromEnemy_Implementation(int32 Damage, AActor* Monster);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PlayAnimation(UAnimMontage* Anim);
		void Multicast_PlayAnimation_Implementation(UAnimMontage* Anim);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PlaySound(USoundBase* SFX, FVector Loc);
		void Multicast_PlaySound_Implementation(USoundBase* SFX, FVector Loc);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_Die();
		void Multicast_Die_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void ShutDownColliders(AMMO_Mob_Character* Target);
		void ShutDownColliders_Implementation(AMMO_Mob_Character* Target);
		bool ShutDownColliders_Validate(AMMO_Mob_Character* Target);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShutDownColliders(AMMO_Mob_Character* Target);
		void Multicast_ShutDownColliders_Implementation(AMMO_Mob_Character* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void GetDamage(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, AMMO_Mob_Character* Target);
		void GetDamage_Implementation(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, AMMO_Mob_Character* Target);
		bool GetDamage_Validate(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, AMMO_Mob_Character* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void ApplyDamageToMonster(int32 Damage, AMMO_Mob_Character* Target);
		void ApplyDamageToMonster_Implementation(int32 Damage, AMMO_Mob_Character* Target);
		bool ApplyDamageToMonster_Validate(int32 Damage, AMMO_Mob_Character* Target);

	UFUNCTION(NetMulticast, Reliable)
		void MCApplyDamageToMonster(int32 Damage, AMMO_Mob_Character* Target);
		void MCApplyDamageToMonster_Implementation(int32 Damage, AMMO_Mob_Character* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void GetDamage_Boss(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, ABoss* Target);
		void GetDamage_Boss_Implementation(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, ABoss* Target);
		bool GetDamage_Boss_Validate(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, ABoss* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void ApplyDamageToMonster_Boss(int32 Damage, ABoss* Target);
		void ApplyDamageToMonster_Boss_Implementation(int32 Damage, ABoss* Target);
		bool ApplyDamageToMonster_Boss_Validate(int32 Damage, ABoss* Target);

	UFUNCTION(NetMulticast, Reliable)
		void MCApplyDamageToMonster_Boss(int32 Damage, ABoss* Target);
		void MCApplyDamageToMonster_Boss_Implementation(int32 Damage, ABoss* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLockOn();
		void ServerLockOn_Implementation();
		bool ServerLockOn_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastLockOn();
		void MulticastLockOn_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastAttackMelee();
		void MulticastAttackMelee_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ShieldBlockOn();
		void Server_ShieldBlockOn_Implementation();
		bool Server_ShieldBlockOn_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShieldBlockOn();
		void Multicast_ShieldBlockOn_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ShieldBlockOff();
		void Server_ShieldBlockOff_Implementation();
		bool Server_ShieldBlockOff_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShieldBlockOff();
		void Multicast_ShieldBlockOff_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_BowAttackOn();
		void Server_BowAttackOn_Implementation();
		bool Server_BowAttackOn_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_BowAttackOff();
		void Server_BowAttackOff_Implementation();
		bool Server_BowAttackOff_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_BowAttackOn();
		void Multicast_BowAttackOn_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_BowAttackOff();
		void Multicast_BowAttackOff_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ShootArrow();
		void Server_ShootArrow_Implementation();
		bool Server_ShootArrow_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShootArrow();
		void Multicast_ShootArrow_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerCalcArrowDamage(int32 ATK, int32 Min, int32 Max);
		void ServerCalcArrowDamage_Implementation(int32 ATK, int32 Min, int32 Max);
		bool ServerCalcArrowDamage_Validate(int32 ATK, int32 Min, int32 Max);

#pragma endregion

	// ************** BOSS VARIABLES ******************** // 

	UPROPERTY(BlueprintReadWrite, Category = "MantineeBlocker")
		bool bMantineeHasControl;

	// ************** MONSTER FIXERS ******************** // 

	void AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);

#pragma endregion



};
