// Fill out your copyright notice in the Description page of Project Settings.

#include "EQS_Demo.h"
#include "AI_Bot.h"
#include "EngineUtils.h"
#include "MasterBot.h"


	AMasterBot* Mastermind;

// Sets default values
AAI_Bot::AAI_Bot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsAlive = true;
}


// Called when the game starts or when spawned
void AAI_Bot::BeginPlay()
{
	Super::BeginPlay();
	//UNavigationSystem::SimpleMoveToLocation(GetController(), AwesomePos);

	Master = Cast<ABot_AI_Controller>(GetController());
	USkeletalMeshComponent* Player_SkeletalMeshComponent = FindComponentByClass<class USkeletalMeshComponent>();
	MyAnimInstance = Cast<UBot_AnimInstance>(Player_SkeletalMeshComponent->GetAnimInstance());
	Mastermind = Cast<AMasterBot>(GetWorld()->GetAuthGameMode());
	bCanTargetEnemies = true;
	HidingSpot = FVector(0);
	
	TArray<UParticleSystemComponent*> Particles;
	GetComponents<UParticleSystemComponent>(Particles);
	for (int32 i = 0; i < Particles.Num(); i++)
	{
		if (Particles[i]->ComponentHasTag("BloodParticle"))
		{
			Blood = Particles[i];
		}
		if (Particles[i]->ComponentHasTag("MuzzleParticle"))
		{
			Muzzle = Particles[i];
		}
		if (Particles[i]->ComponentHasTag("FearParticle"))
		{
			Fear = Particles[i];
		}

	}

	Elusiveness = FMath::RandRange(ElusivenessMin, ElusivenessMax);
	Mobility = FMath::RandRange(MobilityMin, MobilityMax);
	Marksmanship = FMath::RandRange(MarksmanshipMin, MarksmanshipMax);
	Stamina = FMath::RandRange(StaminaMin, StaminaMax);
	Luck = FMath::RandRange(LuckMin, LuckMax);

	Fearless = FMath::RandRange(FearlessMin, FearlessMax);
	Strategy = FMath::RandRange(StrategyMin, StrategyMax);


	MaxHealth += Stamina;



	if (MyArchetype == EClassArchetypes::Tactician)
	{
		Strategy += 25;
		Master->BB->SetValueAsVector(TEXT("TacticianHomeLoc"),GetActorLocation());
	}

	if (MyArchetype == EClassArchetypes::Guard)
	{
		MaxHealth *= 1.5f;
		Fearless = 101;
	}

	if (MyArchetype == EClassArchetypes::Berserker)
	{
		MaxHealth *= 2.f;
		Fearless = 101;
		Master->BB->SetValueAsVector(TEXT("Favorite"), FVector(0));
		Elusiveness += 10;
	}

	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = 350 + Mobility;
}

// Called every frame
void AAI_Bot::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (!bIsAlive)return;

	//Enemy Detection Logic
	if(bCanTargetEnemies)
	GetEnemyTarget();

	RadarLogic(DeltaTime);
	RegenHP(DeltaTime);
	AdjustBT_Values();
	FearLogic(DeltaTime);
	ShootingLogic(DeltaTime);

}

// Called to bind functionality to input
void AAI_Bot::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

FRotator AAI_Bot::FindLookAtRotation(FVector Start, FVector Target)
{
	FVector Direction = Target - Start;
	FRotator FinalRotation = Direction.Rotation();

	return FinalRotation;
}

