
/////////////////////////////////////////////////////
// Main Character of our Online RPG - Kostas Dokos //
/////////////////////////////////////////////////////

#include "PirateLand.h"
#include "UnrealNetwork.h"
#include "ArcherArrow.h"
#include "MMO_Mob_AI_Controller.h"

#pragma region Core Functions

AMMO_Character::AMMO_Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AMMO_Character::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	Player_SkeletalMeshComponent = FindComponentByClass<class USkeletalMeshComponent>();
	Player_SpringArmComponent = FindComponentByClass<class USpringArmComponent>();
	Player_Widget = FindComponentByClass<class UWidgetComponent>();

	CurrentHealth = MaxHealth;

	OverheadUI = Cast<UMMO_OverheadUI_HUD>(Player_Widget->GetUserWidgetObject());
	OverheadUI->CharName = CharacterName;
	float TempPercetage = CurrentHealth / MaxHealth;
	OverheadUI->HPBarValue = TempPercetage;

	AnimInstance = Cast<UMMO_Char_AnimInstance>(Player_SkeletalMeshComponent->GetAnimInstance());
	MyController = Cast<AMMO_Player_Controller>(GetController());

	TArray<UCameraComponent*> Cameras;
	GetComponents<UCameraComponent>(Cameras);
	for (size_t i = 0; i < Cameras.Num(); i++)
	{
		if (Cameras[i]->ComponentHasTag("MainCam"))
		{
			MainCam = Cameras[i];
		}
		else
		{
			LockOnCam = Cameras[i];
		}
	}

	if (!bIsWarrior)
	{
		TArray<UStaticMeshComponent*> StaticMeshes;
		GetComponents<UStaticMeshComponent>(StaticMeshes);
		for (size_t i = 0; i < StaticMeshes.Num(); i++)
		{
			if (StaticMeshes[i]->ComponentHasTag("HandArrow"))
			{
				HandArrow = StaticMeshes[i];
			}
			else if (StaticMeshes[i]->ComponentHasTag("ArrowShooterLoc"))
			{
				ShootingLocation = StaticMeshes[i];
			}
			else if (StaticMeshes[i]->ComponentHasTag("ShootTarget"))
			{
				ShootTarget = StaticMeshes[i];
			}
		}

		
	}

	//Get Reference to Other Character.
	if (Role < ROLE_Authority)
	{
		for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (*ActorItr != this)
			{
				OtherCharacter = *ActorItr;
			}
		}
	}

	for (TActorIterator<AObjectPool> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->ActorHasTag("ObjectPooler"))
		{
			GlobalPool = *ActorItr;
		}
	}

	
}

void AMMO_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		OverheadUI->HPBarValue = 0;
		Player_SkeletalMeshComponent->SetWorldRotation(DeathRotation);
		return;
	}

	LockedTargetMechanics();

	ArcherFocusMode(DeltaTime);

	HealingPotionMechanic(DeltaTime);

	GetOtherCharacterInfo();
	UI_Updater();

	RotationFixer();
	AnimationHandler();
	MeleeAttackRaycasts();
}

#pragma region Tick Helpers

void AMMO_Character::LockedTargetMechanics()
{
	if (LockedTarget)
	{
		FRotator NewLookAt = FRotator((LockedTarget->GetActorLocation() - LockOnCam->GetComponentLocation()).Rotation());

		FRotator NewLookAtActor = FRotator((LockedTarget->GetActorLocation() - GetActorLocation()).Rotation());
		SetActorRotation(NewLookAtActor);

		LockOnCam->SetRelativeRotation(FRotator(0));
		LockOnCam->SetWorldRotation(FRotator(-22.5f, NewLookAtActor.Yaw, 0));
	}

	if (LockedTarget)
	{
		if (FVector::Dist(LockedTarget->GetActorLocation(), GetActorLocation()) > PingAreaForLockOn)
		{
			ServerLockOn();
		}
	}
}

void AMMO_Character::ArcherFocusMode(float DeltaTime)
{
	if (!bIsWarrior)
	{
		if (bIsInFocusMode)
		{
			if (Player_SpringArmComponent->TargetArmLength > FocusSpringLength)
			{
				Player_SpringArmComponent->TargetArmLength -= DeltaTime * SpringArmLengthChangeRate;
			}
		}
		else
		{
			if (Player_SpringArmComponent->TargetArmLength < NormalSpringLength)
			{
				Player_SpringArmComponent->TargetArmLength += DeltaTime * SpringArmLengthChangeRate;
			}
		}
	}
}

