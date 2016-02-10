
/////////////////////////////////////////////////////
// Custom Monster Melee Attack Task - Kostas Dokos //
/////////////////////////////////////////////////////

#include "PirateLand.h"
#include "MMO_Character.h"
#include "BT_AttackMelee.h"


UBT_AttackMelee::UBT_AttackMelee()
{
	bCreateNodeInstance = true;
	TimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_AttackMelee::OnAnimationTimerDone);
	CheckerTimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_AttackMelee::OnCheckTimer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// As long as this task runs, it means that the monster is in a favorable melee position to attack our player. //
// 																											   //
// Like the movement itself, it's interruptable.															   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EBTNodeResult::Type UBT_AttackMelee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	MyOwnerComp = &OwnerComp;

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(OwnerComp.GetOwner());
	AMMO_Mob_Character* Monster = Cast<AMMO_Mob_Character>(AIController->GetCharacter());

	MyMonster = Monster;
	MyController = AIController;

	Monster->Server_AttackAnim();
	AIController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, AttackSpeed, false);

	GetWorld()->GetTimerManager().ClearTimer(CheckerTimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(CheckerTimerHandle, CheckerTimerDelegate, 0.1f, false);

	return EBTNodeResult::InProgress;
}

void UBT_AttackMelee::OnAnimationTimerDone()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
}

void UBT_AttackMelee::OnCheckTimer()
{
	if (MyMonster->EnemyInMeleeRange == nullptr)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
		return;
	}
	else if (MyMonster->MyCurrentTarget == nullptr)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(CheckerTimerHandle);
	MyController->GetWorld()->GetTimerManager().SetTimer(CheckerTimerHandle, CheckerTimerDelegate, 0.1f, false);

	FinishLatentTask(*MyOwnerComp, EBTNodeResult::InProgress);
}



