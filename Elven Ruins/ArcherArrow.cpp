
////////////////////////////////////////////
// Archer Arrow Logic - Kostas Dokos	  //
////////////////////////////////////////////

#include "PirateLand.h"
#include "MMO_Mob_Character.h"
#include "ArcherArrow.h"



AArcherArrow::AArcherArrow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AArcherArrow::BeginPlay()
{
	Super::BeginPlay();
	
	ColliderComponent = FindComponentByClass<class UBoxComponent>();
	PhysicsComponent = FindComponentByClass<class UStaticMeshComponent>();

	bHasDamaged = false;

	ColliderComponent->OnComponentBeginOverlap.AddDynamic(this, &AArcherArrow::OnOverlapBegin);
}

void AArcherArrow::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

////////////////////////////////////////////////////////////////////////////////////////////////
// When our Arrow collides with something we make sure that it hasn't applied damage before,  //
// the collided object is a monster or a boss and then apply the damage via the server.		  //
////////////////////////////////////////////////////////////////////////////////////////////////

void AArcherArrow::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (MyArcher && !bHasDamaged)
	{
		if (Cast<AMMO_Mob_Character>(OtherActor) && !bHasDamaged)
		{
			
			Cast<AMMO_Mob_Character>(OtherActor)->Server_TakeDamageFromHero(MyArcher->FinalArrowDamage, Cast<AActor>(MyArcher));

			if (Cast<AMMO_Mob_Character>(OtherActor)->bIsDead)
			{
				for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					if (ActorItr->LockedTarget == OtherActor)
					{
						ActorItr->ServerLockOn();
					}
				}
				MyArcher->ShutDownColliders(Cast<AMMO_Mob_Character>(OtherActor));
			}

		}
		else if (Cast<ABoss>(OtherActor) && !bHasDamaged)
		{
			Cast<ABoss>(OtherActor)->Server_TakeDamageFromHero(MyArcher->FinalArrowDamage, MyArcher);

			if (Cast<ABoss>(OtherActor)->bIsDead)
			{
				for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					if (ActorItr->LockedTarget == OtherActor)
					{
						ActorItr->ServerLockOn();
					}
				}
			}
		}

		if (OtherActor != GetOwner())
		{
			if (!Cast<AMMO_Character>(OtherActor) && OtherActor->GetName() != TEXT("Trigger"))
			{
				bHasDamaged = true;
			}
		}
		
	}
}

////////////////////////////////////////////////////////
// Basic Force Applied to Arrow when initially shot.  //
////////////////////////////////////////////////////////

void AArcherArrow::ApplyForce(float Force, AMMO_Character* Archer, FVector Direction)
{
	MyArcher = Archer;
	PhysicsComponent->AddForce(Direction*Force, "",true);
}

#pragma region Networking Helper Functions

void AArcherArrow::Server_ApplyForce_Implementation(float Force, AMMO_Character* Archer, FVector Direction)
{
	Multicast_ApplyForce(Force,Archer,Direction);
}

bool AArcherArrow::Server_ApplyForce_Validate(float Force, AMMO_Character* Archer, FVector Direction)
{
	return true;
}

void AArcherArrow::Multicast_ApplyForce_Implementation(float Force, AMMO_Character* Archer, FVector Direction)
{
	ApplyForce(Force, Archer, Direction);
}

#pragma endregion

