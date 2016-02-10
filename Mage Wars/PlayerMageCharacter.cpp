// Fill out your copyright notice in the Description page of Project Settings.

#include "MageWars.h"
#include "PlayerMageCharacter.h"
#include "AutoAttackProjectile.h"
#include "MageMastermind.h"

#include "Spell_LineTest.h"
#include "Spell_IceTrap.h"
#include "Spell_GlacierShattering.h"
#include "Spell_FrostArmor.h"

#pragma region Core Functions

APlayerMageCharacter::APlayerMageCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	PlayerComp_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("MageCamera"));
	PlayerComp_SpringArm = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("MageSpringArm"));
	PlayerComp_SpringArmExtension = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("MageSpringArmExtension"));

	PlayerComp_SkeletalMesh = GetMesh();

	PlayerComp_SkeletalMesh->SkeletalMesh = PlayerMageModel;
	

}

void APlayerMageCharacter::BeginPlay()
{
	Super::BeginPlay();

	

	InitializeGameplayStats();
}

void APlayerMageCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (!MagePlayerController)
	{
		MagePlayerController = Cast<AMageMastermind>(GetController());
		if (MagePlayerController)
		{
			MagePlayerController->GenerateUI();
		}
	}
	else
	{
		MagePlayerController->SetAudioListenerOverride(PlayerComp_SkeletalMesh, FVector(0),FRotator(0));
	}

	if (bIsInAOE && !bIsDead)
	{
		AOETimer += DeltaTime;
		if (AOETimer > AOECD)
		{
			AOETimer = 0;
			Server_TakeDamageFromEnemy(7);
			Cast<AMageMastermind>(GetWorld()->GetFirstPlayerController())->CreateDamageTextAboveActor(this, 7, 20);
		}
	}

	if (GetAnimationInstance())
	{
		GetAnimationInstance()->bHasJumped = !GetCharacterMovement()->IsMovingOnGround();
		//Server_JumpFixer();
	}

	if (JumpResetterTimer > 0)
	{
		JumpResetterTimer -= DeltaTime;
		if (JumpResetterTimer <= 0)
		{
			GetCharacterMovement()->JumpZVelocity = JumpZ_Init;

			CapsuleComp->SetSimulatePhysics(false);
		}
	}

	IceTrapParticle->SetVisibility(IceTrapped);
	
	if (bHasSpawnedProjectile)
	{
		bHasSpawnedProjectile_Timer += DeltaTime;
		if (bHasSpawnedProjectile_Timer > bHasSpawnedProjectile_Cooldown)
		{
			bHasSpawnedProjectile_Timer = 0;
			bHasSpawnedProjectile = false;
		}
	}
	if (bSpellCast)
	{
		SpellCast_Timer += DeltaTime;
		if (SpellCast_Timer > SpellCast_CD)
		{
			SpellCast_Timer = 0;
			bSpellCast = false;
		}
	}

	

	if (PlayerComp_Stats)
	{
		PlayerComp_Stats->RegenTimer += DeltaTime;
		if (PlayerComp_Stats->RegenTimer > PlayerComp_Stats->RegenTick)
		{
			PlayerComp_Stats->RegenTimer = 0;
			if (!bIsDead)
			{
				UpdateHealth(PlayerComp_Stats->HealthRegenRate * MaxHealth);
				UpdateMana(PlayerComp_Stats->ManaRegenRate * MaxMana);
			}
		}
	}
	DamageOverlayComp->Value = CurrentHealth / MaxHealth;
	/*if (IceTrapped)
	{
		if (GetAnimationInstance())
		{
			PlayerMageAnimation->WholeSpellBlend = 0;
		}
	}*/


	if (ActiveCD_1 > 0)
	{
		ActiveCD_1 -= DeltaTime;
	}
	if (ActiveCD_2 > 0)
	{
		ActiveCD_2 -= DeltaTime;
	}
	if (ActiveCD_3 > 0)
	{
		ActiveCD_3 -= DeltaTime;
	}
	if (ActiveCD_4 > 0)
	{
		ActiveCD_4 -= DeltaTime;
	}

	if (CDMessageTimer > 0)
	{
		CDMessageTimer -= DeltaTime;

		
	}
	if (ManaMessageTimer > 0)
	{
		ManaMessageTimer -= DeltaTime;
		
	}

	if (MagePlayerController)
	{
		MagePlayerController->UpdateUI();
	}
}

void APlayerMageCharacter::JumpFixer()
{
	if (bIsManipulatingCircle)
	{

		if (PlayerComp_SpringArm->GetComponentRotation().Pitch > PitchOnCircleMode)
		{
			if (CameraCircleGoingUpOrDown == 0)
			{
				CameraCircleGoingUpOrDown = 1;
			}
			else if (CameraCircleGoingUpOrDown == 2)
			{
				PlayerComp_SpringArm->SetWorldRotation(FRotator(PitchOnCircleMode, PlayerComp_SpringArm->GetComponentRotation().Yaw, PlayerComp_SpringArm->GetComponentRotation().Roll));
			}
			else if (CameraCircleGoingUpOrDown == 1)
			{
				if (PlayerComp_SpringArm->GetComponentRotation().Pitch + 0.1f > PitchOnCircleMode)
				{
					AddControllerPitchInput(0.1f);
				}
			}
		}
		else if (PlayerComp_SpringArm->GetComponentRotation().Pitch < PitchOnCircleMode)
		{
			if (CameraCircleGoingUpOrDown == 0)
			{
				CameraCircleGoingUpOrDown = 2;
			}
			else if (CameraCircleGoingUpOrDown == 1)
			{
				PlayerComp_SpringArm->SetWorldRotation(FRotator(PitchOnCircleMode, PlayerComp_SpringArm->GetComponentRotation().Yaw, PlayerComp_SpringArm->GetComponentRotation().Roll));
			}
			else if (CameraCircleGoingUpOrDown == 2)
			{
				if (PlayerComp_SpringArm->GetComponentRotation().Pitch - 0.1f < PitchOnCircleMode)
				{
					AddControllerPitchInput(-0.1f);
				}
			}
		}
		else
		{
			PlayerComp_SpringArm->SetWorldRotation(FRotator(PitchOnCircleMode, PlayerComp_SpringArm->GetComponentRotation().Yaw, PlayerComp_SpringArm->GetComponentRotation().Roll));
		}
	}
}

#pragma endregion

#pragma region Input

void APlayerMageCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &APlayerMageCharacter::Server_CharacterMoveForward);
	InputComponent->BindAxis("MoveRight", this, &APlayerMageCharacter::Server_CharacterMoveRight);

	InputComponent->BindAxis("Turn", this, &APlayerMageCharacter::CharacterMouseTurn);
	//InputComponent->BindAxis("TurnKeyboard", this, &APlayerMageCharacter::CharacterKeyboardTurn);


	InputComponent->BindAxis("LookUp", this, &APlayerMageCharacter::Server_CharacterMouseLookUpDown);

	InputComponent->BindAction("Jump", IE_Pressed, this, &APlayerMageCharacter::Server_CharacterJump);

	InputComponent->BindAction("Attack", IE_Pressed, this, &APlayerMageCharacter::Server_CharacterAttackOn);
	InputComponent->BindAction("Attack", IE_Released, this, &APlayerMageCharacter::Server_CharacterAttackOff);

	InputComponent->BindAction("RMB", IE_Pressed, this, &APlayerMageCharacter::RMB_On);
	InputComponent->BindAction("RMB", IE_Released, this, &APlayerMageCharacter::RMB_Off);

	InputComponent->BindAction("Spell1", IE_Pressed, this, &APlayerMageCharacter::Server_PrepareSpell_1);
	InputComponent->BindAction("Spell2", IE_Pressed, this, &APlayerMageCharacter::Server_PrepareSpell_2);
	InputComponent->BindAction("Spell3", IE_Pressed, this, &APlayerMageCharacter::Server_PrepareSpell_3);
	InputComponent->BindAction("Spell4", IE_Pressed, this, &APlayerMageCharacter::Server_PrepareSpell_4);

	InputComponent->BindAction("Attack", IE_Pressed, this, &APlayerMageCharacter::Server_ExecuteSpell_1);
	InputComponent->BindAction("Attack", IE_Pressed, this, &APlayerMageCharacter::Server_ExecuteSpell_2);
	InputComponent->BindAction("Attack", IE_Pressed, this, &APlayerMageCharacter::Server_ExecuteSpell_3);
	InputComponent->BindAction("Attack", IE_Pressed, this, &APlayerMageCharacter::Server_ExecuteSpell_4);
}

// ----------- Movement Input -------------- //

void APlayerMageCharacter::CharacterMoveForward(float AxisValue)
{
	if (bIsDead)return;
	if (IceTrapped)return;

	MoveDirY = AxisValue;
	float AxisVal = FMath::Abs(AxisValue);
	PlayerMageAnimation->MovementSpeedX = AxisValue;

	if (AxisVal > 0.05f)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds());


	}

	FRotator X = PlayerComp_Camera->GetForwardVector().Rotation();
	
		PlayerComp_SkeletalMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
		PlayerComp_SkeletalMesh->AddWorldRotation(FRotator(0, -90, 0));
	

}

void APlayerMageCharacter::CharacterMoveRight(float AxisValue)
{
	if (bIsDead)return;
	if (IceTrapped)return;

	MoveDirX = AxisValue;
	float AxisVal = FMath::Abs(AxisValue);
	PlayerMageAnimation->MovementSpeedY = AxisValue;
	if (AxisVal > 0.05f)
	{

		AddMovementInput(GetActorRightVector(), AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds());
		//GLog->Log(FString::SanitizeFloat(GetWorld()->GetDeltaSeconds()));
		//PlayerAwesomeMovement->AddInputVector(GetActorRightVector() * AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds(), false);
		//Internal_AddMovementInput(GetActorRightVector() * AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds(), false);

	}
	FRotator X = PlayerComp_Camera->GetForwardVector().Rotation();
	PlayerComp_SkeletalMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
	PlayerComp_SkeletalMesh->AddWorldRotation(FRotator(0, -90, 0));
	

}

// ----------- Turning Input -------------- //

void APlayerMageCharacter::CharacterMouseTurn(float AxisValue)
{

	if (bIsDead)return;

	if (FMath::Abs(AxisValue) > 0.005f)
	{
		AddControllerYawInput(AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds());
	}

}

void APlayerMageCharacter::CharacterKeyboardTurn(float AxisValue)
{

	//if (bIsDead || bIsRMBPressed)return;
//
	//if (FMath::Abs(AxisValue) > 0.005f)
	//{
	//	AddControllerYawInput(AxisValue * (TurningSpeed/1.5f) * GetWorld()->GetDeltaSeconds());
	//}

}

void APlayerMageCharacter::CharacterMouseLookUpDown(float AxisValue)
{
	if (bIsDead)return;
	if (FMath::Abs(AxisValue) > 0.005f)
	{
		float Amt = AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds();

		if (!bIsManipulatingCircle)
		{
			if (AxisValue > 0)
			{
				if (PlayerComp_SpringArm->GetComponentRotation().Pitch + Amt <= -ClampingAngle)
					return;
				else
				{
					AddControllerPitchInput(Amt);
				}
			}
			else if (AxisValue < 0)
			{
				if (PlayerComp_SpringArm->GetComponentRotation().Pitch + Amt >= ClampingAngle)
					return;
				else
				{
					AddControllerPitchInput(Amt);
				}
			}
		}
		else
		{
			if (AxisValue > 0)
			{
				if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X > MagicCircle_Z_Clamp_Min)
				{
					PlayerComp_CircleSpell->AddLocalOffset(FVector(-Amt * 15, 0, 0));
					if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X <= MagicCircle_Z_Clamp_Min)
					{
						PlayerComp_CircleSpell->SetRelativeLocation(FVector(MagicCircle_Z_Clamp_Min, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Y, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Z));
					}
				}
			}
			else if (AxisValue < 0)
			{
				if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X < MagicCircle_Z_Clamp_Max)
				{
					PlayerComp_CircleSpell->AddLocalOffset(FVector(-Amt * 15, 0, 0));
					if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X >= MagicCircle_Z_Clamp_Max)
					{
						PlayerComp_CircleSpell->SetRelativeLocation(FVector(MagicCircle_Z_Clamp_Max, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Y, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Z));
					}
				}
			}

		}


	}

}

void APlayerMageCharacter::CharacterJump()
{
	if (bIsDead)return;
	if (IceTrapped)return;
	if (PlayerMageAnimation->bHasJumped)return;

	Jump();
	SFXManager->PlaySoundEffect_3D(ESoundEffects::MageJump);
	PlayerMageAnimation->bHasJumped = true;
}

// ******************************************************************

void APlayerMageCharacter::CharacterAttackOn()
{
	if (bIsDead)return;
	if (IsPreparingSpell != 0)return;
	if (bBlockAttacksTemp)return;
	if (IceTrapped)return;
	if (PlayerMageAnimation->WholeSpellBlend == 0)return;

	bIsAttacking = true;
	PlayerMageAnimation->bIsAttacking = true;
}

void APlayerMageCharacter::CharacterAttackOff()
{
	bIsAttacking = false;
	PlayerMageAnimation->bIsAttacking = false;
}

// ******************************************************************

void APlayerMageCharacter::RMB_On()
{
	bIsRMBPressed = true;

	
	//Cast<AMageMastermind>(Controller)->bShowMouseCursor = false;
}

void APlayerMageCharacter::RMB_Off()
{
	bIsRMBPressed = false;
	PlayerMageAnimation->MovementSpeedY = 0;

	//Cast<AMageMastermind>(Controller)->bShowMouseCursor = true;
}

