// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Bot_AI_Controller.h"
#include "Bot_AnimInstance.h"
#include "AI_Bot.generated.h"

UENUM(BlueprintType)
enum EClassArchetypes
{
	Warrior,
	Tactician,
	Guard,
	Berserker
};

UCLASS()
class EQS_DEMO_API AAI_Bot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Bot();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	ABot_AI_Controller* Master;
	
	UBot_AnimInstance* MyAnimInstance;

	/*The Behavior Tree of the Character*/
	UPROPERTY(EditAnywhere, Category = "AI - Master")
		UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		bool bIsRedTeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		bool bCanTargetEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		float ShootingCD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		AAI_Bot* EnemyToShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		AAI_Bot* FavoredTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		TArray<AAI_Bot*> BotsTargetedMe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		float RadarCooldown;
	float CurrentRadarCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		TEnumAsByte<EClassArchetypes> MyArchetype;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		FVector AwesomePos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float AggroRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float DamagePerBullet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float DamageVariance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
		float EscapeRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
	float InternalFearCD;
	float InternalFearTimer = 0;
	bool bWillCountFear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int Fearless; // Determines chance to be feared by being closer to death

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int Strategy; // Determines chance to prefer weaker enemies to closer ones

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int FearlessMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int FearlessMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int StrategyMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
		int StrategyMax;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int Marksmanship; // Determines Aim (Chance To Hit)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int Mobility; // Determines Movement Speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int Stamina; // Determines Bonus Health

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int Luck; // Determines Chance for Critical Hit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int Elusiveness; // Determines Chance to avoid damage while moving

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skills")
		int GlobalHeadShotChance; // Determines Chance for Headshot

	//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int MarksmanshipMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int MarksmanshipMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int MobilityMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int MobilityMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int StaminaMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int StaminaMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int LuckMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int LuckMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int ElusivenessMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Skill Ranges")
		int ElusivenessMax;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Media")
		USoundBase* GunshotSfx;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Media")
		USoundBase* DeathSfx;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		UParticleSystemComponent* Muzzle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		UParticleSystemComponent* Blood;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		UParticleSystemComponent* Fear;


	bool bShouldTurnOffParticle;

		float InternalParticleTimer;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		bool bIsAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		FVector HidingSpot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		float ResetFearDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		float AggroCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		float DistanceToFriendly;


		float AggroCD_Cur;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Testing")
		bool bHasAggro;
	
	

	float CurrentShootingTimer;
	
	void GetEnemyTarget();
	void RegenHP(float DeltaTime);
	void RadarLogic(float DeltaTime);
	void RetargetingAdjustment();
	void AdjustBT_Values();
	void ShootingLogic(float DeltaTime);
	void FearLogic(float DeltaTime);

	int GetIndexOfEnemyWithLowestHealth(TArray<AAI_Bot*> EnemySet, FHitResult &Hit);
	int GetIndexOfEnemyWithLowestDistance(TArray<AAI_Bot*> EnemySet, FHitResult &Hit);
	void TargetingRules(TArray<AAI_Bot*> PossibleBots, int Index, FHitResult &Hit);

	int GetIndexOfEnemyWithLowestHealth(TArray<AAI_Bot*> EnemySet);
	int GetIndexOfEnemyWithLowestDistance(TArray<AAI_Bot*> EnemySet);
	void TargetingRules(TArray<AAI_Bot*> PossibleBots, int Index);

	FVector PosOfFriendWhoNeedsMeMost();
	void GetPossibleFavoredTarget();

	void TakeDamage(float Damage);
	void Die();

	
	void GetBotsTargetedMe();

	FRotator FindLookAtRotation(FVector Start, FVector Target);


};