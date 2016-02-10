
//////////////////////////////////////
// Monster Character - Kostas Dokos //
//////////////////////////////////////

#include "PirateLand.h"
#include "UnrealNetwork.h"
#include "MMO_Character.h"
#include "MMO_Mob_Character.h"

AMMO_Mob_Character::AMMO_Mob_Character()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMMO_Mob_Character::BeginPlay()
{
	Super::BeginPlay();
	
	Monster_SkeletalMesh = FindComponentByClass<class USkeletalMeshComponent>();
	MonsterAnimInstance = Cast<UMMO_Mob_AnimInstance>(Monster_SkeletalMesh->GetAnimInstance());

	TArray<UWidgetComponent*> MyWidgets;
	GetComponents<UWidgetComponent>(MyWidgets);
	for (size_t i = 0; i < MyWidgets.Num(); i++)
	{
		if (MyWidgets[i]->ComponentHasTag("HP Bar"))
		{
			MyHealthBar = Cast<UMMO_OverheadUI_HUD>(MyWidgets[i]->GetUserWidgetObject());
		}
		if (MyWidgets[i]->ComponentHasTag("LockOnWidget"))
		{
			MyLockOnWidget = Cast<UMMO_OverheadUI_HUD>(MyWidgets[i]->GetUserWidgetObject());
		}
		else
		{
			MyWidgetComp = MyWidgets[i];
		}
	}
	
	MyDamageOverlay = Cast<UMMO_DamageOverlay_HUD>(MyWidgetComp->GetUserWidgetObject());
	
	CurrentHealth = MaxHealth;
	MyHealthBar->HPBarValue = CurrentHealth / MaxHealth;
	MyHealthBar->bIsEnemyHPBar = true;
	MyHealthBar->bIsMyOwnerAlive = true;

	MyController = Cast<AMMO_Mob_AI_Controller>(GetController());

	PawnSensingComp = FindComponentByClass<class UPawnSensingComponent>();

	//Register the function that is going to fire when the character sees a Pawn
	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AMMO_Mob_Character::OnSeePlayer);
	}

	MyColliderComp = FindComponentByClass<class UBoxComponent>();

	MyColliderComp->OnComponentBeginOverlap.AddDynamic(this, &AMMO_Mob_Character::OnOverlapBegin);       // set up a notification for when this component overlaps something
	MyColliderComp->OnComponentEndOverlap.AddDynamic(this, &AMMO_Mob_Character::OnOverlapEnd);       // set up a notification for when this component overlaps something
}

void AMMO_Mob_Character::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	
	UI_Updater();

	if (bIsDead)
		return;
	
	AttackingLogic();
	
	UpdateLocationOfMonster();
}

#pragma region Tick Helpers

void AMMO_Mob_Character::UpdateLocationOfMonster()
{
	if (MyController)
	{
		MyController->BlackboardComp->SetValueAsVector(TEXT("CurrentLocation"), GetActorLocation());
		if (bIsDead)MyController->BlackboardComp->SetValueAsBool(TEXT("IAmDead"), true);
	}
}

void AMMO_Mob_Character::AttackingLogic()
{
	if (MonsterAnimInstance)
	{
		AnimFixer();
		AttackHero();

		if (MonsterAnimInstance->Montage_IsPlaying(MeleeAttack))
		{
			if (MyCurrentTarget)
				SetActorRotation((GetActorLocation() - MyCurrentTarget->GetActorLocation()).Rotation());
		}
	}
}

void AMMO_Mob_Character::UI_Updater()
{
	if (MyHealthBar)
		MyHealthBar->EnemyPos = GetActorLocation();
}

void AMMO_Mob_Character::AnimFixer()
{
	FVector Dir;
	float Len;
	Cast<UPrimitiveComponent>(GetRootComponent())->ComponentVelocity.ToDirectionAndLength(Dir, Len);
	MonsterAnimInstance->bIsMoving = (Len > 0);
	
	if (Role == ROLE_Authority)
		Server_MoveQuick((MyCurrentTarget != nullptr));

	GetCharacterMovement()->MaxWalkSpeed = (bHasAggro) ? 600 : 200;
	MonsterAnimInstance->bHasAggro = (bHasAggro) ? true : false;
}

#pragma endregion

#pragma region Gameplay Related (Combat, Death, Damage)