#pragma endregion

#pragma region Basic Gameplay

void APlayerMageCharacter::InitializeGameplayStats()
{
	CapsuleComp = FindComponentByClass<class UCapsuleComponent>();
	PlayerComp_SpringArm->AttachTo(this->GetRootComponent());
	PlayerComp_SpringArmExtension->AttachTo(PlayerComp_SpringArm);
	PlayerComp_Camera->AttachTo(PlayerComp_SpringArmExtension);
	IceTrapParticle = FindComponentByClass<class UParticleSystemComponent>();

	CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &APlayerMageCharacter::OnOverlapBegin);       // set up a notification for when this component overlaps something
	CapsuleComp->OnComponentEndOverlap.AddDynamic(this, &APlayerMageCharacter::OnOverlapEnd);       // set up a notification for when this component overlaps something

	PlayerMageAnimation = Cast<UMageAnimInstance>(PlayerComp_SkeletalMesh->GetAnimInstance());

	JumpZ_Init = GetCharacterMovement()->JumpZVelocity;
	PlayerMageAnimation->AttackSpeed = 1;

	PlayerMageAnimation->WholeSpellBlend = 1;

	SFXManager = FindComponentByClass<class USoundEffectManager>();

	

	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);
	for (int32 i = 0; i < StaticMeshes.Num(); i++)
	{
		if (StaticMeshes[i]->ComponentHasTag("Spell_Line"))
		{
			PlayerComp_LineSpell = StaticMeshes[i];
		}
		if (StaticMeshes[i]->ComponentHasTag("ConeCollision"))
		{
			PlayerComp_ConeCollider = StaticMeshes[i];
		}
		if (StaticMeshes[i]->ComponentHasTag("Spell_Circle"))
		{
			PlayerComp_CircleSpell = StaticMeshes[i];
		}
	}

	TArray<UMageStats*> Stats;
	GetComponents<UMageStats>(Stats);
	PlayerComp_Stats = Stats[0];

	TArray<UWidgetComponent*> Widgets;
	GetComponents<UWidgetComponent>(Widgets);
	for (size_t i = 0; i < 2; i++)
	{
		if (Widgets[i]->ComponentHasTag("DMG"))
		{
			DamageOverlayComp = Cast<UEnemyHealthBarUI>(Widgets[i]->GetUserWidgetObject());
		}
	}

	

	MaxHealth = PlayerComp_Stats->MaxHealth;
	MaxMana = PlayerComp_Stats->MaxMana;

	CurrentHealth = MaxHealth;
	CurrentMana = MaxMana;

	PlayerComp_Stats->MyMage = Cast<AActor>(this);

	UpdateMana(0);
	UpdateHealth(0);
}

UMageAnimInstance* APlayerMageCharacter::GetAnimationInstance()
{
	return Cast<UMageAnimInstance>(PlayerComp_SkeletalMesh->GetAnimInstance());
}

AMageMastermind* APlayerMageCharacter::GetMagePlayerController()
{
	return MagePlayerController;
}

void APlayerMageCharacter::UpdateMana(float Value)
{
	CurrentMana += Value;
	if (CurrentMana > MaxMana)
	{
		CurrentMana = MaxMana;
	}
	else if (CurrentMana < 0)
	{
		CurrentMana = 0;
	}
}

void APlayerMageCharacter::UpdateHealth(float Value)
{
	CurrentHealth += Value;
	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	else if (CurrentHealth < 0)
	{
		CurrentHealth = 0;
	}

	if (CurrentHealth <= 0)Server_Die();

}

#pragma endregion

#pragma region Combat Related

void APlayerMageCharacter::Local_ShootProjectile(float Damage)
{
	if (bHasSpawnedProjectile)return;
	
	if (!PlayerMageAnimation->Montage_IsPlaying(MageAttackingMontage)) return;
	
	bHasSpawnedProjectile = true;
	bHasSpawnedProjectile_Timer = 0;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = Instigator;
	AActor* SpawnedObject;

	SpawnedObject = GetWorld()->SpawnActor<AActor>(PlayerProjectile, PlayerComp_SkeletalMesh->GetSocketLocation("RightHandMiddle2"), FRotator(0), spawnParams);
	
	FRotator PlayerDir = PlayerComp_SkeletalMesh->GetComponentRotation() + FRotator(0, 90, 0);
	FVector Speed = PlayerDir.Vector() * ProjectileSpeed;

	AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
	
	MyProjectile->FindComponentByClass<class USoundEffectManager>()->PlaySoundEffect_3D(ESoundEffects::ShootBaseProjectile);
	MyProjectile->GrandeDamage = Damage;
	MyProjectile->ApplyForce(Speed);
	MyProjectile->ShooterMagus = this;
	MyProjectile->Lifetime = ProjectileLifetime;
}

void APlayerMageCharacter::TakeDamageFromEnemy(float Damage)
{
	if (bIsDead)return;

	PlayerMageAnimation->GetHurt();
	SFXManager->PlaySoundEffect_3D(ESoundEffects::MageGetHurt);

	if (Role == ROLE_Authority)
		UpdateHealth(-Damage);

	if (Role < ROLE_Authority)
	{
		Cast<AMageMastermind>(GetWorld()->GetFirstPlayerController())->CreateDamageTextAboveActor(this, Damage, 20);
	}
}

void APlayerMageCharacter::Die()
{
	bIsDead = true;
	PlayerMageAnimation->bIsDead = true;
	CharacterAttackOff();
	SFXManager->PlaySoundEffect_3D(ESoundEffects::MageDeath);
}

#pragma endregion

#pragma region Collisions

void APlayerMageCharacter::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("AOE") && OtherActor->GetOwner() != this)
	{
		GLog->Log("AOE STARTED");
		bIsInAOE = true;
		AOETimer = 0;
	}
	if (OtherActor->ActorHasTag("Prison") && OtherActor->GetOwner() != this)
	{
		SFXManager->PlaySoundEffect_3D(ESoundEffects::IceTrap_Impact);
		AddBuffToCharacter(EBuffs::IceTrap_Debuff, PlayerComp_Stats->Frost_IceTrap_Lifetime);
	}
}

void APlayerMageCharacter::OnOverlapEnd(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("AOE") && OtherActor->GetOwner() != this)
	{
		bIsInAOE = false;
		GLog->Log("AOE ENDED");
	}
}

#pragma endregion

void APlayerMageCharacter::SetSpellName(int32 Slot, FString Name)
{
	switch (Slot)
	{
	case 1:
		Spell_1_Name = Name;
		break;
	case 2:
		Spell_2_Name = Name;
		break;
	case 3:
		Spell_3_Name = Name;
		break;
	case 4:
		Spell_4_Name = Name;
		break;
	}
}

#pragma region Spells_Input