void AMMO_Character::HealingPotionMechanic(float DeltaTime)
{
	if (bIsHealingPotInCD)
	{
		HealingPotionCurrentTimer -= DeltaTime;
		if (HealingPotionCurrentTimer < 0)
		{
			HealingPotionCurrentTimer = 0;
			bIsHealingPotInCD = false;
		}
	}
}

void AMMO_Character::GetOtherCharacterInfo()
{
	// Feed Party stuff to your UI
	if (Role < ROLE_Authority)
	{
		if (GetController() && OtherCharacter)
		{
			if (Cast<AMMO_Player_Controller>(GetController()))
			{
				Cast<AMMO_Player_Controller>(GetController())->MainHUD->OtherCharName = OtherCharacter->CharacterName;

				if (OtherCharacter->MaxHealth != 0)
					Cast<AMMO_Player_Controller>(GetController())->MainHUD->OtherHPBarVal = OtherCharacter->CurrentHealth / OtherCharacter->MaxHealth;
			}
		}
	}
}

void AMMO_Character::UI_Updater()
{
	if (OverheadUI)
	{
		OverheadUI->bIsVisible = (bMantineeHasControl) ? false : true;
		OverheadUI->HPBarValue = CurrentHealth / MaxHealth;

		if (OtherCharacter)
		{
			if (OtherCharacter->OverheadUI)
			{
				OtherCharacter->OverheadUI->bIsVisible = (bMantineeHasControl) ? false : true;
			}
		}
	}
}

#pragma endregion

#pragma endregion

#pragma region Visuals

// ************************************************************************************
// This handles animation property changes and passes them through to the anim Instance
// ************************************************************************************

void AMMO_Character::AnimationHandler()
{
	AnimInstance->bIsMoving = ((MoveX != 0 || MoveY != 0) && !bIsAttacking && !bIsShieldBlocking);
	AnimInstance->bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
	if (AnimInstance->bShootArrowNow)
	{
		AnimInstance->bShootArrowNow = false;
		ShootArrow();
	}


	if(!bIsWarrior && HandArrow)
	HandArrow->SetVisibility(AnimInstance->bIsHandArrowVisible);

	bIsAttacking = (AnimInstance->Montage_IsPlaying(AttackMontage) || AnimInstance->Montage_IsPlaying(SecondAttackMontage));

}

// ******************************************************************************************************************
// This makes sure that the character looks at the direction of the input.
// It keeps the old rotation if no input is present, so the player can freely rotate the camera around the character.
// ******************************************************************************************************************

void AMMO_Character::RotationFixer()
{
	if (bIsDead)return;

	if (AnimInstance->bDoesBowBlockMovement && LockedTarget)
	{
		Player_SkeletalMeshComponent->SetWorldRotation(GetActorRotation() + FRotator(0, -90, 0));
		LastKnownRotation = Player_SkeletalMeshComponent->GetComponentRotation();
	}
	else if (AnimInstance->bIsHandArrowVisible)
	{
		FRotator AddedRot = FRotator(0, -85, 0);
		Player_SkeletalMeshComponent->SetWorldRotation(GetActorRotation() + AddedRot);
		LastKnownLocationOfTarget = ShootTarget->GetComponentLocation();
	}
	else if (MoveX != 0 || MoveY != 0) // If input is received... fix the rotation.
	{
		if (!LockedTarget)
		{
			FRotator AddedRot = FRotator(0, Player_SpringArmComponent->GetComponentRotation().Yaw + 90, 0); //Get the Yaw value of SpringArm and add 90 degrees
			Player_SkeletalMeshComponent->SetWorldRotation(LookingDir + AddedRot);
			LastKnownRotation = Player_SkeletalMeshComponent->GetComponentRotation();
		}
		else
		{
			FRotator AddedRot = FRotator(0, LockOnCam->GetComponentRotation().Yaw + 90, 0); //Get the Yaw value of SpringArm and add 90 degrees
			Player_SkeletalMeshComponent->SetWorldRotation(LookingDir + AddedRot);
			LastKnownRotation = Player_SkeletalMeshComponent->GetComponentRotation();
		}
	}
	else if (MoveX == 0 && MoveY == 0) // If No Input is received, don't fix rotation!
	{
		if (!LockedTarget)
		{
			Player_SkeletalMeshComponent->SetWorldRotation(LastKnownRotation);
		}
		else
		{
			Player_SkeletalMeshComponent->SetWorldRotation(GetActorRotation() + FRotator(0,-90,0));
		}
	}

	if (bIsAttacking && LockedTarget)
	{
		Player_SkeletalMeshComponent->SetWorldRotation(GetActorRotation() + FRotator(0, -90, 0));
	}
}