void AMMO_Mob_Character::TakeDamageFromHero(int32 Damage, AActor* DamageCauser)
{
	Server_AggroTarget(this,DamageCauser);

	if (Cast<AMMO_Character>(DamageCauser)->bIsWarrior)
	{
		Cast<AMMO_Character>(DamageCauser)->Multicast_PlaySound(
			Cast<AMMO_Character>(DamageCauser)->GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_SwordHit), GetActorLocation());
	}
	else
	{
		Cast<AMMO_Character>(DamageCauser)->Multicast_PlaySound(
			Cast<AMMO_Character>(DamageCauser)->GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_ArrowHit), GetActorLocation());
	}

	Damage -= Damage*DamageReduction;

	CurrentHealth -= Damage;
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
		Die(DamageCauser);
	}
	else
	{
		if(!MonsterAnimInstance->Montage_IsPlaying(MeleeAttack))
			MonsterAnimInstance->GetHurt();
	}

	MyDamageOverlay->DoSomeDamage(false, FLinearColor(1, 1, 1, 1), *FString::SanitizeFloat(Damage));
	MyHealthBar->HPBarValue = CurrentHealth / MaxHealth;
	MyHealthBar->bIsEnemyHPBar = true;


}

void AMMO_Mob_Character::AttackHero()
{
	FVector BaseSocketLoc = Monster_SkeletalMesh->GetSocketLocation(FName("RightHand"));
	FVector TipSocketLoc = Monster_SkeletalMesh->GetSocketLocation(FName("Dagger_End"));

	const int sub = 32;
	float curLength = (BaseSocketLoc - TipSocketLoc).Size() * 2;
	float prevLength = (prevBase - prevTip).Size();
	for (int i = 1; i < sub; i++)
	{
		FVector tmpBase = FMath::Lerp(BaseSocketLoc, prevBase, i / float(sub));
		FVector tmpTip = FMath::Lerp(TipSocketLoc, prevTip, i / float(sub));
		FVector tmpOff = (tmpTip - tmpBase);
		tmpOff.Normalize();
	}
	prevBase = BaseSocketLoc;
	prevTip = TipSocketLoc;

	FHitResult Hit;
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("NoName"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, BaseSocketLoc, TipSocketLoc, ECollisionChannel::ECC_WorldDynamic, ColParams))
	{
		if (Hit.Actor != GetOwner() && MonsterAnimInstance->bCanDamageMelee)
		{
			if (Cast<AMMO_Character>(Hit.Actor.Get()))
			{
				AMMO_Character* Hero = Cast<AMMO_Character>(Hit.Actor.Get());
				if (!Hero->bIsDead)
				{
					if (Role < ROLE_Authority)
					{
						MonsterAnimInstance->bCanDamageMelee = false;
						Hero->ServerTakeDamageFromEnemy(Attack, this);
					}
				}
			}
		}
	}
}

void AMMO_Mob_Character::Die(AActor* DamageCauser)
{
	bIsDead = true;

	if (MonsterAnimInstance->Montage_IsPlaying(MeleeAttack))MonsterAnimInstance->Montage_Stop(0.f,MeleeAttack);

	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetActorEnableCollision(false);

	MonsterAnimInstance->bHasDied = true;

	for (size_t i = 0; i < 2; i++)
	{
		MyLockOnWidget->PlayersLockedOn[i] = -1;
	}
	
	Cast<AMMO_Mob_AI_Controller>(GetController())->BlackboardComp->SetValueAsBool(TEXT("AmIDead"), true);
	Cast<AMMO_Mob_AI_Controller>(GetController())->BehaviorComp->StopTree(EBTStopMode::Forced);
	GetCharacterMovement()->MaxWalkSpeed = 0;
	GetRootComponent()->SetMobility(EComponentMobility::Static);

	MyHealthBar->bIsMyOwnerAlive = false;
}

#pragma endregion

#pragma region AI Sensoring Components (Collisions for Melee, Sight, Aggro)

void AMMO_Mob_Character::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(GetController());

		if (Cast<AMMO_Character>(OtherActor) && OtherActor == MyCurrentTarget)
		{
			if (!Cast<AMMO_Character>(OtherActor)->bIsDead)
			{
				AIController->BlackboardComp->SetValueAsBool(TEXT("IsInMeleeRange"), true);
				EnemyInMeleeRange = Cast<AActor>(MyCurrentTarget);
			}
		}
	
}

void AMMO_Mob_Character::OnOverlapEnd(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(GetController());

	if (Cast<AMMO_Character>(OtherActor) && OtherActor == MyCurrentTarget)
	{
		if (!Cast<AMMO_Character>(OtherActor)->bIsDead)
		{
			AIController->BlackboardComp->SetValueAsBool(TEXT("IsInMeleeRange"), false);
			EnemyInMeleeRange = nullptr;
		}
	}
}


