// Fill out your copyright notice in the Description page of Project Settings.

#include "PirateLand.h"
#include "BT_InterruptableWait.h"


UBT_InterruptableWait::UBT_InterruptableWait()
{
	bCreateNodeInstance = true;

	TimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_InterruptableWait::OnTimerTick);
	FinalTimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_InterruptableWait::OnFinalTimerTick);
}

EBTNodeResult::Type UBT_InterruptableWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	MyOwnerComp = &OwnerComp;

	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(OwnerComp.GetOwner());
	AMMO_Mob_Character* Monster = Cast<AMMO_Mob_Character>(AIController->GetCharacter());

	MyMonster = Monster;
	MyController = AIController;

	GetWorld()->GetTimerManager().ClearTimer(FinalTimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(FinalTimerHandle, FinalTimerDelegate, TimeRemaining, false);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.2f, false);

	return EBTNodeResult::InProgress;

}

void UBT_InterruptableWait::OnTimerTick()
{
	if (MyMonster->MyCurrentTarget != nullptr)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
	}

	FinishLatentTask(*MyOwnerComp, EBTNodeResult::InProgress);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.2f, false);
}

void UBT_InterruptableWait::OnFinalTimerTick()
{
	FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
}