void APlayerMageCharacter::PrepareSpell_1()
{
	
	if (IceTrapped)return;

	if (ActiveCD_1 > 0)
	{
		CDMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_NotReadyYet);
	}
	else if (CurrentMana < Spell_1_ManaCost)
	{
		ManaMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_OOM);
	}

	if (CurrentMana >= Spell_1_ManaCost && IsPreparingSpell == 0 && !bBlockAttacksTemp && ActiveCD_1 <= 0)
	{
		IsPreparingSpell = 1;
		RenderSpellPrepGraphics(1, true);
		SFXManager->PlaySoundEffect(ESoundEffects::UI_GenericClick);
	}
}
void APlayerMageCharacter::PrepareSpell_2()
{
	//if (!GetMagePlayerController())return;
	if (IceTrapped)return;

	if (ActiveCD_2 > 0)
	{
		CDMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_NotReadyYet);
	}
	else if (CurrentMana < Spell_2_ManaCost)
	{
		ManaMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_OOM);
	}

	if (CurrentMana >= Spell_2_ManaCost && IsPreparingSpell == 0 && !bBlockAttacksTemp && ActiveCD_2 <= 0)
	{
		IsPreparingSpell = 2;
		RenderSpellPrepGraphics(2, true);
		SFXManager->PlaySoundEffect(ESoundEffects::UI_GenericClick);
	}
}
void APlayerMageCharacter::PrepareSpell_3()
{
	if (IceTrapped)return;
	//if (!GetMagePlayerController())return;
	
	if (ActiveCD_3 > 0)
	{
		CDMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_NotReadyYet);
	}
	else if (CurrentMana < Spell_3_ManaCost)
	{
		ManaMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_OOM);
	}

	if (CurrentMana >= Spell_3_ManaCost && IsPreparingSpell == 0 && !bBlockAttacksTemp && ActiveCD_3<=0)
	{
		IsPreparingSpell = 3;
		RenderSpellPrepGraphics(3, true);
		SFXManager->PlaySoundEffect(ESoundEffects::UI_GenericClick);
	}
}
void APlayerMageCharacter::PrepareSpell_4()
{
	
	if (IceTrapped)return;

	if (ActiveCD_4 > 0)
	{
		CDMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_NotReadyYet);
	}
	else if (CurrentMana < Spell_4_ManaCost)
	{
		ManaMessageTimer = 3.0f;
		SFXManager->PlaySoundEffect(ESoundEffects::Ability_OOM);
	}

	if (CurrentMana >= Spell_4_ManaCost && IsPreparingSpell == 0 && !bBlockAttacksTemp && ActiveCD_4 <= 0)
	{
		IsPreparingSpell = 4;
		RenderSpellPrepGraphics(4, true);
		SFXManager->PlaySoundEffect(ESoundEffects::UI_GenericClick);
		
	}
}