#pragma endregion

#pragma region Gameplay

#pragma region Combat System

// ************************************************************************************************************************************
// Currently, the Damaging System works as follows: We get a reference to the monster or hero we want to hit, and then
// we apply the damage based on the attacker's Attack and Min-Max bonuses. Everything is done on server, affecting Replicated variables.
// *************************************************************************************************************************************

void AMMO_Character::GetDamage_Boss_Implementation(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, ABoss* Target)
{
	int32 BaseDamage = ATK;
	int32 BonusDamage = FMath::RandRange(ATK_Bonus_Min, ATK_Bonus_Max);
	DamageOutput = BaseDamage + BonusDamage;

	int32 FinalDamage = DamageOutput;
	ApplyDamageToMonster_Boss(DamageOutput, Target);

	if (Target->bIsDead)
	{
		for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->LockedTarget == Cast<AActor>(Target))
			{
				ActorItr->ServerLockOn();
			}
		}

		//ShutDownColliders(Target);
	}
}

void AMMO_Character::MulticastAttackMelee_Implementation()
{
	if (bIsShieldBlocking)return;

	if (!bIsAttacking || AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			if (AnimInstance->bOpportunityForAttack2)
			{
				AnimInstance->Montage_Stop(0.00f, AttackMontage);
				AnimInstance->Montage_Play(SecondAttackMontage);
				AnimInstance->bOpportunityForAttack2 = false;
			}
		}
		else
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}
}

void AMMO_Character::GetDamage_Implementation(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, AMMO_Mob_Character* Target)
{
	int32 BaseDamage = ATK; 
	int32 BonusDamage = FMath::RandRange(ATK_Bonus_Min, ATK_Bonus_Max);
	DamageOutput =  BaseDamage + BonusDamage; 

	int32 FinalDamage = DamageOutput;
	ApplyDamageToMonster(DamageOutput, Target);

	if (Target->bIsDead)
	{
		for (TActorIterator<AMMO_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->LockedTarget == Cast<AActor>(Target))
			{
				ActorItr->ServerLockOn();
			}
		}

		ShutDownColliders(Target);
	}
}

void AMMO_Character::Multicast_ShutDownColliders_Implementation(AMMO_Mob_Character* Target)
{
	Cast<UPrimitiveComponent>(Target->GetRootComponent())->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cast<UPrimitiveComponent>(Target->GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Target->FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Target->FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Target->SetActorEnableCollision(false);
	Target->GetCharacterMovement()->GravityScale = 0;

	AMMO_Mob_AI_Controller* Controller = Cast<AMMO_Mob_AI_Controller>(Target->GetController());

	if (Controller)
	{
		Controller->BehaviorComp->StopTree(EBTStopMode::Safe);
		Controller->BlackboardComp->SetValueAsObject(Controller->SensedTarget, nullptr);
	}

	//AMMO_Mob_AI_Controller* AIController = Cast<AMMO_Mob_AI_Controller>(Target->GetController())->
		//BlackboardComp->InitializeBlackboard(*(AICharacter->BehaviorTree->BlackboardAsset));
	
}

//This is a very important function, that calculates the shot arrow's damage beforehand
void AMMO_Character::ServerCalcArrowDamage_Implementation(int32 ATK, int32 Min, int32 Max)
{
	FinalArrowDamage = ATK + FMath::RandRange(Min, Max);
}

void AMMO_Character::AttackMelee_Implementation()
{
	if (bIsDead)return;

	if (LockedTarget)
	{
		Player_SkeletalMeshComponent->SetWorldRotation(GetActorRotation() + FRotator(0, -90, 0));
	}

	if (AnimInstance->bIsGrounded)
		MulticastAttackMelee();
}

// ***************************************************************
// Main Attacking Functionality done with Socket-Based Raycasting. 
// ***************************************************************

void AMMO_Character::MeleeAttackRaycasts()
{
	FVector BaseSocketLoc = Player_SkeletalMeshComponent->GetSocketLocation(FName("Sword_joint"));
	FVector TipSocketLoc = Player_SkeletalMeshComponent->GetSocketLocation(FName("Sword_Edge"));

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
		if (Hit.Actor != GetOwner() && AnimInstance->bCanDamageMelee)
		{
			if (Cast<AMMO_Mob_Character>(Hit.Actor.Get()))
			{
				if (!Cast<AMMO_Mob_Character>(Hit.Actor.Get())->bIsDead && !Cast<AMMO_Mob_Character>(Hit.Actor.Get())->bIsImmunized)
				{
					if (Role < ROLE_Authority)
					{
						AnimInstance->bCanDamageMelee = false;
						GetDamage(BaseAttack, AttackBonusMin, AttackBonusMax, Cast<AMMO_Mob_Character>(Hit.Actor.Get()));
					}
				}
			}
			else if (Cast<ABoss>(Hit.Actor.Get()))
			{
				if (!Cast<ABoss>(Hit.Actor.Get())->bIsDead)
				{
					if (Role < ROLE_Authority)
					{
						AnimInstance->bCanDamageMelee = false;

						GetDamage_Boss(BaseAttack, AttackBonusMin, AttackBonusMax, Cast<ABoss>(Hit.Actor.Get()));
					}
				}
			}
		}
	}
}