void AAI_Bot::GetEnemyTarget()
{
	

	FHitResult Hit;
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	FVector Origin = GetActorLocation();

	int Index = -1;
	int StrategistRoll = FMath::RandRange(1, 100);

	TArray<AAI_Bot*> EnemiesTargetingFriends;
	for (TActorIterator<AAI_Bot> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{

		if ((ActorItr->bIsAlive && Mastermind->BlueTeam.Contains(ActorItr->EnemyToShoot) && bIsRedTeam && ActorItr->EnemyToShoot != this)
			|| (ActorItr->bIsAlive && Mastermind->RedTeam.Contains(ActorItr->EnemyToShoot) && !bIsRedTeam && ActorItr->EnemyToShoot != this))
			EnemiesTargetingFriends.Add(*ActorItr);
	}

	GetBotsTargetedMe();
	if (BotsTargetedMe.Num() > 0)
	{
		TArray<AAI_Bot*> PossibleBots;
		for (size_t i = 0; i < BotsTargetedMe.Num(); i++)
		{
			PossibleBots.Add(BotsTargetedMe[i]);
		}

		if (StrategistRoll < Strategy)
		{
			Index = GetIndexOfEnemyWithLowestHealth(PossibleBots, Hit);
		}
		else
		{
			Index = GetIndexOfEnemyWithLowestDistance(PossibleBots, Hit);
		}

		TargetingRules(PossibleBots, Index, Hit);
	}
	else if (MyArchetype == EClassArchetypes::Berserker && FavoredTarget==nullptr)
	{
		// TO UPOLOGIZW LATHOS!! DEN PREPEI NA UPOLOGISW RAYCAST! GIA ARXI ME ENDIAFEREI NA PAW PROS TO MEROS TOY MALAKA POU THELW NA TARGETARW!
		// META THA GINEI TARGET EFOSON TON BLEPW KIOLAS!
		GetPossibleFavoredTarget();
	}
	else if (MyArchetype == EClassArchetypes::Tactician && EnemiesTargetingFriends.Num() > 0)
	{
		// Strategic Pick: Select the one with lowest health.
		if (StrategistRoll < Strategy)
		{
			Index = GetIndexOfEnemyWithLowestHealth(EnemiesTargetingFriends, Hit);
		}

		// Classic Pick: Select the closest one.
		else
		{
			Index = GetIndexOfEnemyWithLowestDistance(EnemiesTargetingFriends, Hit);
		}

		if (Index == -1)
		{
			Master->BB->SetValueAsBool(TEXT("Supporting"),true);
			UNavigationSystem::SimpleMoveToLocation(GetController(), PosOfFriendWhoNeedsMeMost());
		}

		TargetingRules(EnemiesTargetingFriends, Index, Hit);
	}
	else if ((MyArchetype == EClassArchetypes::Tactician && bIsRedTeam && Mastermind->RedTeam_Tacticians.Num() == Mastermind->RedTeam.Num())
		|| (MyArchetype == EClassArchetypes::Tactician && !bIsRedTeam && Mastermind->BlueTeam_Tacticians.Num() == Mastermind->BlueTeam.Num())
		|| MyArchetype!= EClassArchetypes::Tactician)
	{
		TArray<AAI_Bot*> Enemies;
		for (TActorIterator<AAI_Bot> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->bIsAlive)
				Enemies.Add(*ActorItr);
		}

		// Strategic Pick: Select the one with lowest health.
		if (StrategistRoll < Strategy)
		{
			Index = GetIndexOfEnemyWithLowestHealth(Enemies, Hit);
		}

		// Classic Pick: Select the closest one.
		else
		{
			Index = GetIndexOfEnemyWithLowestDistance(Enemies, Hit);
		}

		TargetingRules(Enemies, Index, Hit);
	}


	if (MyArchetype == EClassArchetypes::Berserker)
	{
		if (FavoredTarget != nullptr)
		{
			Master->BB->SetValueAsVector(TEXT("Favorite"), FavoredTarget->GetActorLocation());
		}
		else
		{
			Master->BB->SetValueAsVector(TEXT("Favorite"), FVector(0));
		}
	}

	//Reset Target Logic
	// Make sure we reset if we lose line of sight.
	if (EnemyToShoot)
	{
		FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
		ColParams.AddIgnoredActor(GetOwner());
		ColParams.AddIgnoredActor(this);

		FVector Origin = GetActorLocation();
		FVector Destination = (EnemyToShoot->GetActorLocation() - Origin)*AggroRadius;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
		{
			AAI_Bot* BotToShoot = Cast<AAI_Bot>(Hit.GetActor());
			if (!BotToShoot)
			{
				EnemyToShoot = nullptr;
				if (MyArchetype == EClassArchetypes::Tactician)
					Master->BB->SetValueAsBool(TEXT("Supporting"), false);
			}
		}
	}
	
}

