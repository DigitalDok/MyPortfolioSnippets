// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "MMO_Mob_AI_Controller.h"
#include "MMO_Mob_Character.h"
#include "BT_InterruptableMoveTo.generated.h"

/**
 * 
 */
UCLASS()
class PIRATELAND_API UBT_InterruptableMoveTo : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBT_InterruptableMoveTo();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "AI Related")
		FVector TargetLocation;

	UBehaviorTreeComponent* MyOwnerComp;
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	AMMO_Mob_AI_Controller* MyController;
	AMMO_Mob_Character* MyMonster;
	void OnTimerTick();
};
