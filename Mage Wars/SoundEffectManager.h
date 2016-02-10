// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "SoundEffectManager.generated.h"

UENUM(BlueprintType)
enum ESoundEffects
{
	ShootBaseProjectile,			// ID = 0
	BaseProjectileImpact,			// ID = 1
	MageGetHurt,                    // ID = 2
	MageDeath,                      // ID = 3
	MageJump,						// ID = 4
	IceTrail_Begin,					// ID = 5
	IceTrail_Shout,					// ID = 6
	IceTrail_Impact,				// ID = 7
	IceTrap_Begin,					// ID = 8
	IceTrap_Shout,					// ID = 9
	IceTrap_Impact,					// ID = 10
	FrostArmor_Begin,				// ID = 11
	FrostArmor_Shout,				// ID = 12
	GlacierShattering_Begin,		// ID = 13
	GlacierShattering_3D_Loop,		// ID = 14
	GlacierShattering_Shout,		// ID = 15
	Ability_OOM,                    // ID = 16
	Ability_NotReadyYet,            // ID = 17
	UI_GenericClick                 // ID = 18
	
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAGEWARS_API USoundEffectManager : public UActorComponent
{
	GENERATED_BODY()

public:
	USoundEffectManager();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


#pragma region Audio Related Functions

	// Plays a desired Sound Effect Based on the enum value given.
	void PlaySoundEffect(ESoundEffects SFX_To_Play);

	// This is the object that will be spawned in order to play a sound.

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Sound Effect Library")
	TSubclassOf<AActor> SoundPlayer3D;

	// Plays a desired 3d Sound Effect on a given location. 
	void PlaySoundEffect_3D_AtLocation(ESoundEffects SFX_To_Play, FVector Location);

	// Plays a desired 3d Sound Effect on a given location. 
	void PlaySoundEffect_3D(ESoundEffects SFX_To_Play);

	// Returns the Associated Sound Effect based on the enum value given.
	USoundWave* EnumToSoundEffect(ESoundEffects SFX_To_Play);

	// Called by the Delegate when the audio is finished to destroy this component.
	UFUNCTION()
		void  DestroyFinishedAudio();

#pragma endregion

#pragma region Audio Related Properties

	// The sound effects must be wired directly through blueprints.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effect Library")
		TArray<USoundWave*> SoundEffects;

	// The components that will be spawned are held in this list.
	TArray<UAudioComponent*> SoundEffectComponents;

	// This delegate runs when the audio is finished.
	TScriptDelegate<FWeakObjectPtr> AudioFinishDelegate;

#pragma endregion


};
