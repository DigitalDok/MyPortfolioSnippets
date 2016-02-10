// Fill out your copyright notice in the Description page of Project Settings.

#include "PirateLand.h"
#include "BT_InterruptableMoveTo.h"


UBT_InterruptableMoveTo::UBT_InterruptableMoveTo()
{
	bCreateNodeInstance = true;
	TimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_InterruptableMoveTo::OnTimerTick);
}

EBTNodeResult::Type UBT_InterruptableMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	MyOwnerComp = &OwnerComp;

	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(OwnerComp.GetOwner());
	AMMO_Mob_Character* Monster = Cast<AMMO_Mob_Character>(AIController->GetCharacter());

	MyMonster = Monster;
	MyController = AIController;

	UNavigationSystem::SimpleMoveToLocation(AIController, AIController->BlackboardComp->GetValueAsVector(TEXT("RandomizedPatrolLocation")));
	Monster->bHasFinishedPath = false;

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.2f, false);

	return EBTNodeResult::InProgress;
}

void UBT_InterruptableMoveTo::OnTimerTick()
{
	if (MyMonster->MyCurrentTarget != nullptr)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
	}
	else if (MyMonster->bHasFinishedPath)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
	}

	FinishLatentTask(*MyOwnerComp, EBTNodeResult::InProgress);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.2f, false);
}