void AMMO_Mob_Character::OnSeePlayer(APawn* Pawn)
{
	AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(GetController());
	
	//Set the seen target on the blackboard
	if (AIController)
	{
		if (Cast<AMMO_Character>(Pawn))
		{
			if (!Cast<AMMO_Character>(Pawn)->bIsDead)
			{
				if (MyCurrentTarget)
				{
					if (FVector::Dist(GetActorLocation(), Pawn->GetActorLocation()) < FVector::Dist(GetActorLocation(),MyCurrentTarget->GetActorLocation()))
					{
						MyCurrentTarget = Cast<AActor>(Pawn);
						AIController->SetSeenTarget(Pawn);
						//Server_AggroTarget(Pawn);
					}
				}
				else
				{
					AIController->SetSeenTarget(Pawn);
					MyCurrentTarget = Cast<AActor>(Pawn);
					//Server_AggroTarget(Pawn);
				}
			}
		}
	}
}

void AMMO_Mob_Character::MoveQuick(bool bFast)
{
	bHasAggro = bFast;
}

#pragma endregion

#pragma region Networking Functions


void AMMO_Mob_Character::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMMO_Mob_Character, bHasFinishedPath);
	DOREPLIFETIME(AMMO_Mob_Character, bIsDead);
	DOREPLIFETIME(AMMO_Mob_Character, EnemyInMeleeRange);
	DOREPLIFETIME(AMMO_Mob_Character, bIsImmunized);
	//DOREPLIFETIME(AMMO_Mob_Character, bHasAggro);
}

void AMMO_Mob_Character::Multicast_ShutDownColliders_Implementation()
{
	if (Role < ROLE_Authority)
	{
		GLog->Log("Client here!");
	}
	if (Role == ROLE_Authority)
	{
		GLog->Log("Server here!");
	}

	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	SetActorEnableCollision(false);
}
void AMMO_Mob_Character::ShutDownColliders_Implementation()
{
	Multicast_ShutDownColliders();
}
bool AMMO_Mob_Character::ShutDownColliders_Validate()
{
	return true;
}

void AMMO_Mob_Character::Server_TakeDamageFromHero_Implementation(int32 Damage, AActor* DamageCauser)
{
	Multicast_TakeDamageFromHero(Damage, DamageCauser);
}
bool AMMO_Mob_Character::Server_TakeDamageFromHero_Validate(int32 Damage, AActor* DamageCauser)
{
	return true;
}
void AMMO_Mob_Character::Multicast_TakeDamageFromHero_Implementation(int32 Damage, AActor* DamageCauser)
{
	TakeDamageFromHero(Damage, DamageCauser);
}

bool AMMO_Mob_Character::Server_AttackAnim_Validate()
{
	return true;
}
void AMMO_Mob_Character::Server_AttackAnim_Implementation()
{
	Multicast_AttackAnim();
}
void AMMO_Mob_Character::Multicast_AttackAnim_Implementation()
{
	if (Role < ROLE_Authority)
	{
		MonsterAnimInstance->Montage_Play(MeleeAttack);
	}
}

void AMMO_Mob_Character::Server_MoveQuick_Implementation(bool bFast)
{
	Multicast_MoveQuick(bFast);
}
bool AMMO_Mob_Character::Server_MoveQuick_Validate(bool bFast)
{
	return true;
}
void AMMO_Mob_Character::Multicast_MoveQuick_Implementation(bool bFast)
{
	MoveQuick(bFast);
}

void AMMO_Mob_Character::AggroTarget(AMMO_Mob_Character* Monster, AActor* Target)
{
	if (MyCurrentTarget == nullptr)
	{
		Monster->MyCurrentTarget = Target;
		Cast<AMMO_Mob_AI_Controller>(Monster->GetController())->SetSeenTarget(Cast<APawn>(Target));//BlackboardComp->SetValueAsObject(TEXT("Target"), Target);
	}
}
void AMMO_Mob_Character::Server_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target)
{
	AggroTarget(Monster, Target);
	//Multicast_AggroTarget(Target);
}
bool AMMO_Mob_Character::Server_AggroTarget_Validate(AMMO_Mob_Character* Monster, AActor* Target)
{
	return true;
}
void AMMO_Mob_Character::Multicast_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target)
{
	AggroTarget(Monster, Target);
}

void AMMO_Mob_Character::ServerImmunize_Implementation()
{
	MulticastImmunize();
}
bool AMMO_Mob_Character::ServerImmunize_Validate()
{
	return true;
}
void AMMO_Mob_Character::MulticastImmunize_Implementation()
{
	CurrentHealth = MaxHealth;
	MyHealthBar->HPBarValue = CurrentHealth / MaxHealth;
	bIsImmunized = true;
}
void AMMO_Mob_Character::ServerDeImmunize_Implementation()
{
	MulticastDeImmunize();
}
bool AMMO_Mob_Character::ServerDeImmunize_Validate()
{
	return true;
}
void AMMO_Mob_Character::MulticastDeImmunize_Implementation()
{
	bIsImmunized = false;
}

#pragma endregion