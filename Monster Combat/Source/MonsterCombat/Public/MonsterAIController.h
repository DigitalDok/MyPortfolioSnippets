// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "MonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERCOMBAT_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
	
public:

	virtual void BeginPlay() override;

	UFUNCTION()
		void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult);

	ACharacter* MyChar;
	
	void DecideNextAction(APlayerController* &ControllerFromMaster);
	bool bHasActionBeenDecidedYet;
	
	/// ---------------------- AI PRIORITIES LIST ----------------------------- ///

	void ResurrectMyTeamMatePriority(ACombatController* &Con);
	void KeepMyHPUpPriority(ACombatController* &Con);
	
	void PartyHPPriorityLow(ACombatController* &Con);
	void PartyHPPriorityHigh(ACombatController* &Con);
	
	void SelfMPPriority(ACombatController* &Con);
	
	void AnyoneMPPriority(ACombatController* &Con);
	void AnyoneHPPriority(ACombatController* &Con);

	int32 DecideForSpellToCast(ACombatController* &Con);

	void PerformDefend(ACombatController* &Con);
	void PerformAttack(ACombatController* &Con, int32 TargetID);
	
};