void APlayerMageCharacter::InvokeSpell(EAvailableSpells Spell)
{
	IsPreparingSpell = 0;
	bBlockAttacksTemp = false;

	if (Spell == EAvailableSpells::Frost_IceTrail)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedObject;

		FVector Location = GetActorLocation() + GetActorLocation().ForwardVector.GetSafeNormal() * 7.f - FVector(0, 0, 50);

		SpawnedObject = GetWorld()->SpawnActor<AActor>(PlayerComp_Stats->Frost_IceTrail_Projectile, Location, FRotator(0), spawnParams);

		FRotator PlayerDir = PlayerComp_SkeletalMesh->GetComponentRotation() + FRotator(0, 90, 0);

		FVector Speed = PlayerDir.Vector() * PlayerComp_Stats->Frost_IceTrail_Speed;
		AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
		MyProjectile->ApplyForce(Speed);
		MyProjectile->ShooterMagus = this;
		MyProjectile->Lifetime = PlayerComp_Stats->Frost_IceTrail_Lifetime;
		MyProjectile->SpecialFlags.Add("Ignore Collisions");
		MyProjectile->SpecialFlags.Add("Ice Trail");

		TArray<FHitResult> HitResults;
		//Start Location
		FVector StartLocation = GetActorLocation();
		FVector EndLocation = StartLocation + PlayerDir.Vector() * 500;
		//Collision Channel
		ECollisionChannel ECC = ECollisionChannel::ECC_WorldDynamic;

		//Collision Shape
		FCollisionShape CollisionShape;

		//Set the collision shape to be a sphere with 500 units radius
		CollisionShape.ShapeType = ECollisionShape::Box;
		CollisionShape.SetBox(FVector(300, 60, 90));

		FVector Center = ((EndLocation - StartLocation) / 2) + StartLocation;
		SFXManager->PlaySoundEffect_3D_AtLocation(ESoundEffects::IceTrail_Begin, Center);

		//Actual sweeping
		if (GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, PlayerDir.Quaternion(), ECC, CollisionShape))
		{
			//If we hit any actors - print in the output log their names.
			for (auto It = HitResults.CreateIterator(); It; It++)
			{
				if ((*It).Actor->FindComponentByClass<USkeletalMeshComponent>())
				{
					USkeletalMeshComponent* MySM = (*It).Actor->FindComponentByClass<USkeletalMeshComponent>();

					if (MySM->ComponentHasTag("Magus") && (*It).Actor != this)
					{
						AActor* MageEnemy = (*It).Actor.Get();

						Cast<APlayerMageCharacter>(MageEnemy)->AddBuffToCharacter(EBuffs::IceTrail_Debuff, PlayerComp_Stats->Frost_IceTrail_Debuff_Duration);

						SFXManager->PlaySoundEffect_3D_AtLocation(ESoundEffects::IceTrail_Impact, Center);

						if (!MagePlayerController)
							return;


						Cast<APlayerMageCharacter>(MageEnemy)->AddForceEffect(FVector(0, 0, PlayerComp_Stats->Frost_IceTrail_Force));
						Cast<APlayerMageCharacter>(MageEnemy)->TakeDamageFromEnemy(PlayerComp_Stats->Frost_IceTrail_Damage);
						break;
					}
				}
			}
		}

		//FVector Center = ((EndLocation - StartLocation) / 2) + StartLocation;
		//DrawDebugBox(GetWorld(), Center, FVector(CollisionShape.Box.HalfExtentX, CollisionShape.Box.HalfExtentY, CollisionShape.Box.HalfExtentZ), PlayerDir.Quaternion(), FColor::Green, true);
	}
	else if (Spell == EAvailableSpells::Frost_IceTrap)
	{
		AMageMastermind* Master = MagePlayerController;

		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedObject;

		FVector Location = PlayerComp_CircleSpell->GetComponentLocation();


		SpawnedObject = GetWorld()->SpawnActor<AActor>(PlayerComp_Stats->Frost_IceTrap_Projectile, Location, FRotator(0), spawnParams);

		AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
		MyProjectile->ShooterMagus = this;
		MyProjectile->Lifetime = PlayerComp_Stats->Frost_IceTrap_Lifetime;

		MyProjectile->SpecialFlags.Add("Ignore Bumped");
		MyProjectile->SpecialFlags.Add("Ignore Collisions");
		MyProjectile->SpecialFlags.Add("Ice Trap");

		TArray<FHitResult> HitResults;
		//Start Location
		FVector StartLocation = Location;

		//Collision Channel
		ECollisionChannel ECC = ECollisionChannel::ECC_WorldDynamic;

		//Collision Shape
		FCollisionShape CollisionShape;

		//Set the collision shape to be a sphere with 500 units radius
		CollisionShape.ShapeType = ECollisionShape::Sphere;
		CollisionShape.SetSphere(PlayerComp_Stats->Frost_IceTrap_RegionRadius);

		SFXManager->PlaySoundEffect_3D_AtLocation(ESoundEffects::IceTrap_Begin, StartLocation);

		//Actual sweeping
		//if (GetWorld()->SweepMultiByChannel(HitResults, StartLocation, StartLocation + FVector(1, 0, 0), FQuat::FQuat(), ECC, CollisionShape))
		//{
		//	//If we hit any actors - print in the output log their names.
		//	for (auto It = HitResults.CreateIterator(); It; It++)
		//	{
		//		if ((*It).Actor->FindComponentByClass<USkeletalMeshComponent>())
		//		{
		//			USkeletalMeshComponent* MySM = (*It).Actor->FindComponentByClass<USkeletalMeshComponent>();
		//			if (MySM->ComponentHasTag("Magus") && (*It).Actor != this)
		//			{
		//				AActor* MageEnemy = (*It).Actor.Get();
		//				Cast<APlayerMageCharacter>(MageEnemy)->AddBuffToCharacter(EBuffs::IceTrap_Debuff, PlayerComp_Stats->Frost_IceTrap_Lifetime);
		//				break;
		//			}
		//		}

		//	}
		//}

		//Calculate the center of the sphere
		//FVector CenterOfSphere = StartLocation;
		//Draw the Debug Sphere
		//DrawDebugSphere(GetWorld(), CenterOfSphere, CollisionShape.GetSphereRadius(), 100, FColor::Green, true);
	}
	else if (Spell == EAvailableSpells::Frost_FrostArmor)
	{
		AMageMastermind* Master = MagePlayerController;

		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = this->Instigator;
		AActor* SpawnedObject;

		FVector Location = GetActorLocation();

		SpawnedObject = GetWorld()->SpawnActor<AActor>(PlayerComp_Stats->Frost_FrostArmor_Projectile, Location, FRotator(0), spawnParams);

		AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
		MyProjectile->ShooterMagus = this;
		MyProjectile->Lifetime = PlayerComp_Stats->Frost_FrostArmor_Lifetime;
		MyProjectile->AttachRootComponentToActor(this, "", EAttachLocation::SnapToTarget, true);

		MyProjectile->SpecialFlags.Add("Ignore Bumped");
		MyProjectile->SpecialFlags.Add("Ignore Collisions");
		MyProjectile->SpecialFlags.Add("Frost Armor");

		AddBuffToCharacter(EBuffs::FrostArmor_Buff, PlayerComp_Stats->Frost_FrostArmor_Lifetime);
		SFXManager->PlaySoundEffect_3D(ESoundEffects::FrostArmor_Begin);
	}
	else if (Spell == EAvailableSpells::Forst_GlacierShattering)
	{
		AMageMastermind* Master = MagePlayerController;

		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Instigator;
		AActor* SpawnedObject;

		FVector Location = GetActorLocation();

		SpawnedObject = GetWorld()->SpawnActor<AActor>(PlayerComp_Stats->Frost_GlacierShattering_Projectile, Location - FVector(0, 0, 50), FRotator(0), spawnParams);

		AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
		MyProjectile->ShooterMagus = this;
		MyProjectile->Lifetime = PlayerComp_Stats->Frost_GlacierShattering_Lifetime;

		MyProjectile->SpecialFlags.Add("Ignore Bumped");
		MyProjectile->SpecialFlags.Add("Ignore Collisions");
		MyProjectile->SpecialFlags.Add("Ice Trap");

		TArray<FHitResult> HitResults;
		//Start Location
		FVector StartLocation = Location;

		//Collision Channel
		ECollisionChannel ECC = ECollisionChannel::ECC_WorldDynamic;

		//Collision Shape
		FCollisionShape CollisionShape;

		//Set the collision shape to be a sphere with 500 units radius
		CollisionShape.ShapeType = ECollisionShape::Sphere;
		CollisionShape.SetSphere(PlayerComp_Stats->Frost_GlacierShattering_CircleRadius);


		SFXManager->PlaySoundEffect_3D_AtLocation(ESoundEffects::GlacierShattering_Begin, StartLocation);

		//Actual sweeping
		if (GetWorld()->SweepMultiByChannel(HitResults, StartLocation, StartLocation + FVector(1, 0, 0), FQuat::FQuat(), ECC, CollisionShape))
		{
			//If we hit any actors - print in the output log their names.
			for (auto It = HitResults.CreateIterator(); It; It++)
			{
				if ((*It).Actor->FindComponentByClass<USkeletalMeshComponent>())
				{
					USkeletalMeshComponent* MySM = (*It).Actor->FindComponentByClass<USkeletalMeshComponent>();
					if (MySM->ComponentHasTag("Magus") && (*It).Actor != this)
					{
						AActor* MageEnemy = (*It).Actor.Get();
						Cast<APlayerMageCharacter>(MageEnemy)->AddBuffToCharacter(EBuffs::GlacierShattering_Debuff, PlayerComp_Stats->Frost_GlacierShattering_Lifetime);

						break;
					}
				}

			}
		}
	}


}

void  APlayerMageCharacter::ResetMageBlend()
{
	PlayerMageAnimation->WholeSpellBlend = 1;
	bBlockAttacksTemp = false;
}

void APlayerMageCharacter::ReplicateSpellEffects(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster)
{
	if (Role < ROLE_Authority)
	{
		if (SpellID == 1)
		{
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.Instigator = Instigator;
			AActor* SpawnedObject;
			
			SpawnedObject = GetWorld()->SpawnActor<AActor>(Caster->MagePlayerController->Frost_IceTrail_Projectile, SpawnLocation, FRotator(0), spawnParams);

			FRotator PlayerDir = Caster->PlayerComp_SkeletalMesh->GetComponentRotation() + FRotator(0, 90, 0);
			FVector Speed = PlayerDir.Vector() * Caster->MagePlayerController->Frost_IceTrail_Speed;
			AAutoAttackProjectile* MyProjectile = Cast<AAutoAttackProjectile>(SpawnedObject);
			MyProjectile->ApplyForce(Speed);
			MyProjectile->ShooterMagus = this;
			MyProjectile->Lifetime = Caster->MagePlayerController->Frost_IceTrail_Lifetime;
			MyProjectile->SpecialFlags.Add("Ignore Collisions");
			MyProjectile->SpecialFlags.Add("Ice Trail");
		}
	}
}

