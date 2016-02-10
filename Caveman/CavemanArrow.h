// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CavemanArrow.generated.h"

UCLASS()
class CAVEMANTESTFIELD_API ACavemanArrow : public AActor
{
	GENERATED_BODY()
	
public:

	// ------- Core Functions ------------- //

	ACavemanArrow();

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	// ----- Collision Component ------ //
	UBoxComponent* MyColliderComp;

	UStaticMeshComponent* MyComp;

	// --- Control Variable that stops damage on First Collision. --- //
	bool HasDamaged;

	UFUNCTION()
		void OnArrowBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
		void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void DamageGivenEnemy(AActor* HitEnemy);

};
