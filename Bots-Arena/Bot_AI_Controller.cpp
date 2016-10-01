// Fill out your copyright notice in the Description page of Project Settings.

#include "EQS_Demo.h"
#include "Bot_AI_Controller.h"
#include "AI_Bot.h"

ABot_AI_Controller::ABot_AI_Controller()
{
	BB = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BT = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

void ABot_AI_Controller::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	//Get the possessed Pawn. If it's the AI Character we created
	//initialize it's blackboard and start it's corresponding behavior tree
	
	AAI_Bot* AI_Bot = Cast<AAI_Bot>(Pawn);
	if (AI_Bot)
	{
		if (AI_Bot->BehaviorTree->BlackboardAsset)
		{
			BB->InitializeBlackboard(*(AI_Bot->BehaviorTree->BlackboardAsset));
			BT->StartTree(*AI_Bot->BehaviorTree);
/*
			BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), AICharacter->GetActorLocation());
			BlackboardComp->SetValueAsFloat(TEXT("RangeUntilReset"), AICharacter->RangeUntilReset);
			BlackboardComp->SetValueAsFloat(TEXT("PatrolRadius"), AICharacter->PatrolRadius);
			BlackboardComp->SetValueAsFloat(TEXT("AggroRadius"), AICharacter->AggroRadius);*/
		}
	}
}

