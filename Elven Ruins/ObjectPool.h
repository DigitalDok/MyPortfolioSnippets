// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ObjectPool.generated.h"

UENUM()
enum ESoundEffectLibrary
{
	SFX_PotionDrink,
	SFX_ArrowHit,
	SFX_SwordHit,
	SFX_DaggerHit,
	SFX_BossHit,
	SFX_GoblinAggro
};

UCLASS()
class PIRATELAND_API AObjectPool : public AActor
{
	GENERATED_BODY()
	
public:	
	AObjectPool();
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		int32 MaxSFX;

	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		TArray<AActor*> SFXList;

	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		TSubclassOf<AActor> SFXPlayer;

	void InitializePool();

	UFUNCTION()
		void PutObjectBackToList();

	TScriptDelegate<FWeakObjectPtr> AudioFinishDelegate;

	int32 GetIndexOfFirstAvailableObject();


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// As a Programmer, all I want to do is play a sound or spawn a particle, and don't care about anything else. //
	// So, these are the only exposed functions from the object pooler.											  //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	UPROPERTY(EditAnywhere, Category = "Generic Sound Effect Library")
		TArray<TEnumAsByte<ESoundEffectLibrary>> SoundEffects_Part_1;

	UPROPERTY(EditAnywhere, Category = "Generic Sound Effect Library")
		TArray<USoundBase*> SoundEffects_Part_2;

	TMap<TEnumAsByte<ESoundEffectLibrary>, USoundBase*> GenericSoundEffects;

	void PlaySound2D(USoundBase* AudioClip);

	void PlaySound3D(USoundBase* AudioClip, FVector Location, float Radius = 100.f, float FallOff = 3600.f );

};
