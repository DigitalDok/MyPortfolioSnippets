// Fill out your copyright notice in the Description page of Project Settings.

#include "CavemanTestField.h"
#include "CavemanGameMode.h"
#include "CavemanArrow.h"


#include "MyCharacter_FirstTry.h"
#include "EnemyStatsComponent.h"

// ***********************************************************************************************
// When the arrow collides with the enemy, damage is applied and then the arrow stops checking
// for collisions. The Static Mesh Component is used for component overlapping (Collision).
// ***********************************************************************************************

ACavemanArrow::ACavemanArrow()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACavemanArrow::BeginPlay()
{
	Super::BeginPlay();


	// --- Gain Reference to Components used for Collision and Physics --- //

	TArray<UBoxComponent*> Comps;
	GetComponents<UBoxComponent>(Comps);
	for (int32 i = 0; i < Comps.Num(); i++)
	{
		if (Comps[i]->ComponentHasTag("TheCollider"))
		{
			MyColliderComp = Comps[i];
		}
	}

	TArray<UStaticMeshComponent*> Comps2;
	GetComponents<UStaticMeshComponent>(Comps2);
	MyComp = Comps2[0];
		
	HasDamaged = false;
	MyColliderComp->OnComponentBeginOverlap.AddDynamic(this, &ACavemanArrow::OnArrowBeginOverlap);
	MyColliderComp->OnComponentHit.AddDynamic(this, &ACavemanArrow::OnHit);

	Cast<AMyCharacter_FirstTry>(GetWorld()->GetFirstPlayerController()->GetCharacter())->SFXManager->PlaySoundEffect(ESoundEffects::SFX_ArrowShot);
}

void ACavemanArrow::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	
}

// This is to ensure that arrows are disabled once they land. Prevents a bug where the wolf can walk over the arrow and get damaged.
void ACavemanArrow::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor->ActorHasTag("Wolf"))
	{
		HasDamaged = true;
	}
}

void ACavemanArrow::OnArrowBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

	if (OtherActor->ActorHasTag("Wolf") && !HasDamaged)
	{
		Cast<AMyCharacter_FirstTry>(GetWorld()->GetFirstPlayerController()->GetCharacter())->SFXManager->PlaySoundEffect(ESoundEffects::SFX_ArrowHit);
		DamageGivenEnemy(OtherActor);
	}


	HasDamaged = true;
}

void ACavemanArrow::DamageGivenEnemy(AActor* HitEnemy)
{
	if (HitEnemy)
	{
		AMyCharacter_FirstTry* PawnWhoPerformedTheHit = Cast<AMyCharacter_FirstTry>(GetWorld()->GetFirstPlayerController()->GetCharacter());

		TArray<UEnemyStatsComponent*> CamComps;
		HitEnemy->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[i]->ComponentHasTag("TheStatComp"))
			{
				CamComps[i]->TakeDamageFromEnemy(PawnWhoPerformedTheHit->CauseDamageByRanged(),PawnWhoPerformedTheHit);
				PawnWhoPerformedTheHit->SetCombatStatus(true);
			}
		}
	}
}