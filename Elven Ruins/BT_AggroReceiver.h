// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTService.h"
#include "BT_AggroReceiver.generated.h"

/**
 * 
 */
UCLASS()
class PIRATELAND_API UBT_AggroReceiver : public UBTService
{
	GENERATED_BODY()
	
public:

	/** update next tick interval
	* this function should be considered as const (don't modify state of object) if node is not instanced! */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	
};