#pragma endregion

#pragma region Polishing Functions

// ****************************************************************************************************************
// Keeping around this handy function in case we notice that the server does not replicate an animation to clients.
// ****************************************************************************************************************

void AMMO_Character::Multicast_PlayAnimation_Implementation(UAnimMontage* Anim)
{
	AnimInstance->Montage_Play(Anim);
}

// ************************************************************************************************************
// Making sure that when an important audio should play on all clients, we really want this multicast function.
// ************************************************************************************************************

void AMMO_Character::Multicast_PlaySound_Implementation(USoundBase* SFX, FVector Loc)
{
	GlobalPool->PlaySound3D(SFX, Loc);
}

// ****************************
// Force the monster to see us!
// ****************************

void AMMO_Character::AggroTarget(AMMO_Mob_Character* Monster, AActor* Target)
{
	if (Monster->MyCurrentTarget == Target)return;
	if (Monster->MyCurrentTarget == nullptr)
	{
		Monster->MyCurrentTarget = Target;
		Cast<AMMO_Mob_AI_Controller>(Monster->GetController())->BlackboardComp->SetValueAsObject(TEXT("Target"), Target);
	}
}

#pragma endregion

#pragma region Damage and Death Related Functions

// *******************************************************************************************************************
// Making sure our health is clamped between 0 and MAX value (set from the BP). When we die, we make sure to play
// the right animations, update the UI as well as shut down our input, collisions, fix our rotation and reset lock-on.
// *******************************************************************************************************************

void AMMO_Character::TakeDamageFromEnemy(int32 Damage, AActor* Monster)
{
	FRotator Rot = Player_SkeletalMeshComponent->GetSocketRotation(TEXT("Head"));
	Rot = FRotator(0, Rot.Yaw, 0);
	Rot += FRotator(0, 90, 0);

	FVector ForwardVector =  (Rot.Vector() * 500);
	FVector DirToGobo = Monster->GetActorLocation() - GetActorLocation();
	ForwardVector = FVector(ForwardVector.X, ForwardVector.Y, 0);
	DirToGobo = FVector(DirToGobo.X, DirToGobo.Y, 0);

	float DotProduct = FVector::DotProduct(ForwardVector.GetSafeNormal(), DirToGobo.GetSafeNormal());
	
	float Radians = FMath::RadiansToDegrees(acosf(DotProduct));

	if (Cast<AMMO_Mob_Character>(Monster))
	{
		Damage = Cast<AMMO_Mob_Character>(Monster)->Attack + FMath::RandRange(Cast<AMMO_Mob_Character>(Monster)->AttackBonusMin, Cast<AMMO_Mob_Character>(Monster)->AttackBonusMax);
		Multicast_PlaySound(GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_DaggerHit), GetActorLocation());
	}
	else if(Cast<ABoss>(Monster))
	{
		Damage = Cast<ABoss>(Monster)->BaseDamage + FMath::RandRange(Cast<ABoss>(Monster)->BonusDamage_Min, Cast<ABoss>(Monster)->BonusDamage_Max);
		Multicast_PlaySound(GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_BossHit), GetActorLocation());
	}

	Damage -= Damage*BaseDamageReduction;

	if (FMath::Abs(Radians) <= 55.f && bIsShieldBlocking)
	{
		Damage = FMath::DivideAndRoundDown(Damage, 3);
		Multicast_PlayAnimation(GetHurtMontageShield);
	}
	else
	{
		if (!(AnimInstance->Montage_IsPlaying(AttackMontage) || AnimInstance->Montage_IsPlaying(SecondAttackMontage)))
		{
			Multicast_PlayAnimation(GetHurtMontage);
		}
	}

	UpdateHealth(Damage);
}

