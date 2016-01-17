// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCombat.h"
#include "CustomMonsterActor.h"
#include "CombatController.h"


// Sets default values
ACustomMonsterActor::ACustomMonsterActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MonsterSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MonsterMesh"));
	MonsterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MonsterCam"));
	MonsterCameraRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonsterCamRoot"));

	MonsterCamera->AttachTo(MonsterCameraRoot);

}

// Called when the game starts or when spawned
void ACustomMonsterActor::BeginPlay()
{
	Super::BeginPlay();
	
	InitialCameraRot = MonsterCameraRoot->RelativeRotation;
	ACombatController* TheController = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());
	TurningSpeed = TheController->TurningSpeed;

	for (size_t i = 0; i < 6; i++)
	{
		if (MonsterType == TheController->MonsterStats[i].MonsterType)
		{
			MaxHealth = TheController->MonsterStats[i].MaxHP;
			MaxMana = TheController->MonsterStats[i].MaxMP;

			Speed = TheController->MonsterStats[i].Speed;
			TheController->MyActionHUD->MonsterNames[MonsterID] = TheController->MonsterStats[i].MonsterNames[FMath::RandRange(0, TheController->MonsterStats[i].MonsterNames.Num() - 1)];
		}
	}
	

}

// Called every frame
void ACustomMonsterActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	MonsterCameraRoot->AddLocalRotation(FRotator(0, DeltaTime * TurningSpeed, 0));
}

