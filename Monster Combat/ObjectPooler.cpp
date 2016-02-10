
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//						Object Pool (For Particles and Audio) - Kostas Dokos							   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MonsterCombat.h"
#include "ObjectPooler.h"

AObjectPooler::AObjectPooler()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AObjectPooler::BeginPlay()
{
	Super::BeginPlay();
	
	AudioFinishDelegate.BindUFunction(this, "PutAudioBackToList");
	ParticleFinishDelegate.BindUFunction(this, "PutParticleBackToList");

	InitializePools();
}

#pragma region Object Pool Setup, Referencing, Maintainance

void AObjectPooler::InitializePools()
{

	for (size_t i = 0; i < MaxParticles; i++)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedObject;

		SpawnedObject = GetWorld()->SpawnActor<AActor>(ParticleTemplate, FVector(-1000), FRotator(0), spawnParams);
		SpawnedObject->FindComponentByClass<class UParticleSystemComponent>()->OnSystemFinished.Add(ParticleFinishDelegate);
		ParticleList.Add(SpawnedObject);
	}


	for (size_t j = 0; j < MaxSFX; j++)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedAudio;

		SpawnedAudio = GetWorld()->SpawnActor<AActor>(SFXPlayer, FVector(-1000), FRotator(0), spawnParams);

		SpawnedAudio->FindComponentByClass<class UAudioComponent>()->OnAudioFinished.Add(AudioFinishDelegate);

		SFXList.Add(SpawnedAudio);
	}
}

int32 AObjectPooler::GetIndexOfFirstAvailableObject_Particles()
{
	for (size_t i = 0; i < ParticleList.Num(); i++)
	{
		if (ParticleList[i]->GetActorLocation() == FVector(-1000))
		{
			return i;
		}
	}

	return -1;
}

int32 AObjectPooler::GetIndexOfFirstAvailableObject_SFX()
{
	for (size_t i = 0; i < MaxSFX; i++)
	{
		if (SFXList[i]->GetActorLocation() == FVector(-1000))
		{
			return i;
		}
	}

	return -1;
}

void AObjectPooler::PutParticleBackToList()
{
	for (size_t i = 0; i < MaxParticles; i++)
	{
		if (ParticleList[i]->FindComponentByClass<class UParticleSystemComponent>()->HasCompleted())
		{
			ParticleList[i]->SetActorLocation(FVector(-1000));
		}
	}
}

void AObjectPooler::PutAudioBackToList()
{
	for (size_t i = 0; i < MaxSFX; i++)
	{
		if (!SFXList[i]->FindComponentByClass<class UAudioComponent>()->IsPlaying())
		{
			SFXList[i]->SetActorLocation(FVector(-1000));
		}
	}
}

#pragma endregion

#pragma region Main Pooler Functionality

void AObjectPooler::SpawnParticleAtLocation(FVector Location, UParticleSystem* Template)
{
	int32 ParticleIndex = GetIndexOfFirstAvailableObject_Particles();
	if (ParticleIndex == -1)return;

	ParticleList[ParticleIndex]->FindComponentByClass<class UParticleSystemComponent>()->SetTemplate(Template);
	ParticleList[ParticleIndex]->SetActorLocation(Location);
	ParticleList[ParticleIndex]->FindComponentByClass<class UParticleSystemComponent>()->ActivateSystem();
}

void AObjectPooler::PlaySound2D(USoundBase* AudioClip)
{
	int32 AudioIndex = GetIndexOfFirstAvailableObject_SFX();
	if (AudioIndex == -1)return;

	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->SetSound(AudioClip);
	SFXList[AudioIndex]->SetActorLocation(FVector(10,10,100));
	SFXList[AudioIndex]->FindComponentByClass<class UAudioComponent>()->Play();
}

#pragma endregion