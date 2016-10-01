// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "BT_CollisionWithFriend.generated.h"

/**
 * 
 */
UCLASS()
class EQS_DEMO_API UBT_CollisionWithFriend : public UBTDecorator
{
	GENERATED_BODY()
	
	
		virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	

};
