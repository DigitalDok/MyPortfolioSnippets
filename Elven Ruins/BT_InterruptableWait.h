// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "MMO_Mob_AI_Controller.h"
#include "MMO_Mob_Character.h"
#include "BT_InterruptableWait.generated.h"

/**
 * 
 */
UCLASS()
class PIRATELAND_API UBT_InterruptableWait : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBT_InterruptableWait();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UBehaviorTreeComponent* MyOwnerComp;

	UPROPERTY(EditAnywhere, Category = "AI Related")
		float TimeRemaining;

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	void OnTimerTick();

	FTimerHandle FinalTimerHandle;
	FTimerDelegate FinalTimerDelegate;
	void OnFinalTimerTick();

	AMMO_Mob_AI_Controller* MyController;

	AMMO_Mob_Character* MyMonster;

	
	
};
