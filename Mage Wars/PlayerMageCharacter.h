// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "MageAnimInstance.h"
#include "MageMastermind.h"
#include "UnrealNetwork.h"
#include "MageStats.h"
#include "WidgetComponent.h"
#include "EnemyHealthBarUI.h"
#include "SoundEffectManager.h"
#include "PlayerMageCharacter.generated.h"

UCLASS()
class MAGEWARS_API APlayerMageCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayerMageCharacter();

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;
	//
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
	// ****************************************************************** COMPONENTS *************************************************** //

	#pragma region Components

	UPROPERTY(EditAnywhere, Category="Mage Components")
	USkeletalMesh* PlayerMageModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Components")
	USkeletalMeshComponent* PlayerComp_SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
	UCameraComponent* PlayerComp_Camera;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
		USpringArmComponent* PlayerComp_SpringArm;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
		USpringArmComponent* PlayerComp_SpringArmExtension;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
		UMageStats* PlayerComp_Stats;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
		UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditAnywhere, Category = "Mage Components")
		UEnemyHealthBarUI* DamageOverlayComp;

	UParticleSystemComponent* IceTrapParticle;

	USoundEffectManager* SFXManager;

#pragma endregion

	// ******************************************************************

	#pragma region Animation Related

	UPROPERTY(EditAnywhere, Category = "Mage Animations")
	UAnimMontage* MageAttackingMontage;

	void PlayReplicatedSpellAnimation(int32 AnimationID, APlayerMageCharacter* Mage);

	#pragma endregion

	// ******************************************************************

	#pragma region Projectile Related

	UPROPERTY(Replicated)
		bool bIsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Projectiles")
		TSubclassOf<AActor> PlayerProjectile;

	UPROPERTY(Replicated)
	bool bHasSpawnedProjectile=false;
	UPROPERTY(Replicated)
	float bHasSpawnedProjectile_Timer;
	UPROPERTY(Replicated)
	float bHasSpawnedProjectile_Cooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack", Replicated)
		float FinalDamageFromProjectile;

	void Local_ShootProjectile(float FinalDamage);

	UPROPERTY(EditAnywhere, Category = "Mage Attack")
		float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "Mage Attack")
		float ProjectileLifetime;

#pragma endregion

	// ******************************************************************

	#pragma region Input Functionality

	void CharacterMouseTurn(float AxisValue);
	void CharacterKeyboardTurn(float AxisValue);
	void CharacterMouseLookUpDown(float AxisValue);

	void CharacterMoveForward(float AxisValue);
	void CharacterMoveRight(float AxisValue);

	void PrepareSpell_1();
	void PrepareSpell_2();
	void PrepareSpell_3();
	void PrepareSpell_4();

	void ExecuteSpell_1();
	void ExecuteSpell_2();
	void ExecuteSpell_3();
	void ExecuteSpell_4();

	void CharacterJump();

	void CharacterAttackOn();
	void CharacterAttackOff();

	void RMB_On();
	void RMB_Off();

	bool bIsRMBPressed;

	void JumpFixer();

#pragma endregion

	// ******************************************************************

	#pragma region Spell Preparing

	UPROPERTY(Replicated)
	int32 IsPreparingSpell;

	float Spell_1_ManaCost;
	float Spell_2_ManaCost;
	float Spell_3_ManaCost;
	float Spell_4_ManaCost;

	UStaticMeshComponent* PlayerComp_LineSpell;
	UStaticMeshComponent* PlayerComp_CircleSpell;
	UStaticMeshComponent* PlayerComp_ConeCollider;

#pragma endregion

	// ******************************************************************

	#pragma region Mage Movement and Camera

	UPROPERTY(EditAnywhere, Category = "Mage Movement & Camera")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Mage Movement & Camera")
	float TurningSpeed;

	UPROPERTY(EditAnywhere, Category = "Mage Movement & Camera")
	float ClampingAngle;

	UPROPERTY(Replicated)
	bool bIsDead;

	UPROPERTY(Replicated)
	float MoveDirX;

	UPROPERTY(Replicated)
	float MoveDirY;

#pragma endregion

	// ******************************************************************
	
	#pragma region Camera Manipulation

	UPROPERTY(EditAnywhere, Category = "Mage Attack")
		float MagicCircle_Z_Clamp_Min;
	UPROPERTY(EditAnywhere, Category = "Mage Attack")
		float MagicCircle_Z_Clamp_Max;
	UPROPERTY(EditAnywhere, Category = "Mage Attack")
		float PitchOnCircleMode;

	int32 CameraCircleGoingUpOrDown;

