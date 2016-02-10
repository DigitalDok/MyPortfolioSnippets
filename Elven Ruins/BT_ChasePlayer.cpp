
///////////////////////////////////////////////
// Custom Chasing Player Task - Kostas Dokos //
///////////////////////////////////////////////

#include "PirateLand.h"
#include "AI/Navigation/NavigationPath.h"
#include "MMO_Character.h"
#include "BT_ChasePlayer.h"


UBT_ChasePlayer::UBT_ChasePlayer()
{
	bCreateNodeInstance = true;
	TimerDelegate = FTimerDelegate::CreateUObject(this, &UBT_ChasePlayer::OnTimerTick);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A Simple Move To Task won't make it. We want every task that the AI Executes to be interruptable.						   //
//																															   //
// This specifiec Move To Task is interrupted or restarted when the monster loses melee contact with the enemy,				   //
// when the path is not valid, or when the distance between them is greater than 100 yards.									   //
// 																															   //
// Our AI Supports Aggro Reset, so it's also interrupted when the Monster itself has a great distance from his home location.  //
// 																															   //
// The Interruption events are implemented using Timers and Delegates.														   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EBTNodeResult::Type UBT_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	MyOwnerComp = &OwnerComp;

	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(OwnerComp.GetOwner());
	AMMO_Mob_Character* Monster = Cast<AMMO_Mob_Character>(AIController->GetCharacter());

	MyMonster = Monster;
	MyController = AIController;

	if (Monster->MyCurrentTarget != nullptr)
	{
		//if (FVector::Dist(Monster->GetActorLocation(), Monster->MyCurrentTarget->GetActorLocation()) <= 50)
		if(Monster->EnemyInMeleeRange!=nullptr)
		if(Monster->EnemyInMeleeRange == Monster->MyCurrentTarget)
		{
			return EBTNodeResult::Succeeded;
		}

		UNavigationSystem::SimpleMoveToActor(AIController, Monster->MyCurrentTarget);
		Monster->bHasFinishedPath = false;		

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);

		return EBTNodeResult::InProgress;
	}
	else
	{
		return EBTNodeResult::Failed;
	}

}

void UBT_ChasePlayer::OnTimerTick()
{
	UNavigationPath* Path = UNavigationSystem::FindPathToActorSynchronously(GetWorld(), MyMonster->GetActorLocation(), MyMonster->MyCurrentTarget);
	bool ValidPath = false;

	if (Path)
	{
		if (Path->GetPath().Get())
		{
			if (Path->GetPath().Get()->IsValid())
			{
				ValidPath = true;
			}
		}
	}

	if (ValidPath)
	{
		if (MyMonster->EnemyInMeleeRange != nullptr)
		{
			if (MyMonster->EnemyInMeleeRange == MyMonster->MyCurrentTarget)
			{
				UNavigationSystem::SimpleMoveToLocation(MyController, MyMonster->GetActorLocation());
				MyMonster->bHasFinishedPath = true;

				FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
			}
			else
			{
				UNavigationSystem::SimpleMoveToActor(MyController, MyMonster->MyCurrentTarget);
				MyMonster->bHasFinishedPath = false;

				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
				MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);

				FinishLatentTask(*MyOwnerComp, EBTNodeResult::InProgress);
			}
		}
		else if (FVector::Dist(MyMonster->GetActorLocation(), MyMonster->MyCurrentTarget->GetActorLocation()) > 100)
		{

			if (FVector::Dist(MyMonster->GetActorLocation(), MyController->BlackboardComp->GetValueAsVector(TEXT("HomeLocation"))) > MyController->BlackboardComp->GetValueAsFloat(TEXT("RangeUntilReset")))
			{
				FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
			}
			else
			{
				UNavigationSystem::SimpleMoveToActor(MyController, MyMonster->MyCurrentTarget);
				MyMonster->bHasFinishedPath = false;

				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
				MyController->GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);

				FinishLatentTask(*MyOwnerComp, EBTNodeResult::InProgress);
			}
		}
	}
	else
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
	}
}

