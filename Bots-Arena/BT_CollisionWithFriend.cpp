// Fill out your copyright notice in the Description page of Project Settings.

#include "EQS_Demo.h"
#include "BT_CollisionWithFriend.h"
#include "MasterBot.h"
#include "Bot_AI_Controller.h"
#include "EngineUtils.h"



bool UBT_CollisionWithFriend::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
		
	AMasterBot* Master = Cast<AMasterBot>(GetWorld()->GetAuthGameMode());
	ABot_AI_Controller* AIController = Cast<ABot_AI_Controller>(OwnerComp.GetOwner());
	AAI_Bot* Bot = Cast<AAI_Bot>(AIController->GetCharacter());

	bool IsItReadTeam = Bot->bIsRedTeam;

	TArray<AAI_Bot*> Bots;
	if (IsItReadTeam) Bots = Master->RedTeam;
	else Bots = Master->BlueTeam;
	//
	for (size_t i = 0; i < Bots.Num(); i++)
	{
		if (FVector::Dist(Bot->GetActorLocation(), Bots[i]->GetActorLocation()) <= Bot->DistanceToFriendly)
		{
			return false;
		}
	}

	return true;
}