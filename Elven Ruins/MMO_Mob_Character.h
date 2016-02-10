
//////////////////////////////////////
// Monster Character - Kostas Dokos //
//////////////////////////////////////

#pragma once

#include "GameFramework/Character.h"
#include "WidgetComponent.h"
#include "MMO_Mob_AnimInstance.h"
#include "MMO_DamageOverlay_HUD.h"
#include "MMO_OverheadUI_HUD.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MMO_Mob_AnimInstance.h"
#include "MMO_Mob_AI_Controller.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/PawnSensingComponent.h"
#include "ObjectPool.h"
#include "MMO_Mob_Character.generated.h"

UCLASS()
class PIRATELAND_API AMMO_Mob_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AMMO_Mob_Character();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	// ********************** COMPONENTS ********************************** //

	#pragma region Components

	UWidgetComponent* MyWidgetComp;

	UMMO_DamageOverlay_HUD* MyDamageOverlay;
	
	UMMO_OverheadUI_HUD* MyHealthBar;

	UMMO_OverheadUI_HUD* MyLockOnWidget;

	UBoxComponent* MyColliderComp;

	USkeletalMeshComponent* Monster_SkeletalMesh;

	AMMO_Mob_AI_Controller* MyController;

	#pragma endregion

#pragma region Tick Helpers

	void UpdateLocationOfMonster();

	void AttackingLogic();

	void UI_Updater();

#pragma endregion



	// ****************** COLLISIONS *********************************** //

	#pragma region Collision Functions

	/** called when something enters the sphere component */
	UFUNCTION()
		void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something leaves the sphere component */
	UFUNCTION()
		void OnOverlapEnd(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	#pragma endregion

	// ********************** ANIMATIONS ********************************** //

	#pragma region Animation Related

	UMMO_Mob_AnimInstance* MonsterAnimInstance;

	void AnimFixer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* MeleeAttack;

	#pragma endregion

	// ********************** MONSTER STATS ********************************** //

	#pragma region Monster Core Stats

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Monster Custom Stats")
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats")
		float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats")
		float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats")
		float AttackBonusMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats")
		float AttackBonusMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats")
		float DamageReduction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Custom Stats", Replicated)
		bool bIsDead;

	#pragma endregion

	// ********************** COMBAT FUNCTIONS ********************************** //
	
	#pragma region Combat Related Functions and Properties

	void AttackHero();

	void Die(AActor* DamageCauser);
		
	FVector prevBase;
	FVector prevTip;

	UPROPERTY(Replicated)
	bool bHasFinishedPath=false;

	UPROPERTY(Replicated)
	AActor* EnemyInMeleeRange;

	UPROPERTY(Replicated)
	bool bIsImmunized;

	#pragma endregion

	// ********************** AI RELATED ************************ //

	#pragma region AI Related | Properties used by Blackboard

	UFUNCTION()
		void OnSeePlayer(APawn* Pawn);

	/*The Component which is used for the "seeing" sense of the AI*/
	UPROPERTY(VisibleAnywhere, Category = "AI")
		UPawnSensingComponent* PawnSensingComp;

	/*The Behavior Tree of the Character*/
	UPROPERTY(EditAnywhere, Category = "AI")
		UBehaviorTree* BehaviorTree;

	/* The Current Target of the Monster, If it has seen someone.*/
	AActor* MyCurrentTarget;
	
	/* Control Variable Indicating Aggro*/
	UPROPERTY(EditAnywhere, Category = "AI")
		bool bHasAggro;

	UPROPERTY(EditAnywhere, Category = "AI")
		 float RangeUntilReset;

	UPROPERTY(EditAnywhere, Category = "AI")
		 float PatrolRadius;

	void MoveQuick(bool bFast);

	UPROPERTY(EditAnywhere, Category="AI")
		float AggroRadius;

	void AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);
	void TakeDamageFromHero(int32 Damage, AActor* DamageCauser);

	#pragma endregion


	#pragma region Networking Definitions

	UFUNCTION(Server, Reliable, WithValidation)
		void ShutDownColliders();
		void ShutDownColliders_Implementation();
		bool ShutDownColliders_Validate();
		
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttackAnim();
		void Server_AttackAnim_Implementation();
		bool Server_AttackAnim_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_AttackAnim();
		void Multicast_AttackAnim_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ShutDownColliders();
		void Multicast_ShutDownColliders_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerImmunize();
		void ServerImmunize_Implementation();
		bool ServerImmunize_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastImmunize();
		void MulticastImmunize_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDeImmunize();
		void ServerDeImmunize_Implementation();
		bool ServerDeImmunize_Validate();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastDeImmunize();
		void MulticastDeImmunize_Implementation();
	
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_MoveQuick(bool bFast);
		void Server_MoveQuick_Implementation(bool bFast);
		bool Server_MoveQuick_Validate(bool bFast);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_MoveQuick(bool bFast);
		void Multicast_MoveQuick_Implementation(bool bFast);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);
		void Server_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target);
		bool Server_AggroTarget_Validate(AMMO_Mob_Character* Monster, AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_AggroTarget(AMMO_Mob_Character* Monster, AActor* Target);
		void Multicast_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_TakeDamageFromHero(int32 Damage, AActor* DamageCauser);
		void Server_TakeDamageFromHero_Implementation(int32 Damage, AActor* DamageCauser);
		bool Server_TakeDamageFromHero_Validate(int32 Damage, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_TakeDamageFromHero(int32 Damage, AActor* DamageCauser);
		void Multicast_TakeDamageFromHero_Implementation(int32 Damage, AActor* DamageCauser);

	#pragma endregion

};