void AAI_Bot::TakeDamage(float Damage)
{
	CurrentHealth -= Damage;
	if (CurrentHealth < 0)
	{
		//Death Logic
		Die();
	}

	bHasAggro = true;
	float PercentageHPLost = ((MaxHealth - CurrentHealth) / MaxHealth) * 100;
	if (PercentageHPLost > Fearless && Fearless<100)
	{
		int R = FMath::RandRange(1, 100);
		if (R < 50 - (Fearless/5))
		{
			InternalFearTimer = 0;
			bWillCountFear = true;
			Fear->Activate();
			Master->BB->SetValueAsBool(TEXT("Feared"), true);
			bCanTargetEnemies = false;
			EnemyToShoot = nullptr;

			if(MyArchetype == EClassArchetypes::Tactician)
			Master->BB->SetValueAsBool(TEXT("Supporting"), false);
		}
	}
}

void AAI_Bot::Die()
{
	bIsAlive = false;
	
	FindComponentByClass<class UAudioComponent>()->SetSound(DeathSfx);
	FindComponentByClass<class UAudioComponent>()->Play();

	Muzzle->Deactivate();
	Blood->Activate();

	if (bIsRedTeam)Mastermind->BlueTeamKills++;
	else Mastermind->RedTeamKills++;

	Mastermind->ChangeMorale(bIsRedTeam, -0.08f);
	
	if (bIsRedTeam)
	{
		Mastermind->RedTeam.Remove(this);
		switch (MyArchetype)
		{
		case EClassArchetypes::Berserker:
			Mastermind->RedTeam_Berserkers.Remove(this);
			break;

		case EClassArchetypes::Warrior:
			Mastermind->RedTeam_Warriors.Remove(this);
			break;

		case EClassArchetypes::Tactician:
			Mastermind->RedTeam_Tacticians.Remove(this);
			break;

		case EClassArchetypes::Guard:
			Mastermind->RedTeam_Guards.Remove(this);
			break;
		}
	}
	else
	{
		Mastermind->BlueTeam.Remove(this);
		switch (MyArchetype)
		{
		case EClassArchetypes::Berserker:
			Mastermind->BlueTeam_Berserkers.Remove(this);
			break;

		case EClassArchetypes::Warrior:
			Mastermind->BlueTeam_Warriors.Remove(this);
			break;

		case EClassArchetypes::Tactician:
			Mastermind->BlueTeam_Tacticians.Remove(this);
			break;

		case EClassArchetypes::Guard:
			Mastermind->BlueTeam_Guards.Remove(this);
			break;
		}
	}
	


	TArray<UStaticMeshComponent*> CamComps;
	GetComponents<UStaticMeshComponent>(CamComps);
	for (int32 i = 0; i < CamComps.Num(); i++)
	{
		if (CamComps[i]->ComponentHasTag("Blood"))
		{
			CamComps[i]->SetVisibility(true);
		}

	}
	EnemyToShoot = nullptr;
	if (MyArchetype == EClassArchetypes::Tactician)
		Master->BB->SetValueAsBool(TEXT("Supporting"), false);
	USkeletalMeshComponent* Player_SkeletalMeshComponent = FindComponentByClass<class USkeletalMeshComponent>();
	Player_SkeletalMeshComponent->SetSimulatePhysics(true);
	Player_SkeletalMeshComponent->Stop();
	Player_SkeletalMeshComponent->SetAllBodiesSimulatePhysics(true);
	Player_SkeletalMeshComponent->WakeAllRigidBodies();
	
	GetRootPrimitiveComponent()->SetCollisionProfileName(TEXT("Ragdoll"));

	Master->BT->StopTree(EBTStopMode::Forced);
	GetCharacterMovement()->MaxWalkSpeed = 0;
	GetRootComponent()->SetMobility(EComponentMobility::Static);
	bCanTargetEnemies = false;

	if (bIsRedTeam)
	{
		for (size_t i = 0; i < Mastermind->BlueTeam_Berserkers.Num(); i++)
		{
			if (Mastermind->BlueTeam_Berserkers[i]->FavoredTarget != nullptr)
			{
				if (Mastermind->BlueTeam_Berserkers[i]->FavoredTarget == this)
				{
					Mastermind->BlueTeam_Berserkers[i]->FavoredTarget = nullptr;
					Mastermind->BlueTeam_Berserkers[i]->Master->BB->SetValueAsVector(TEXT("Favorite"), FVector(0));
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < Mastermind->RedTeam_Berserkers.Num(); i++)
		{
			if (Mastermind->RedTeam_Berserkers[i]->FavoredTarget != nullptr)
			{
				if (Mastermind->RedTeam_Berserkers[i]->FavoredTarget == this)
				{
					Mastermind->RedTeam_Berserkers[i]->FavoredTarget = nullptr;
					Mastermind->RedTeam_Berserkers[i]->Master->BB->SetValueAsVector(TEXT("Favorite"), FVector(0));
				}
			}
		}
	}


	for (size_t i = 0; i < Mastermind->AllBots.Num(); i++)
	{
		if (Mastermind->AllBots[i]->EnemyToShoot == this)
		{
			Mastermind->AllBots[i]->EnemyToShoot = nullptr;
			Mastermind->AllBots[i]->Master->BB->SetValueAsBool(TEXT("HasTarget"), nullptr);

			if (Mastermind->AllBots[i]->MyArchetype == EClassArchetypes::Tactician)
				Mastermind->AllBots[i]->Master->BB->SetValueAsBool(TEXT("Supporting"), false);

		}

	}


	Mastermind->LastStand(bIsRedTeam);
}

void AAI_Bot::RegenHP(float DeltaTime)
{
	AggroCD_Cur += DeltaTime;
	if (AggroCD_Cur > AggroCD && !bHasAggro)
	{
		AggroCD_Cur = 0;
		CurrentHealth += MaxHealth / 10;
		if (CurrentHealth > MaxHealth)CurrentHealth = MaxHealth;
	}
}

void AAI_Bot::RadarLogic(float DeltaTime)
{
	InternalParticleTimer += DeltaTime;
	if (InternalParticleTimer > 0.1f && bShouldTurnOffParticle)
	{
		bShouldTurnOffParticle = false;
		Muzzle->Deactivate();
	}

	CurrentRadarCooldown += DeltaTime;
	if (CurrentRadarCooldown > RadarCooldown)
	{
		CurrentRadarCooldown = 0;
		GetBotsTargetedMe();

		if (BotsTargetedMe.Num() > 0)
		{
			bHasAggro = true;
		}

		Master->BB->SetValueAsBool(TEXT("TwoPlusTargetedMe"), (BotsTargetedMe.Num() >= 3));

		if (BotsTargetedMe.Num() >= 3 && MyArchetype != EClassArchetypes::Guard && MyArchetype != EClassArchetypes::Berserker)
		{
			bCanTargetEnemies = false;
			EnemyToShoot = nullptr;
			if (MyArchetype == EClassArchetypes::Tactician)
				Master->BB->SetValueAsBool(TEXT("Supporting"), false);
		}
	}

	RetargetingAdjustment();
}

void AAI_Bot::RetargetingAdjustment()
{



	if (!bCanTargetEnemies && HidingSpot == FVector(0) && Master->BB->GetValueAsBool(TEXT("TwoPlusTargetedMe")))
	{
		HidingSpot = Master->BB->GetValueAsVector(TEXT("WanderLocation"));
	}
	if (HidingSpot != FVector(0))
	{
		if (FVector::Dist(GetActorLocation(), HidingSpot) < ResetFearDistance)
		{
			HidingSpot = FVector(0);
			bCanTargetEnemies = true;
			bHasAggro = false;
			AggroCD_Cur = 0;
		}
		else if (MyArchetype == EClassArchetypes::Tactician)
		{
			if (FVector::Dist(GetActorLocation(), Master->BB->GetValueAsVector(TEXT("TacticianHomeLoc"))) < ResetFearDistance)
			{
				HidingSpot = FVector(0);
				bCanTargetEnemies = true;
				bHasAggro = false;
				AggroCD_Cur = 0;
			}
		}
	}

	if (!Master->BB->GetValueAsBool(TEXT("TwoPlusTargetedMe")) && !Master->BB->GetValueAsBool(TEXT("Feared")))
	{
		HidingSpot = FVector(0);
		bCanTargetEnemies = true;
		bHasAggro = false;
		AggroCD_Cur = 0;
	}
}

void AAI_Bot::AdjustBT_Values()
{
	if (EnemyToShoot != nullptr)
	{
		bHasAggro = true;
		MyAnimInstance->bHasTarget = true;
		Master->BB->SetValueAsBool(TEXT("HasTarget"), true);
	}
	else
	{
		MyAnimInstance->bHasTarget = false;
		Master->BB->SetValueAsBool(TEXT("HasTarget"), false);
	}
}

void AAI_Bot::ShootingLogic(float DeltaTime)
{
	if (EnemyToShoot != nullptr)
	{
		SetActorRotation(FindLookAtRotation(GetActorLocation(), EnemyToShoot->GetActorLocation()));

		CurrentShootingTimer += DeltaTime;
		if (CurrentShootingTimer > ShootingCD)
		{
			CurrentShootingTimer = 0;
			FindComponentByClass<class UAudioComponent>()->SetSound(GunshotSfx);
			FindComponentByClass<class UAudioComponent>()->Play();
			
			Muzzle->Activate();
			
			InternalParticleTimer = 0;
			bShouldTurnOffParticle = true;

			FHitResult Hit;
			FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
			ColParams.AddIgnoredActor(GetOwner());
			ColParams.AddIgnoredActor(this);

			FVector Origin = GetActorLocation();
			FVector ForwardVector = GetCapsuleComponent()->GetForwardVector();
			FVector Destination = Origin + (ForwardVector * 5000);


			if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
			{

				AAI_Bot* BotToShoot = Cast<AAI_Bot>(Hit.GetActor());
				if (BotToShoot)
				{
					int ChanceDie = FMath::RandRange(1, 100);
					if (ChanceDie < Marksmanship)
					{
						ChanceDie = FMath::RandRange(1, 100);
						if (ChanceDie >= BotToShoot->Elusiveness + (BotToShoot->bCanTargetEnemies) ? 0 : BotToShoot->Mobility / 20)
						{
							float Damage = DamagePerBullet + FMath::RandRange(-DamageVariance, DamageVariance);
							
							ChanceDie = FMath::RandRange(1, 100);
							if (ChanceDie < Luck)
							{
								Damage *= 2;
							}

							ChanceDie = FMath::RandRange(1, 100);
							if (ChanceDie < GlobalHeadShotChance)
							{
								Damage = 1000;
								Mastermind->ChangeMorale(bIsRedTeam, 0.03f);
							}

							if (!bIsRedTeam)
								Damage *= Mastermind->BlueTeamMorale;
							else
								Damage *= Mastermind->RedTeamMorale;

							if(BotToShoot->bIsAlive)
							BotToShoot->TakeDamage(Damage);

							if(EnemyToShoot)
							if (!EnemyToShoot->bIsAlive)
							{
								if (MyArchetype == EClassArchetypes::Tactician)
									Master->BB->SetValueAsBool(TEXT("Supporting"), false);
								EnemyToShoot = nullptr;
							}
						}
					}
				}
			}
		}
	}
}

void AAI_Bot::FearLogic(float DeltaTime)
{
	if (bWillCountFear)
	{
		InternalFearTimer += DeltaTime;
		if (InternalFearTimer > InternalFearCD)
		{
			InternalFearTimer = 0;
			bWillCountFear = false;
			Master->BB->SetValueAsBool(TEXT("Feared"), false);
			Fear->Deactivate();
			bCanTargetEnemies = true;
		}
	}
}

void AAI_Bot::GetBotsTargetedMe()
{

	TArray<AAI_Bot*> Enemies;
	if (bIsRedTeam)Enemies = Mastermind->BlueTeam;
	else Enemies = Mastermind->RedTeam;

	BotsTargetedMe.Empty();
		
	for (size_t i = 0; i < Enemies.Num(); i++)
	{
		if (Enemies[i]->EnemyToShoot == this)
		{
			BotsTargetedMe.Add(Enemies[i]);
		}
	}

	if(BotsTargetedMe.Num()>0)bHasAggro = true;
}

int AAI_Bot::GetIndexOfEnemyWithLowestHealth(TArray<AAI_Bot*> EnemySet, FHitResult &Hit)
{
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = GetActorLocation();


	int32 Index = -1;
	float LowestHealth = 100000000;
	for (size_t i = 0; i < EnemySet.Num(); i++)
	{
		if (FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation()) <= AggroRadius)
		{
			if (EnemySet[i]->CurrentHealth < LowestHealth)
			{
				if (EnemySet[i]->bIsAlive && EnemySet[i]->bIsRedTeam != bIsRedTeam)
				{
					FVector Destination = (EnemySet[i]->GetActorLocation() - Origin)*AggroRadius;
					if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
					{
						AAI_Bot* BotToShoot = Cast<AAI_Bot>(Hit.GetActor());
						if (BotToShoot)
						{
							LowestHealth = EnemySet[i]->CurrentHealth;
							Index = i;
						}
					}
				}
			}
		}
		else
		{
			if (EnemySet[i] == EnemyToShoot)
			{
				if (MyArchetype == EClassArchetypes::Tactician)
					Master->BB->SetValueAsBool(TEXT("Supporting"), false);
				EnemyToShoot = nullptr;
			}
		}
	}

	return Index;
}

int AAI_Bot::GetIndexOfEnemyWithLowestDistance(TArray<AAI_Bot*> EnemySet, FHitResult &Hit)
{
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = GetActorLocation();


	int32 Index = -1;
	float Distance = 100000000;
	for (size_t i = 0; i < EnemySet.Num(); i++)
	{
		if (FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation()) <= AggroRadius)
		{
			if (FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation()) < Distance)
			{
				if (EnemySet[i]->bIsAlive && EnemySet[i]->bIsRedTeam != bIsRedTeam)
				{
					FVector Destination = (EnemySet[i]->GetActorLocation() - Origin)*AggroRadius;
					if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
					{
						AAI_Bot* BotToShoot = Cast<AAI_Bot>(Hit.GetActor());
						if (BotToShoot)
						{
							Distance = FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation());
							Index = i;
						}
					}
				}
			}
		}
		else
		{
			if (EnemySet[i] == EnemyToShoot)
			{
				if (MyArchetype == EClassArchetypes::Tactician)
					Master->BB->SetValueAsBool(TEXT("Supporting"), false);
				EnemyToShoot = nullptr;
			}
		}
	}

	return Index;
}