void AMMO_Character::UpdateHealth_Implementation(float Damage)
{
	CurrentHealth -= Damage;

	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
	}
	else if (CurrentHealth >= MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}

void AMMO_Character::UpdateHealth(int32 Damage)
{
	CurrentHealth -= Damage;
	if (CurrentHealth < 0)
	{
		CurrentHealth = 0;
		Multicast_Die();
	}
	else if(CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	
}

void AMMO_Character::Die()
{
	bIsDead = true;
	AnimInstance->bHasDied = true;
	DeathRotation = Player_SkeletalMeshComponent->GetComponentRotation();

	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cast<UPrimitiveComponent>(GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FindComponentByClass<class USkeletalMeshComponent>()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetActorEnableCollision(false);

	if (LockedTarget)
	{
		Cast<AMMO_Mob_Character>(LockedTarget)->MyLockOnWidget->PlayersLockedOn[LockOnID] = -1;
		LockedTarget = nullptr;
		MainCam->Activate();
		LockOnCam->Deactivate();
	}
}

#pragma endregion

#pragma region Input Functions

void AMMO_Character::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AMMO_Character::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMMO_Character::MoveRight);

	InputComponent->BindAxis("MoveForward", this, &AMMO_Character::MoveForwardInput);
	InputComponent->BindAxis("MoveRight", this, &AMMO_Character::MoveRightInput);
	
	InputComponent->BindAxis("LookUp", this, &AMMO_Character::TurnUp);
	InputComponent->BindAxis("LookUpRate", this, &AMMO_Character::TurnUp);
	
	InputComponent->BindAxis("Turn", this, &AMMO_Character::TurnRight);
	InputComponent->BindAxis("TurnRate", this, &AMMO_Character::TurnRight);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AMMO_Character::JumpAction);

	InputComponent->BindAction("LockOn", IE_Pressed, this, &AMMO_Character::ServerLockOn);

	InputComponent->BindAction("HealthPot", IE_Pressed, this, &AMMO_Character::Server_DrinkPotion);

	if (bIsWarrior)
	{
		InputComponent->BindAction("Attack", IE_Pressed, this, &AMMO_Character::AttackMelee);
		InputComponent->BindAction("Attack", IE_Pressed, this, &AMMO_Character::LocalAttackMelee);

		InputComponent->BindAction("ShieldBlock", IE_Pressed, this, &AMMO_Character::Server_ShieldBlockOn);
		InputComponent->BindAction("ShieldBlock", IE_Released, this, &AMMO_Character::Server_ShieldBlockOff);
	}
	else
	{
		InputComponent->BindAction("BowAttack", IE_Pressed, this, &AMMO_Character::Server_BowAttackOn);
		InputComponent->BindAction("BowAttack", IE_Released, this, &AMMO_Character::Server_BowAttackOff);

		InputComponent->BindAction("FocusMode", IE_Pressed, this, &AMMO_Character::FocusModeOn);
		InputComponent->BindAction("FocusMode", IE_Released, this, &AMMO_Character::FocusModeOff);
	}
}

void AMMO_Character::DrinkPotion()
{
	if (bMantineeHasControl)return;

	if (HealingPotions > 0 && !bIsHealingPotInCD)
	{
		HealingPotions--;
		GlobalPool->PlaySound2D(GlobalPool->GenericSoundEffects.FindRef(ESoundEffectLibrary::SFX_PotionDrink));
		bIsHealingPotInCD = true;
		HealingPotionCurrentTimer = HealingPotionCooldown;
		
		FindComponentByClass<class UParticleSystemComponent>()->ActivateSystem();

		CurrentHealth += HealingPotionPower;
		
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
		}

		OverheadUI->HPBarValue = CurrentHealth / MaxHealth;
	}
}

void AMMO_Character::FocusModeOn()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	if (LockedTarget)return;
	if (bMantineeHasControl)return;

	bIsInFocusMode = true;
}