void APlayerMageCharacter::StartCasting()
{
	if (!MagePlayerController) return;

	switch (IsPreparingSpell)
	{
	case 1:
		Server_InvokeSpell(MagePlayerController->Spell_1);
		ActiveCD_1 = MagePlayerController->Cooldown_Spell_1;
		break;
	case 2:
		Server_InvokeSpell(MagePlayerController->Spell_2);
		ActiveCD_2 = MagePlayerController->Cooldown_Spell_2;
		break;
	case 3:
		Server_InvokeSpell(MagePlayerController->Spell_3);
		ActiveCD_3 = MagePlayerController->Cooldown_Spell_3;
		break;
	case 4:
		Server_InvokeSpell(MagePlayerController->Spell_4);
		ActiveCD_4 = MagePlayerController->Cooldown_Spell_4;
		break;
	}

	IsPreparingSpell = 0;
}

void APlayerMageCharacter::RenderSpellPrepGraphics(int32 Slot, bool bShouldRender)
{
	if (!MagePlayerController)return;
	

	TEnumAsByte<EAvailableSpells> CurrentSpell = EAvailableSpells::None;
	if (Slot == 1)
	{
		if (!MagePlayerController->Spell_1)return;
		CurrentSpell = MagePlayerController->Spell_1;
	}
	if (Slot == 2)
	{
		if (!MagePlayerController->Spell_2)return;
		CurrentSpell = MagePlayerController->Spell_2;
	}
	if (Slot == 3)
	{
		if (!MagePlayerController->Spell_3)return;
		CurrentSpell = MagePlayerController->Spell_3;
	}
	if (Slot == 4)
	{
		if (!MagePlayerController->Spell_4)return;
		CurrentSpell = MagePlayerController->Spell_4;
	}
	
	
	if (CurrentSpell == EAvailableSpells::Frost_IceTrail)
	{
		PlayerComp_LineSpell->SetVisibility(bShouldRender);
		UMaterial* LineMat = MagePlayerController->Frost_IceTrail_LinePlaneMaterial;
		PlayerComp_LineSpell->SetMaterial(0, LineMat);
	}
	else if (CurrentSpell == EAvailableSpells::Frost_IceTrap)
	{
		PlayerComp_CircleSpell->SetVisibility(bShouldRender);
		UMaterial* CircleMat = MagePlayerController->Frost_IceTrap_CirclePlaneMaterial;
		PlayerComp_CircleSpell->SetMaterial(0, CircleMat);
		bIsManipulatingCircle = bShouldRender;
		MagicCircle_Z_Clamp_Max = MagePlayerController->Frost_IceTrap_MaxRange;
		PlayerComp_CircleSpell->SetWorldScale3D(FVector(MagePlayerController->Frost_IceTrap_RegionRadius/40, MagePlayerController->Frost_IceTrap_RegionRadius/40, PlayerComp_CircleSpell->GetComponentScale().Z));
		
		if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X >= MagicCircle_Z_Clamp_Max)
		{
			PlayerComp_CircleSpell->SetRelativeLocation(FVector(MagicCircle_Z_Clamp_Max, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Y, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Z));
		}

		CameraCircleGoingUpOrDown = 0;
	}
	else if (CurrentSpell == EAvailableSpells::Frost_FrostArmor)
	{
		if(bShouldRender)
		InstaExecute(Slot);
	}
	else if (CurrentSpell == EAvailableSpells::Forst_GlacierShattering)
	{
		PlayerComp_CircleSpell->SetVisibility(bShouldRender);
		
		UMaterial* CircleMat = MagePlayerController->Frost_GlacierShattering_CirclePlaneMaterial;
		PlayerComp_CircleSpell->SetMaterial(0, CircleMat);
		
		MagicCircle_Z_Clamp_Max = MagePlayerController->Frost_GlacierShattering_MaxRange;
		PlayerComp_CircleSpell->SetWorldScale3D(FVector(MagePlayerController->Frost_GlacierShattering_CircleRadius / 40, MagePlayerController->Frost_GlacierShattering_CircleRadius / 40, PlayerComp_CircleSpell->GetComponentScale().Z));

		if (PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().X >= MagicCircle_Z_Clamp_Max)
		{
			PlayerComp_CircleSpell->SetRelativeLocation(FVector(MagicCircle_Z_Clamp_Max, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Y, PlayerComp_CircleSpell->GetRelativeTransform().GetLocation().Z));
		}

	}	
}

void APlayerMageCharacter::InstaExecute(int32 Slot)
{
	if (Slot == 1)
	{
		ExecuteSpell_1();
	}
	else if (Slot == 2)
	{
		ExecuteSpell_2();
	}
	else if (Slot == 3)
	{
		ExecuteSpell_3();
	}
	else if (Slot == 4)
	{
		ExecuteSpell_4();
	}
}

void APlayerMageCharacter::StartPlayingCastAnimation(int32 Slot)
{
	if (!MagePlayerController)return;

	TEnumAsByte<EAvailableSpells> Spell;
	switch (Slot)
	{
	case 1:
		if (!MagePlayerController->Spell_1)return;
		Spell = MagePlayerController->Spell_1;
		break;
	case 2:
		if (!MagePlayerController->Spell_2)return;
		Spell = MagePlayerController->Spell_2;
		break;
	case 3:
		if (!MagePlayerController->Spell_3)return;
		Spell = MagePlayerController->Spell_3;
		break;
	case 4:
		if (!MagePlayerController->Spell_4)return;
		Spell = MagePlayerController->Spell_4;
		break;
	}


	switch (Spell)
	{
	case EAvailableSpells::Frost_IceTrail:
		Server_PlayReplicatedSpellAnimation(0, this);
		break;
	case EAvailableSpells::Frost_IceTrap:
		Server_PlayReplicatedSpellAnimation(1, this);
		break;
	case EAvailableSpells::Frost_FrostArmor:
		Server_PlayReplicatedSpellAnimation(2, this);
		break;
	case EAvailableSpells::Forst_GlacierShattering:
		Server_PlayReplicatedSpellAnimation(3, this);
		break;
	}
}

void APlayerMageCharacter::PlayReplicatedSpellAnimation(int32 AnimationID, APlayerMageCharacter* Mage)
{
	if (AnimationID == 0)
	{
		SFXManager->PlaySoundEffect_3D(ESoundEffects::IceTrail_Shout);
		PlayerMageAnimation->WholeSpellBlend = 0;
		PlayerMageAnimation->Spell_A_Anim();
	}
	if (AnimationID == 1)
	{
		SFXManager->PlaySoundEffect_3D(ESoundEffects::IceTrap_Shout);
		PlayerMageAnimation->WholeSpellBlend = 0;
		PlayerMageAnimation->Spell_A_Anim();
	}
	if (AnimationID == 2)
	{
		SFXManager->PlaySoundEffect_3D(ESoundEffects::FrostArmor_Shout);
		PlayerMageAnimation->WholeSpellBlend = 0;
		PlayerMageAnimation->Spell_SelfBuff_Anim();
	}
	if (AnimationID == 3)
	{
		SFXManager->PlaySoundEffect_3D(ESoundEffects::GlacierShattering_Shout);
		PlayerMageAnimation->WholeSpellBlend = 0;
		PlayerMageAnimation->Spell_AoeUlti_Anim();
	}
}

