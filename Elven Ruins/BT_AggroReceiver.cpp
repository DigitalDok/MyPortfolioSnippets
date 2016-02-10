
///////////////////////////////////////////
// Aggro Receiver Service - Kostas Dokos //
///////////////////////////////////////////

#include "PirateLand.h"
#include "MMO_Mob_AI_Controller.h"
#include "MMO_Mob_Character.h"
#include "MMO_Character.h"
#include "BT_AggroReceiver.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////
// When too many properties and conditions affect your current target as well as your AGGRO status, //
// you want a Service to feed that data back to the monster 										//
//																									//
// The Aggro Receiver Service takes under consideration	the living state of the current target if 	//
// there is one, else it's on constant vigilance for new targets to feed back to the monster.		//
//////////////////////////////////////////////////////////////////////////////////////////////////////

/** update next tick interval
* this function should be considered as const (don't modify state of object) if node is not instanced! */
void UBT_AggroReceiver::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(OwnerComp.GetOwner());
	AMMO_Mob_Character* Monster = Cast<AMMO_Mob_Character>(AIController->GetCharacter());

	TArray<AMMO_Character*> Enemies;
	for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (!ActorItr->bIsDead)
			Enemies.Add(*ActorItr);
	}
	int32 Index = -1;
	float Distance = 100000000;
	for (size_t i = 0; i < Enemies.Num(); i++)
	{
		if (FVector::Dist(Monster->GetActorLocation(), Enemies[i]->GetActorLocation()) <= AIController->BlackboardComp->GetValueAsFloat(TEXT("AggroRadius")))
		{
			if (FVector::Dist(Monster->GetActorLocation(), Enemies[i]->GetActorLocation()) < Distance)
			{
				if (!Enemies[i]->bIsDead && !Enemies[i]->AnimInstance->bHasDied && Enemies[i]->CurrentHealth>0)
				{
					Distance = FVector::Dist(Monster->GetActorLocation(), Enemies[i]->GetActorLocation());
					Index = i;
				}
			}
		}
	}

	if (Cast<AMMO_Character>(Monster->MyCurrentTarget))
	{
		int32 Rand = FMath::RandRange(0, 1000);
		if(Rand > 990)
		{
			Cast<AMMO_Character>(Monster->MyCurrentTarget)->Multicast_PlaySound(
				Cast<AMMO_Character>(Monster->MyCurrentTarget)->GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_GoblinAggro),
				Monster->GetActorLocation());
		}

		if (Cast<AMMO_Character>(Monster->MyCurrentTarget)->CurrentHealth <= 0)
		{
			if (Index != -1)
			{
				AIController->SetSeenTarget(Enemies[Index]);
				Monster->MyCurrentTarget = Cast<AActor>(Enemies[Index]);
				
			}
			else
			{
				AIController->SetSeenTarget(nullptr);
				Monster->MyCurrentTarget = nullptr;
			}
		}
	}
	else if (Monster->MyCurrentTarget == nullptr)
	{
			if (Index != -1)
			{
				AIController->SetSeenTarget(Enemies[Index]);
				Monster->MyCurrentTarget = Cast<AActor>(Enemies[Index]);
			}
			else
			{
				AIController->SetSeenTarget(nullptr);
				Monster->MyCurrentTarget = nullptr;
			}
	}
		
		
	
}