void AMMO_Character::FocusModeOff()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	if (LockedTarget)return;

	bIsInFocusMode = false;
}

void AMMO_Character::MoveRightInput(float AxisValue)
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (AnimInstance->bDoesBowBlockMovement)return;
	if (bIsShieldBlocking)return;
	if (bMantineeHasControl)return;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorRightVector(), AxisValue);
	}
}

void AMMO_Character::MoveForwardInput(float AxisValue)
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (AnimInstance->bDoesBowBlockMovement)return;
	if (bIsShieldBlocking)return;
	if (bMantineeHasControl)return;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue);
	}
}

void AMMO_Character::MoveForward_Implementation(float AxisValue)
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (AnimInstance->bDoesBowBlockMovement)return;
	if (bIsShieldBlocking)return;
	if (bMantineeHasControl)return;

	MoveX = AxisValue;
	LookingDir = FVector(-MoveX, -MoveY, 0).Rotation();
	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue);
	}
}

void AMMO_Character::MoveRight_Implementation(float AxisValue)
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (AnimInstance->bDoesBowBlockMovement)return;
	if (bIsShieldBlocking)return;
	if (bMantineeHasControl)return;

	MoveY = AxisValue;
	LookingDir = FVector(-MoveX, -MoveY, 0).Rotation();
	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorRightVector(), AxisValue);
	}
}

void AMMO_Character::TurnUp(float AxisValue)
{
	if (LockedTarget)return;
	if (bMantineeHasControl)return;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddControllerPitchInput(AxisValue* TurningSpeed);
	}
}

void AMMO_Character::TurnRight(float AxisValue)
{
	if (LockedTarget)return;
	if (bMantineeHasControl)return;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddControllerYawInput(AxisValue * TurningSpeed);
	}
}

void AMMO_Character::JumpAction()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (bIsHoldingBow) return;
	if (bIsShieldBlocking)return;
	if (bMantineeHasControl)return;

	Jump();
}


void AMMO_Character::AttemptToLockOn()
{
	if (bIsDead)return;
	if (bMantineeHasControl)return;

	// Part 1 of 2: Set / Unset LockOn Target

	if (!LockedTarget)
	{
		TArray<AMMO_Mob_Character*> Enemies;
		for (TActorIterator<AMMO_Mob_Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if(!ActorItr->bIsDead)
			Enemies.Add(*ActorItr);
		}

		int32 Index = -1;
		float Distance = 100000000;
		for (size_t i = 0; i < Enemies.Num(); i++)
		{
			if (FVector::Dist(GetActorLocation(), Enemies[i]->GetActorLocation()) <= PingAreaForLockOn)
			{
				if (FVector::Dist(GetActorLocation(), Enemies[i]->GetActorLocation()) < Distance)
				{
					if (!Enemies[i]->bIsDead && !Enemies[i]->MonsterAnimInstance->bHasDied )
					{
						Distance = FVector::Dist(GetActorLocation(), Enemies[i]->GetActorLocation());
						Index = i;
					}
				}
			}
		}

		if (Index != -1)
		{
			if (bIsInFocusMode)
			{
				bIsInFocusMode = false;
			}

			LockedTarget = Cast<AActor>(Enemies[Index]);
		}
		else
		{
			LockedTarget = nullptr;
		}
	}
	else
	{
		Cast<AMMO_Mob_Character>(LockedTarget)->MyLockOnWidget->PlayersLockedOn[LockOnID]=-1;
		LockedTarget = nullptr;
	}


	// Part 2 of 2: Activate and Deactivate as well as set some helper variables for Rotation of Actor and Camera.

	if (LockedTarget)
	{
		LastKnownRotation = FRotator((LockedTarget->GetActorLocation() - LockOnCam->GetComponentLocation()).Rotation());
		
		Cast<AMMO_Mob_Character>(LockedTarget)->MyLockOnWidget->PlayersLockedOn[LockOnID] = LockOnID;

		SetActorRotation(LastKnownRotation);
		Player_SkeletalMeshComponent->SetWorldRotation(LastKnownRotation);
		
		LockOnCam->Activate();
		MainCam->Deactivate();

		LockOnCam->SetRelativeLocation(FVector(-300, 0, 120));
		LockOnCam->SetRelativeRotation(FRotator(-22.5f, 0, 0));
	}
	else
	{
		GLog->Log(GetActorLabel());

		LockOnCam->Deactivate();
		MainCam->Activate();
	}
}