void APlayerMageCharacter::ExecuteSpell_1()
{
	if (IceTrapped)return;

	if (IsPreparingSpell == 1 && !bBlockAttacksTemp)
	{
		RenderSpellPrepGraphics(1, false);
		UpdateMana(-Spell_1_ManaCost);
		bBlockAttacksTemp = true;
		StartPlayingCastAnimation(1);
	}
}
void APlayerMageCharacter::ExecuteSpell_2()
{
	//if (!GetMagePlayerController())return;
	if (IceTrapped)return;

	if (IsPreparingSpell == 2 && !bBlockAttacksTemp)
	{
		RenderSpellPrepGraphics(2, false);
		UpdateMana(-Spell_2_ManaCost);
		bBlockAttacksTemp = true;
		StartPlayingCastAnimation(2);
		
	}
}
void APlayerMageCharacter::ExecuteSpell_3()
{
	//if (!GetMagePlayerController())return;
	if (IceTrapped)return;

	if (IsPreparingSpell == 3 && !bBlockAttacksTemp)
	{
		RenderSpellPrepGraphics(3, false);
		UpdateMana(-Spell_3_ManaCost);
		bBlockAttacksTemp = true;
		StartPlayingCastAnimation(3);

	}
}
void APlayerMageCharacter::ExecuteSpell_4()
{
	//if (!GetMagePlayerController())return;
	if (IceTrapped)return;

	if (IsPreparingSpell == 4 && !bBlockAttacksTemp)
	{
		RenderSpellPrepGraphics(4, false);
		UpdateMana(-Spell_4_ManaCost);
		bBlockAttacksTemp = true;
		StartPlayingCastAnimation(4);

	}
}

#pragma endregion

void APlayerMageCharacter::AddForceEffect(FVector Power)
{
	TArray<UCapsuleComponent*> Caps;
	GetComponents<UCapsuleComponent>(Caps);
	for (int32 i = 0; i < Caps.Num(); i++)
	{
		Caps[0]->SetSimulatePhysics(true);
	}
	Caps[0]->AddImpulse(Power, "", true);
	JumpResetterTimer = 1.0f;
	
}

void APlayerMageCharacter::AddBuffToCharacter(TEnumAsByte<EBuffs> BuffToApply, float Duration)
{
	PlayerComp_Stats->Server_UpdateBuffEffect(BuffToApply, false);
	
	if (Role < ROLE_Authority)
	{
		switch (BuffToApply)
		{
		case IceTrail_Debuff:

			if(MagePlayerController)
				MagePlayerController->GameplayUI->AddNewBuff(PlayerComp_Stats->Frost_IceTrail_Icon, Duration, IceTrail_Debuff);

			//PlayerComp_Stats->AddNewBuff(PlayerComp_Stats->Frost_IceTrail_Icon, Duration, IceTrail_Debuff);
			break;
		case IceTrap_Debuff:

			if (MagePlayerController)
				MagePlayerController->GameplayUI->AddNewBuff(PlayerComp_Stats->Frost_IceTrap_Icon, Duration, IceTrap_Debuff);

			//PlayerComp_Stats->AddNewBuff(PlayerComp_Stats->Frost_IceTrap_Icon, Duration, IceTrap_Debuff);
			break;
		case FrostArmor_Buff:

			if (MagePlayerController)
				MagePlayerController->GameplayUI->AddNewBuff(PlayerComp_Stats->Frost_FrostArmor_Icon, Duration, FrostArmor_Buff);

			//PlayerComp_Stats->AddNewBuff(PlayerComp_Stats->Frost_FrostArmor_Icon, Duration, FrostArmor_Buff);
			break;
		case GlacierShattering_Debuff:

			if (MagePlayerController)
				MagePlayerController->GameplayUI->AddNewBuff(PlayerComp_Stats->Frost_GlacierShattering_Icon, Duration, GlacierShattering_Debuff);

			//PlayerComp_Stats->AddNewBuff(PlayerComp_Stats->Frost_GlacierShattering_Icon, Duration, GlacierShattering_Debuff);
			break;
		default:
			break;
		}
	}
}

#pragma region Networking Functions

// ************************************************************************************************************ //
// I Stash all my Networking Related Functions here, since they are mostly Server Multicasting and Validators..
// ************************************************************************************************************ //

void APlayerMageCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(APlayerMageCharacter, CurrentHealth);
	DOREPLIFETIME(APlayerMageCharacter, bIsAttacking);
	DOREPLIFETIME(APlayerMageCharacter, bHasSpawnedProjectile);
	DOREPLIFETIME(APlayerMageCharacter, bHasSpawnedProjectile_Timer);
	DOREPLIFETIME(APlayerMageCharacter, bHasSpawnedProjectile_Cooldown);
	
}


void APlayerMageCharacter::Server_Die_Implementation()
{
	Multicast_Die();
}
bool APlayerMageCharacter::Server_Die_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_Die_Implementation()
{
	Die();
}

void APlayerMageCharacter::Server_CalculateDamageFromProj_Implementation()
{
	float BonusDmg = FMath::FRandRange(PlayerComp_Stats->SpellPower_Extra_Percentage_Min, PlayerComp_Stats->SpellPower_Extra_Percentage_Max) * PlayerComp_Stats->SpellPower;
	float FinalDamage = PlayerComp_Stats->SpellPower + BonusDmg;

	int32 Proc = FMath::RandRange(1, 100);

	if (Proc <= PlayerComp_Stats->SpellCrit)
	{
		FinalDamage *= PlayerComp_Stats->CritMultiplier;
	}
	FinalDamageFromProjectile = FinalDamage;
}
bool APlayerMageCharacter::Server_CalculateDamageFromProj_Validate()
{
	return true;
}

void APlayerMageCharacter::Server_ShootProjectile_Implementation(float FinalDamage)
{
	Multicast_ShootProjectile(FinalDamageFromProjectile);
}
bool APlayerMageCharacter::Server_ShootProjectile_Validate(float FinalDamage)
{
	return true;
}
void APlayerMageCharacter::Multicast_ShootProjectile_Implementation(float Damage)
{
	Local_ShootProjectile(Damage);
}

void APlayerMageCharacter::Server_TakeDamageFromEnemy_Implementation(float Damage)
{
	TakeDamageFromEnemy(Damage);
}
bool APlayerMageCharacter::Server_TakeDamageFromEnemy_Validate(float Damage)
{
	return true;
}
void APlayerMageCharacter::Multicast_TakeDamageFromEnemy_Implementation(float Damage)
{
	TakeDamageFromEnemy(Damage);
}