void AAI_Bot::TargetingRules(TArray<AAI_Bot*> PossibleBots, int Index, FHitResult &Hit)
{
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = GetActorLocation();

	if (EnemyToShoot == nullptr)
	{
		if (Index != -1)
		{
			if (MyArchetype == EClassArchetypes::Berserker)
			{
				if (FavoredTarget == Cast<AAI_Bot>(PossibleBots[Index]))
				{
					EnemyToShoot = Cast<AAI_Bot>(PossibleBots[Index]);
				}
			}
			else
			{
				EnemyToShoot = Cast<AAI_Bot>(PossibleBots[Index]);
			}
		}
	}
	else
	{
		FVector Destination = (EnemyToShoot->GetActorLocation() - Origin)*AggroRadius;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
		{
			AAI_Bot* BotToShoot = Cast<AAI_Bot>(Hit.GetActor());
			if (!BotToShoot)
			{
				EnemyToShoot = nullptr;
				if (MyArchetype == EClassArchetypes::Tactician)
					Master->BB->SetValueAsBool(TEXT("Supporting"), false);
			}
		}
	}
}



int AAI_Bot::GetIndexOfEnemyWithLowestHealth(TArray<AAI_Bot*> EnemySet)
{
	
	int32 Index = -1;
	float LowestHealth = 100000000;
	for (size_t i = 0; i < EnemySet.Num(); i++)
	{
		if (EnemySet[i]->CurrentHealth < LowestHealth)
		{
			if (EnemySet[i]->bIsAlive && EnemySet[i]->bIsRedTeam != bIsRedTeam)
			{
				LowestHealth = EnemySet[i]->CurrentHealth;
				Index = i;
			}
		}
	}

	return Index;
}

