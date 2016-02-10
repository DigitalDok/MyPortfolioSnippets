// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "MMO_Mob_AI_Controller.h"
#include "MMO_Mob_Character.h"
#include "BT_AttackMelee.generated.h"

/**
 * 
 */
UCLASS()
class PIRATELAND_API UBT_AttackMelee : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBT_AttackMelee();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UBehaviorTreeComponent* MyOwnerComp;

	AMMO_Mob_AI_Controller* MyController;
	AMMO_Mob_Character* MyMonster;

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	void OnAnimationTimerDone();

	FTimerHandle CheckerTimerHandle;
	FTimerDelegate CheckerTimerDelegate;
	void OnCheckTimer();

	UPROPERTY(EditAnywhere,Category="AI")
	float AttackSpeed;
	
};