void AMMO_Character::ShieldBlockOn()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	if (bMantineeHasControl)return;

	bIsShieldBlocking = true;
	AnimInstance->bIsShieldBlocking = bIsShieldBlocking;
}

void AMMO_Character::ShieldBlockOff()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	if (!bIsShieldBlocking)return;

	bIsShieldBlocking = false;
	AnimInstance->bIsShieldBlocking = bIsShieldBlocking;
}

void AMMO_Character::BowAttackOn()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	if (ArrowsLeft <= 0)return;
	if (bMantineeHasControl)return;

	bIsHoldingBow = true;
	AnimInstance->bIsHoldingBow = true;
}

void AMMO_Character::BowAttackOff()
{
	if (bIsAttacking)return;
	if (bIsDead)return;
	if (!GetCharacterMovement()->IsMovingOnGround())return;
	
	bIsHoldingBow = false;
	AnimInstance->bIsHoldingBow = false;

	
}

void AMMO_Character::ShootArrow()
{
	if(Role<ROLE_Authority)
	ArrowsLeft--;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = Instigator;

	FVector SpawnLoc = FVector(0);
	FVector DirHelper = FVector(0);
	if (LockedTarget)
	{
		SpawnLoc = Player_SkeletalMeshComponent->GetComponentLocation();
		DirHelper = LockedTarget->GetActorLocation() - SpawnLoc;
	}
	else
	{
		SpawnLoc = ShootingLocation->GetComponentLocation();
		DirHelper = LastKnownLocationOfTarget - SpawnLoc;
	}
	
	FRotator SpawnRot = Player_SpringArmComponent->GetComponentRotation() + FRotator(0,-90,0);

	if(Role == ROLE_Authority)
	FinalArrowDamage = BaseAttack + FMath::RandRange(AttackBonusMin, AttackBonusMax);

	AArcherArrow* OurNewObject = GetWorld()->SpawnActor<AArcherArrow>(MyArrowObject, SpawnLoc , SpawnRot, spawnParams);
	

	OurNewObject->ApplyForce(ArrowForce, this, DirHelper.GetSafeNormal());
}

#pragma endregion

#pragma endregion

#pragma region Networking Functions (Replication, Validators, Implementations that call Multicasts)

void AMMO_Character::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMMO_Character, MoveX);
	DOREPLIFETIME(AMMO_Character, MoveY);

	DOREPLIFETIME(AMMO_Character, LastKnownRotation);
	DOREPLIFETIME(AMMO_Character, LookingDir);

	DOREPLIFETIME(AMMO_Character, CurrentHealth);
	DOREPLIFETIME(AMMO_Character, MaxHealth);

	DOREPLIFETIME(AMMO_Character, bAttackNow);
	DOREPLIFETIME(AMMO_Character, bIsAttacking);

	DOREPLIFETIME(AMMO_Character, LockOnCam);
	DOREPLIFETIME(AMMO_Character, Player_SkeletalMeshComponent);

	DOREPLIFETIME(AMMO_Character, AnimInstance);
	DOREPLIFETIME(AMMO_Character, DamageOutput);
	DOREPLIFETIME(AMMO_Character, bIsDead);

	DOREPLIFETIME(AMMO_Character, DeathRotation);

	DOREPLIFETIME(AMMO_Character, bIsHoldingBow);
	DOREPLIFETIME(AMMO_Character, FinalArrowDamage);
}

void AMMO_Character::Server_DrinkPotion_Implementation()
{
	Multicast_DrinkPotion();
}
bool AMMO_Character::Server_DrinkPotion_Validate()
{
	return true;
}
void AMMO_Character::Multicast_DrinkPotion_Implementation()
{
	DrinkPotion();
}

void AMMO_Character::ServerLockOn_Implementation()
{
	MulticastLockOn();
}
bool AMMO_Character::ServerLockOn_Validate()
{
	return true;
}
void AMMO_Character::MulticastLockOn_Implementation()
{
	AttemptToLockOn();
}

void AMMO_Character::Server_ShieldBlockOn_Implementation()
{
	Multicast_ShieldBlockOn();
}
bool AMMO_Character::Server_ShieldBlockOn_Validate()
{
	return true;
}
void AMMO_Character::Multicast_ShieldBlockOn_Implementation()
{
	ShieldBlockOn();
}