void APlayerMageCharacter::Server_JumpFixer_Implementation()
{
	Multicast_JumpFixer();
}
bool APlayerMageCharacter::Server_JumpFixer_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_JumpFixer_Implementation()
{
	JumpFixer();
}

void APlayerMageCharacter::Server_CharacterMoveForward_Implementation(float AxisValue)
{
	Multicast_CharacterMoveForward(AxisValue);
}
bool APlayerMageCharacter::Server_CharacterMoveForward_Validate(float AxisValue)
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterMoveForward_Implementation(float AxisValue)
{
	CharacterMoveForward(AxisValue);
}


void APlayerMageCharacter::Server_CharacterMouseLookUpDown_Implementation(float AxisValue)
{
	Multicast_CharacterMouseLookUpDown(AxisValue);
}
bool APlayerMageCharacter::Server_CharacterMouseLookUpDown_Validate(float AxisValue)
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterMouseLookUpDown_Implementation(float AxisValue)
{
	CharacterMouseLookUpDown(AxisValue);
}


void APlayerMageCharacter::Server_CharacterJump_Implementation()
{
	Multicast_CharacterJump();
}
bool APlayerMageCharacter::Server_CharacterJump_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterJump_Implementation()
{
	CharacterJump();
}



void APlayerMageCharacter::Server_CharacterMoveRight_Implementation(float AxisValue)
{
	Multicast_CharacterMoveRight(AxisValue);
}
bool APlayerMageCharacter::Server_CharacterMoveRight_Validate(float AxisValue)
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterMoveRight_Implementation(float AxisValue)
{
	CharacterMoveRight(AxisValue);
}

void APlayerMageCharacter::Server_CharacterAttackOn_Implementation()
{
	Multicast_CharacterAttackOn();
}
bool APlayerMageCharacter::Server_CharacterAttackOn_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterAttackOn_Implementation()
{
	CharacterAttackOn();
}

void APlayerMageCharacter::Server_CharacterAttackOff_Implementation()
{
	Multicast_CharacterAttackOff();
}
bool APlayerMageCharacter::Server_CharacterAttackOff_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_CharacterAttackOff_Implementation()
{
	CharacterAttackOff();
}

void APlayerMageCharacter::Server_ReplicateSpellEffects_Implementation(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster)
{
	Multicast_ReplicateSpellEffects(SpellID, SpawnLocation, Direction, Caster);
}
bool APlayerMageCharacter::Server_ReplicateSpellEffects_Validate(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster)
{
	return true;
}
void APlayerMageCharacter::Multicast_ReplicateSpellEffects_Implementation(int32 SpellID, FVector SpawnLocation, FVector Direction, APlayerMageCharacter* Caster)
{
	ReplicateSpellEffects(SpellID, SpawnLocation, Direction, Caster);
}

void APlayerMageCharacter::Server_InvokeSpell_Implementation(EAvailableSpells Spell)
{
	Multicast_InvokeSpell(Spell);
}
bool APlayerMageCharacter::Server_InvokeSpell_Validate(EAvailableSpells Spell)
{
	return true;
}
void APlayerMageCharacter::Multicast_InvokeSpell_Implementation(EAvailableSpells Spell)
{
	InvokeSpell(Spell);
}

void APlayerMageCharacter::Server_ResetMageBlend_Implementation()
{
	Multicast_ResetMageBlend();
}
bool APlayerMageCharacter::Server_ResetMageBlend_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_ResetMageBlend_Implementation()
{
	ResetMageBlend();
}

void APlayerMageCharacter::Server_StartCasting_Implementation()
{
	Multicast_StartCasting();
}
bool APlayerMageCharacter::Server_StartCasting_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_StartCasting_Implementation()
{
	StartCasting();
}

void APlayerMageCharacter::Server_PlayReplicatedSpellAnimation_Implementation(int32 AnimationID, APlayerMageCharacter* Mage)
{
	Multicast_PlayReplicatedSpellAnimation(AnimationID, Mage);
}
bool APlayerMageCharacter::Server_PlayReplicatedSpellAnimation_Validate(int32 AnimationID, APlayerMageCharacter* Mage)
{
	return true;
}
void APlayerMageCharacter::Multicast_PlayReplicatedSpellAnimation_Implementation(int32 AnimationID, APlayerMageCharacter* Mage)
{
	PlayReplicatedSpellAnimation(AnimationID, Mage);
}

void APlayerMageCharacter::Server_ExecuteSpell_1_Implementation()
{
	Multicast_ExecuteSpell_1();
}
bool APlayerMageCharacter::Server_ExecuteSpell_1_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_ExecuteSpell_1_Implementation()
{
	ExecuteSpell_1();
}

void APlayerMageCharacter::Server_ExecuteSpell_2_Implementation()
{
	Multicast_ExecuteSpell_2();
}
bool APlayerMageCharacter::Server_ExecuteSpell_2_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_ExecuteSpell_2_Implementation()
{
	ExecuteSpell_2();
}

void APlayerMageCharacter::Server_ExecuteSpell_3_Implementation()
{
	Multicast_ExecuteSpell_3();
}
bool APlayerMageCharacter::Server_ExecuteSpell_3_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_ExecuteSpell_3_Implementation()
{
	ExecuteSpell_3();
}

void APlayerMageCharacter::Server_ExecuteSpell_4_Implementation()
{
	Multicast_ExecuteSpell_4();
}
bool APlayerMageCharacter::Server_ExecuteSpell_4_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_ExecuteSpell_4_Implementation()
{
	ExecuteSpell_4();
}

void APlayerMageCharacter::Server_PrepareSpell_1_Implementation()
{
	Multicast_PrepareSpell_1();
}
bool APlayerMageCharacter::Server_PrepareSpell_1_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_PrepareSpell_1_Implementation()
{
	PrepareSpell_1();
}

void APlayerMageCharacter::Server_PrepareSpell_2_Implementation()
{
	Multicast_PrepareSpell_2();
}
bool APlayerMageCharacter::Server_PrepareSpell_2_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_PrepareSpell_2_Implementation()
{
	PrepareSpell_2();
}

void APlayerMageCharacter::Server_PrepareSpell_3_Implementation()
{
	Multicast_PrepareSpell_3();
}
bool APlayerMageCharacter::Server_PrepareSpell_3_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_PrepareSpell_3_Implementation()
{
	PrepareSpell_3();
}

void APlayerMageCharacter::Server_PrepareSpell_4_Implementation()
{
	Multicast_PrepareSpell_4();
}
bool APlayerMageCharacter::Server_PrepareSpell_4_Validate()
{
	return true;
}
void APlayerMageCharacter::Multicast_PrepareSpell_4_Implementation()
{
	PrepareSpell_4();
}

#pragma endregion