int AAI_Bot::GetIndexOfEnemyWithLowestDistance(TArray<AAI_Bot*> EnemySet)
{
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = GetActorLocation();


	int32 Index = -1;
	float Distance = 100000000;
	for (size_t i = 0; i < EnemySet.Num(); i++)
	{
		if (FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation()) < Distance)
		{
			if (EnemySet[i]->bIsAlive && EnemySet[i]->bIsRedTeam != bIsRedTeam)
			{

				Distance = FVector::Dist(GetActorLocation(), EnemySet[i]->GetActorLocation());
				Index = i;

			}
		}

	}

	return Index;
}

void AAI_Bot::TargetingRules(TArray<AAI_Bot*> PossibleBots, int Index)
{
	
	
		if (Index != -1)
		{
			//AIController->SetSeenTarget(Enemies[Index]);
			FavoredTarget = Cast<AAI_Bot>(PossibleBots[Index]);
		}
	
}

FVector AAI_Bot::PosOfFriendWhoNeedsMeMost()
{
	float LowestPercentage = -1;
	FVector Loc = FVector(0);
	if (bIsRedTeam)
	{
		for (size_t i = 0; i < Mastermind->RedTeam.Num(); i++)
		{
			if (Mastermind->RedTeam[i]->BotsTargetedMe.Num()>0)
				if (Mastermind->RedTeam[i]->MaxHealth - Mastermind->RedTeam[i]->CurrentHealth / Mastermind->RedTeam[i]->MaxHealth)
				{
					LowestPercentage = Mastermind->RedTeam[i]->MaxHealth - Mastermind->RedTeam[i]->CurrentHealth / Mastermind->RedTeam[i]->MaxHealth;
					Loc = Mastermind->RedTeam[i]->GetActorLocation();
				}
		}
	}
	else
	{
		for (size_t i = 0; i < Mastermind->BlueTeam.Num(); i++)
		{
			if(Mastermind->BlueTeam[i]->BotsTargetedMe.Num()>0)
			if (Mastermind->BlueTeam[i]->MaxHealth - Mastermind->BlueTeam[i]->CurrentHealth / Mastermind->BlueTeam[i]->MaxHealth)
			{
				LowestPercentage = Mastermind->BlueTeam[i]->MaxHealth - Mastermind->BlueTeam[i]->CurrentHealth / Mastermind->BlueTeam[i]->MaxHealth;
				Loc = Mastermind->BlueTeam[i]->GetActorLocation();
			}
		}

		
	}
	return Loc;
}