void AMMO_Character::Server_ShieldBlockOff_Implementation()
{
	Multicast_ShieldBlockOff();
}
bool AMMO_Character::Server_ShieldBlockOff_Validate()
{
	return true;
}
void AMMO_Character::Multicast_ShieldBlockOff_Implementation()
{
	ShieldBlockOff();
}

void AMMO_Character::Server_BowAttackOn_Implementation()
{
	Multicast_BowAttackOn();
}
bool AMMO_Character::Server_BowAttackOn_Validate()
{
	return true;
}

void AMMO_Character::Server_BowAttackOff_Implementation()
{
	Multicast_BowAttackOff();
}
bool AMMO_Character::Server_BowAttackOff_Validate()
{
	return true;
}

void AMMO_Character::Multicast_BowAttackOn_Implementation()
{
	BowAttackOn();
}
void AMMO_Character::Multicast_BowAttackOff_Implementation()
{
	BowAttackOff();
}

void AMMO_Character::Server_ShootArrow_Implementation()
{
	Multicast_ShootArrow();
}
bool AMMO_Character::Server_ShootArrow_Validate()
{
	return true;
}

void AMMO_Character::Multicast_ShootArrow_Implementation() 
{
	ShootArrow();
}

bool AMMO_Character::ServerCalcArrowDamage_Validate(int32 ATK, int32 Min, int32 Max)
{
	return true;
}

bool AMMO_Character::GetDamage_Boss_Validate(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, ABoss* Target)
{
	return true;
}

void AMMO_Character::ApplyDamageToMonster_Boss_Implementation(int32 Damage, ABoss* Target)
{
	MCApplyDamageToMonster_Boss(Damage, Target);
}
bool AMMO_Character::ApplyDamageToMonster_Boss_Validate(int32 Damage, ABoss* Target)
{
	return true;
}
void AMMO_Character::MCApplyDamageToMonster_Boss_Implementation(int32 Damage, ABoss* Target)
{
	Target->TakeDamageFromHero(Damage, this);
}

void AMMO_Character::ApplyDamageToMonster_Implementation(int32 Damage, AMMO_Mob_Character* Target)
{
	MCApplyDamageToMonster(Damage, Target);
}
bool AMMO_Character::ApplyDamageToMonster_Validate(int32 Damage, AMMO_Mob_Character* Target)
{
	return true;
}
void AMMO_Character::MCApplyDamageToMonster_Implementation(int32 Damage, AMMO_Mob_Character* Target)
{
	Target->TakeDamageFromHero(Damage, Cast<AActor>(this));
}

bool AMMO_Character::GetDamage_Validate(int32 ATK, int32 ATK_Bonus_Min, int32 ATK_Bonus_Max, AMMO_Mob_Character* Target)
{
	return true;
}
void AMMO_Character::ShutDownColliders_Implementation(AMMO_Mob_Character* Target)
{
	Multicast_ShutDownColliders(Target);
}
bool AMMO_Character::ShutDownColliders_Validate(AMMO_Mob_Character* Target)
{
	return true;
}

void AMMO_Character::Server_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target)
{
	AggroTarget(Monster,Target);
	//Multicast_AggroTarget(Target);
}
bool AMMO_Character::Server_AggroTarget_Validate(AMMO_Mob_Character* Monster, AActor* Target)
{
	return true;
}
void AMMO_Character::Multicast_AggroTarget_Implementation(AMMO_Mob_Character* Monster, AActor* Target)
{
	AggroTarget(Monster,Target);
}

bool AMMO_Character::ServerTakeDamageFromEnemy_Validate(int32 Damage, AActor* Monster)
{
	return true;
}
void AMMO_Character::ServerTakeDamageFromEnemy_Implementation(int32 Damage, AActor* Monster)
{
	TakeDamageFromEnemy(Damage, Monster);
}
void AMMO_Character::MulticastTakeDamageFromEnemy_Implementation(int32 Damage, AActor* Monster)
{
	TakeDamageFromEnemy(Damage, Monster);
}

bool AMMO_Character::MoveForward_Validate(float AxisValue)
{
	return true;
}
bool AMMO_Character::MoveRight_Validate(float AxisValue)
{
	return true;
}

bool AMMO_Character::UpdateHealth_Validate(float Damage)
{
	return true;
}

bool AMMO_Character::AttackMelee_Validate()
{
	return true;
}

void AMMO_Character::Multicast_Die_Implementation()
{
	Die();
}

#pragma endregion
