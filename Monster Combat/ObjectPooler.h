
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//						Object Pool (For Particles and Audio) - Kostas Dokos							   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "GameFramework/Actor.h"
#include "ObjectPooler.generated.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Here, we store enumerations for quick reference from the Controller	when we map generic sound fx to enum values   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM()
enum ESoundEffectLibrary
{
	SFX_ItemUsed,
	SFX_Critical,
	SFX_Error,
	SFX_Click,
	SFX_Missed,
	SFX_Hit,
	SFX_ActionStart,
	SFX_Cancel,
	SFX_DOT
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// We want to avoid tiring our machine with numerous Memory-Consuming Spawning and Destroying Functions, so in this demo	//
// i have created a global object pool containing Audios and Particles. 													//
//																															//
// The way this works is simple: I want a particle or sound to play, and after getting a reference to the object pool 		//
// i just call the function that does exactly that. In the background, all objects are created when the game first starts.	//
//																															//
// The objects are stored in a list. Both Particles and Sounds are binded with a delegate that puts them back to the pool	//
// when they are finished, so I can do this forever, as long as I don't need more particles or sounds than those allocated.	//
// 																															//
// Just need to make sure to spawn as many concurrent particles and sounds as I need!										//
//																															//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class MONSTERCOMBAT_API AObjectPooler : public AActor
{
	GENERATED_BODY()
	
public:	
	AObjectPooler();
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere, Category = "Pool System|Particles")
		int32 MaxParticles;							
													
	UPROPERTY(EditAnywhere, Category = "Pool System|Particles")
		TArray<AActor*> ParticleList;				
													
	UPROPERTY(EditAnywhere, Category = "Pool System|Particles")
		TSubclassOf<AActor> ParticleTemplate;		
													
	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		int32 MaxSFX;								
													
	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		TArray<AActor*> SFXList;					
													
	UPROPERTY(EditAnywhere, Category = "Pool System|Audio")
		TSubclassOf<AActor> SFXPlayer;

	void InitializePools();

	UFUNCTION()
		void PutParticleBackToList();

	UFUNCTION()
		void PutAudioBackToList();

	TScriptDelegate<FWeakObjectPtr> ParticleFinishDelegate;
	TScriptDelegate<FWeakObjectPtr> AudioFinishDelegate;

	int32 GetIndexOfFirstAvailableObject_Particles();
	int32 GetIndexOfFirstAvailableObject_SFX();


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// As a Programmer, all I want to do is play a sound or spawn a particle, and don't care about anything else. //
	// So, these are the only exposed functions from the object pooler.											  //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	void SpawnParticleAtLocation(FVector Location, UParticleSystem* Template);
	
	void PlaySound2D(USoundBase* AudioClip);

};
