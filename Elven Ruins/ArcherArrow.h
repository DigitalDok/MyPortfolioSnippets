
////////////////////////////////////////////
// Archer Arrow Logic - Kostas Dokos	  //
////////////////////////////////////////////

#pragma once

#include "GameFramework/Actor.h"
#include "MMO_Character.h"
#include "ArcherArrow.generated.h"

UCLASS()
class PIRATELAND_API AArcherArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	AArcherArrow();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;


	UBoxComponent* ColliderComponent;
	UStaticMeshComponent* PhysicsComponent;
	AMMO_Character* MyArcher;

	UFUNCTION()
		void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// If this is true, then damage is already applied for this arrow, and further collisions will be ignored.
	bool bHasDamaged;

	int32 FinalArrowDamage;

	void ApplyForce(float Force, AMMO_Character* Archer, FVector Direction);

	#pragma region Networking

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ApplyForce(float Force, AMMO_Character* Archer, FVector Direction);
		void Server_ApplyForce_Implementation(float Force, AMMO_Character* Archer, FVector Direction);
		bool Server_ApplyForce_Validate(float Force, AMMO_Character* Archer, FVector Direction);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ApplyForce(float Force, AMMO_Character* Archer, FVector Direction);
		void Multicast_ApplyForce_Implementation(float Force, AMMO_Character* Archer, FVector Direction);

	#pragma endregion

	

	
};
