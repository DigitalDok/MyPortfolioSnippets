// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "MonsterAnimInstance.generated.h"



/**
 * 
 */

UCLASS()
class MONSTERCOMBAT_API UMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation C++ Code")
		void AttackAnim();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation C++ Code")
		void GetHurt();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation C++ Code")
		void AbilityA();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation C++ Code")
		void AbilityB();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation C++ Code")
		void AbilityC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation C++ Code")
		bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation C++ Code")
		bool bIsDead;
	
	UFUNCTION(BlueprintCallable, Category = "Animation C++ Code")
		void ApplyDamageToTargetActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation C++ Code")
		AActor* TargetActor;
	
};
