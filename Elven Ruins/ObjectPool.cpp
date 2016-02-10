

#include "PirateLand.h"
#include "ObjectPool.h"

AObjectPool::AObjectPool()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AObjectPool::BeginPlay()
{
	Super::BeginPlay();
	
	AudioFinishDelegate.BindUFunction(this, "PutObjectBackToList");

	InitializePool();

	for (size_t i = 0; i < SoundEffects_Part_1.Num(); i++)
	{
		GenericSoundEffects.Add(SoundEffects_Part_1[i], SoundEffects_Part_2[i]);
	}
}

void AObjectPool::InitializePool()
{
	for (size_t j = 0; j < MaxSFX; j++)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedAudio;

		SpawnedAudio = GetWorld()->SpawnActor<AActor>(SFXPlayer, FVector(0), FRotator(0), spawnParams);
		SpawnedAudio->FindComponentByClass<class UAudioComponent>()->OnAudioFinished.Add(AudioFinishDelegate);
		SpawnedAudio->Tags.Add("Available");

		SFXList.Add(SpawnedAudio);
	}
}

void AObjectPool::PutObjectBackToList()
{
	for (size_t i = 0; i < MaxSFX; i++)
	{
		if (!SFXList[i]->FindComponentByClass<class UAudioComponent>()->IsPlaying())
		{
			SFXList[i]->Tags[0] = "Available";

			SFXList[i]->FindComponentByClass<class UAudioComponent>()->bAllowSpatialization = false;
			SFXList[i]->FindComponentByClass<class UAudioComponent>()->bOverrideAttenuation = false;

			SFXList[i]->FindComponentByClass<class UAudioComponent>()->AttenuationOverrides.OmniRadius = 0;
			SFXList[i]->FindComponentByClass<class UAudioComponent>()->AttenuationOverrides.FalloffDistance = 0;
		}
	}
}

int32 AObjectPool::GetIndexOfFirstAvailableObject()
{
	for (size_t i = 0; i < MaxSFX; i++)
	{
		if (SFXList[i]->ActorHasTag("Available"))
		{
			SFXList[i]->Tags[0] = "Unavailable";
			return i;
		}
	}
	return -1;
}



void AObjectPool::PlaySound2D(USoundBase* AudioClip)
{
	int32 AudioIndex = GetIndexOfFirstAvailableObject();
	if (AudioIndex == -1)return;
	
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->SetSound(AudioClip);
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->Play();
}

void AObjectPool::PlaySound3D(USoundBase* AudioClip, FVector Location, float Radius, float FallOff)
{
	int32 AudioIndex = GetIndexOfFirstAvailableObject();
	if (AudioIndex == -1)
	{
		GLog->Log("Audio Object Not Found");
		return;
	}

	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->SetSound(AudioClip);
	SFXList[AudioIndex]->SetActorLocation(Location);
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->Play();

	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->bAllowSpatialization = true;
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->bOverrideAttenuation = true;

	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->AttenuationOverrides.OmniRadius = Radius;
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->AttenuationOverrides.FalloffDistance = FallOff;
}