#pragma endregion

	// ******************************************************************

	#pragma region Cooldowns and Timers

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float ActiveCD_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float ActiveCD_2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float ActiveCD_3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float ActiveCD_4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		bool bIsCDMessage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		bool bIsManaMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float CDMessageTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float ManaMessageTimer;

#pragma endregion
	
	// ******************************************************************

	#pragma region Health and Mana (UI)

	UPROPERTY(Replicated)
		float CurrentHealth;

		float CurrentMana;

	UPROPERTY(EditAnywhere, Category = "Health & Mana")
		float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Health & Mana")
		float MaxMana;

	void InitializeGameplayStats();

	void UpdateHealth(float Value);
	void UpdateMana(float Value);

#pragma endregion

	// ******************************************************************

	#pragma region Spellcasting

	void SetSpellName(int32 Slot, FString Name);

	FString Spell_1_Name;
	FString Spell_2_Name;
	FString Spell_3_Name;
	FString Spell_4_Name;

	UPROPERTY(Replicated)
	TArray<UActorComponent*> MySpells;

	void RenderSpellPrepGraphics(int32 Slot, bool bShouldRender);

	void InvokeSpell(EAvailableSpells Spell);

	void StartPlayingCastAnimation(int32 Slot);
	void InstaExecute(int32 Slot);

	UFUNCTION(BlueprintCallable, Category = "Mage Attack")
		void StartCasting();

	void ReplicateSpellEffects(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);

	void ResetMageBlend();

	UPROPERTY(Replicated)
	bool bBlockAttacksTemp;

	bool bIsManipulatingCircle;

	bool bSpellCast = false;
	float SpellCast_Timer;
	float SpellCast_CD = 0.2f;

#pragma endregion

	// ******************************************************************

	#pragma region Collision Related

	UFUNCTION()
		void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	float AOETimer;
	float AOECD = 0.5f;
	bool bIsInAOE;

	bool IceTrapped;

#pragma endregion

	// ******************************************************************

	#pragma region Useful References

	UMageAnimInstance* PlayerMageAnimation;

	AMageMastermind* MagePlayerController;
	
	UMageAnimInstance* GetAnimationInstance();
	AMageMastermind* GetMagePlayerController();

#pragma endregion

	// ******************************************************************

	#pragma region Combat Functions

	void TakeDamageFromEnemy(float Damage);

	void Die();

	void AddForceEffect(FVector Power);

	void AddBuffToCharacter(TEnumAsByte<EBuffs> BuffToApply, float Duration);

	float JumpResetterTimer;
	float JumpZ_Init;
	
#pragma endregion
	
	// ******************************************************************

#pragma region Audio Related