void AAI_Bot::GetPossibleFavoredTarget()
{
	
	int Index = -1;
	if (bIsRedTeam)
	{
		int StrategicRoll = FMath::RandRange(1, 100);
		TArray<AAI_Bot*> Enemies;
		for (size_t i = 0; i < Mastermind->BlueTeam_Warriors.Num(); i++)
		{
			Enemies.Add(Mastermind->BlueTeam_Warriors[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->BlueTeam_Berserkers.Num(); i++)
		{
			Enemies.Add(Mastermind->BlueTeam_Berserkers[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->BlueTeam_Tacticians.Num(); i++)
		{
			Enemies.Add(Mastermind->BlueTeam_Tacticians[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->BlueTeam_Guards.Num(); i++)
		{
			Enemies.Add(Mastermind->BlueTeam_Guards[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}
	}
	else
	{
		int StrategicRoll = FMath::RandRange(1, 100);
		TArray<AAI_Bot*> Enemies;
		for (size_t i = 0; i < Mastermind->RedTeam_Warriors.Num(); i++)
		{
			Enemies.Add(Mastermind->RedTeam_Warriors[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->RedTeam_Berserkers.Num(); i++)
		{
			Enemies.Add(Mastermind->RedTeam_Berserkers[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->RedTeam_Tacticians.Num(); i++)
		{
			Enemies.Add(Mastermind->RedTeam_Tacticians[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}

		StrategicRoll = FMath::RandRange(1, 100);
		for (size_t i = 0; i < Mastermind->RedTeam_Guards.Num(); i++)
		{
			Enemies.Add(Mastermind->RedTeam_Guards[i]);
		}
		if (Enemies.Num() > 0)
		{
			if (StrategicRoll < Strategy)
			{
				Index = GetIndexOfEnemyWithLowestHealth(Enemies);
			}
			else
			{
				Index = GetIndexOfEnemyWithLowestDistance(Enemies);
			}

			TargetingRules(Enemies, Index);
			return;
		}
	}
}