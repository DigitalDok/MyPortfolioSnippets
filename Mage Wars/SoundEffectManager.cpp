// Fill out your copyright notice in the Description page of Project Settings.

#include "MageWars.h"
#include "SoundEffectManager.h"


// Sets default values for this component's properties
USoundEffectManager::USoundEffectManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USoundEffectManager::BeginPlay()
{
	Super::BeginPlay();


	// Wiring up the delegate that will take care of destroying an audio comp when a sound has finished playing.
	AudioFinishDelegate.BindUFunction(this, "DestroyFinishedAudio");
}


// Called every frame
void USoundEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

USoundWave* USoundEffectManager::EnumToSoundEffect(ESoundEffects SFX_To_Play)
{
	return SoundEffects[SFX_To_Play];
}

void USoundEffectManager::PlaySoundEffect(ESoundEffects SFX_To_Play)
{
	UAudioComponent* Audio = NewObject<UAudioComponent>(this);
	Audio->RegisterComponent();

	Audio->OnAudioFinished.Add(AudioFinishDelegate);

	USoundWave* SoundEffect = EnumToSoundEffect(SFX_To_Play);

	Audio->Sound = SoundEffect;
	Audio->Play();

	SoundEffectComponents.Add(Audio);
}

void USoundEffectManager::PlaySoundEffect_3D_AtLocation(ESoundEffects SFX_To_Play, FVector Location)
{
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = GetOwner();
	spawnParams.Instigator = GetOwner()->Instigator;
	AActor* SpawnedObject;

	SpawnedObject = GetWorld()->SpawnActor<AActor>(SoundPlayer3D, Location, FRotator(0), spawnParams);

	UAudioComponent* Audio = SpawnedObject->FindComponentByClass<class UAudioComponent>();
	Audio->OnAudioFinished.Add(AudioFinishDelegate);

	USoundWave* SoundEffect = EnumToSoundEffect(SFX_To_Play);

	Audio->bAllowSpatialization = true;
	Audio->bOverrideAttenuation = true;
	Audio->AttenuationOverrides.OmniRadius = 100.f;
	Audio->AttenuationOverrides.FalloffDistance = 3000.f;

	Audio->Sound = SoundEffect;
	Audio->Play();

	SoundEffectComponents.Add(Audio);
}

void USoundEffectManager::PlaySoundEffect_3D(ESoundEffects SFX_To_Play)
{
	UAudioComponent* Audio = NewObject<UAudioComponent>(this);
	Audio->RegisterComponent();
	Audio->AttachTo(GetOwner()->GetRootComponent(),"",EAttachLocation::SnapToTarget,true);
	
	Audio->OnAudioFinished.Add(AudioFinishDelegate);

	USoundWave* SoundEffect = EnumToSoundEffect(SFX_To_Play);

	Audio->bAllowSpatialization = true;
	Audio->bOverrideAttenuation = true;
	Audio->AttenuationOverrides.OmniRadius = 100.f;
	Audio->AttenuationOverrides.FalloffDistance = 3000.f;

	Audio->Sound = SoundEffect;
	Audio->Play();

	SoundEffectComponents.Add(Audio);
}

void  USoundEffectManager::DestroyFinishedAudio()
{
	bool WillRerun = false;
	for (size_t i = 0; i < SoundEffectComponents.Num(); i++)
	{
		if (!SoundEffectComponents[i]->IsPlaying())
		{
			SoundEffectComponents[i]->DestroyComponent();
			SoundEffectComponents.RemoveAt(i);
			WillRerun = true;
			break;
		}
	}

	if (WillRerun)DestroyFinishedAudio();
}