#pragma endregion


	// ******************* NETWORKING FUNCTIONS *************************** //

	#pragma region Networking Functions

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_TakeDamageFromEnemy(float Damage);
		void Server_TakeDamageFromEnemy_Implementation(float Damage);
		bool Server_TakeDamageFromEnemy_Validate(float Damage);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_TakeDamageFromEnemy(float Damage);
		void Multicast_TakeDamageFromEnemy_Implementation(float Damage);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Die();
		void Server_Die_Implementation();
		bool Server_Die_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_Die();
		void Multicast_Die_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_StartCasting();
		void Server_StartCasting_Implementation();
		bool Server_StartCasting_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_StartCasting();
		void Multicast_StartCasting_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ReplicateSpellEffects(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);
		void Server_ReplicateSpellEffects_Implementation(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);
		bool Server_ReplicateSpellEffects_Validate(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ReplicateSpellEffects(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);
		void Multicast_ReplicateSpellEffects_Implementation(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Mage Attack")
		void Server_ResetMageBlend();
		void Server_ResetMageBlend_Implementation();
		bool Server_ResetMageBlend_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ResetMageBlend();
		void Multicast_ResetMageBlend_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Mage Attack")
		void Server_CalculateDamageFromProj();
		void Server_CalculateDamageFromProj_Implementation();
		bool Server_CalculateDamageFromProj_Validate();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Mage Attack")
		void Server_ShootProjectile(float FinalDamage);
		void Server_ShootProjectile_Implementation(float FinalDamage);
		bool Server_ShootProjectile_Validate(float FinalDamage);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShootProjectile(float FinalDamage);
		void Multicast_ShootProjectile_Implementation(float FinalDamage);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_InvokeSpell(EAvailableSpells Spell);
		void Server_InvokeSpell_Implementation(EAvailableSpells Spell);
		bool Server_InvokeSpell_Validate(EAvailableSpells Spell);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_InvokeSpell(EAvailableSpells Spell);
		void Multicast_InvokeSpell_Implementation(EAvailableSpells Spell);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterMoveForward(float AxisValue);
		void Server_CharacterMoveForward_Implementation(float AxisValue);
		bool Server_CharacterMoveForward_Validate(float AxisValue);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterMoveForward(float AxisValue);
	void Multicast_CharacterMoveForward_Implementation(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterMoveRight(float AxisValue);
		void Server_CharacterMoveRight_Implementation(float AxisValue);
		bool Server_CharacterMoveRight_Validate(float AxisValue);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterMoveRight(float AxisValue);
		void Multicast_CharacterMoveRight_Implementation(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterJump();
		void Server_CharacterJump_Implementation();
		bool Server_CharacterJump_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterJump();
		void Multicast_CharacterJump_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterAttackOn();
		void Server_CharacterAttackOn_Implementation();
		bool Server_CharacterAttackOn_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterAttackOn();
		void Multicast_CharacterAttackOn_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterAttackOff();
		void Server_CharacterAttackOff_Implementation();
		bool Server_CharacterAttackOff_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterAttackOff();
		void Multicast_CharacterAttackOff_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_JumpFixer();
		void Server_JumpFixer_Implementation();
		bool Server_JumpFixer_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_JumpFixer();
		void Multicast_JumpFixer_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ExecuteSpell_1();
		void Server_ExecuteSpell_1_Implementation();
		bool Server_ExecuteSpell_1_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ExecuteSpell_1();
		void Multicast_ExecuteSpell_1_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ExecuteSpell_2();
		void Server_ExecuteSpell_2_Implementation();
		bool Server_ExecuteSpell_2_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ExecuteSpell_2();
		void Multicast_ExecuteSpell_2_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ExecuteSpell_3();
		void Server_ExecuteSpell_3_Implementation();
		bool Server_ExecuteSpell_3_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ExecuteSpell_3();
		void Multicast_ExecuteSpell_3_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ExecuteSpell_4();
		void Server_ExecuteSpell_4_Implementation();
		bool Server_ExecuteSpell_4_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ExecuteSpell_4();
		void Multicast_ExecuteSpell_4_Implementation();


	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PrepareSpell_1();
		void Server_PrepareSpell_1_Implementation();
		bool Server_PrepareSpell_1_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PrepareSpell_1();
		void Multicast_PrepareSpell_1_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PrepareSpell_2();
		void Server_PrepareSpell_2_Implementation();
		bool Server_PrepareSpell_2_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PrepareSpell_2();
		void Multicast_PrepareSpell_2_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PrepareSpell_3();
		void Server_PrepareSpell_3_Implementation();
		bool Server_PrepareSpell_3_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PrepareSpell_3();
		void Multicast_PrepareSpell_3_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PrepareSpell_4();
		void Server_PrepareSpell_4_Implementation();
		bool Server_PrepareSpell_4_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PrepareSpell_4();
		void Multicast_PrepareSpell_4_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlayReplicatedSpellAnimation(int32 AnimationID, APlayerMageCharacter* Mage);
		void Server_PlayReplicatedSpellAnimation_Implementation(int32 AnimationID, APlayerMageCharacter* Mage);
		bool Server_PlayReplicatedSpellAnimation_Validate(int32 AnimationID, APlayerMageCharacter* Mage);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_PlayReplicatedSpellAnimation(int32 AnimationID, APlayerMageCharacter* Mage);
		void Multicast_PlayReplicatedSpellAnimation_Implementation(int32 AnimationID, APlayerMageCharacter* Mage);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CharacterMouseLookUpDown(float AxisValue);
		void Server_CharacterMouseLookUpDown_Implementation(float AxisValue);
		bool Server_CharacterMouseLookUpDown_Validate(float AxisValue);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_CharacterMouseLookUpDown(float AxisValue);
		void Multicast_CharacterMouseLookUpDown_Implementation(float AxisValue);

#pragma endregion

};
