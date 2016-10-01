// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Bot_AI_Controller.generated.h"

/**
 * 
 */
UCLASS()
class EQS_DEMO_API ABot_AI_Controller : public AAIController
{
	GENERATED_BODY()
	
public:

	ABot_AI_Controller();

	/*Executes right when the controller possess a Pawn*/
	virtual void Possess(APawn* Pawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI - Master")
		UBlackboardComponent* BB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI - Master")
		UBehaviorTreeComponent* BT;


	
	
};
