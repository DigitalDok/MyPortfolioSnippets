// Fill out your copyright notice in the Description page of Project Settings.

#include "RoguelikeSurvival.h"
#include "RoguelikeChar.h"
#include "ZombieAnimInstance.h"
#include "ZombieCharacter.h"
#include "DrawDebugHelpers.h"
#include "PlayerDamageType.h"
#include "RoguelikeGameInstance.h"
#include "ZombieSpawnPoint.h"

#pragma region Base Functions

ARoguelikeChar::ARoguelikeChar()
{

	PrimaryActorTick.bCanEverTick = true;

	// ----- Create some of the needed Components ------------ //
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCam"));
	CharacterMesh = GetMesh();
	GunPositionHelperComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyGunHelper"));
	PlayerMachineGun = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMachineGun"));
}

void ARoguelikeChar::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<ADokController>(Controller);

	SpawnCharInRandomTile();
	SpawnZombiesSpawnPoints();
	SpawnPowerUps();
	SpawnHealthKits();
	SpawnDDs();

	GameplayComponentReferencing();

	PlayerMachineGun->StaticMesh = MachineGunMesh;
	PlayerMachineGun->AttachTo(CharacterMesh, "GunSocket", EAttachLocation::SnapToTarget, false);

	PlayerAnimationInstance = Cast<UHeroAnimInstance>(CharacterMesh->GetAnimInstance());

	URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());
	GameInstance->Character = this;

	GameInstance->StartGameInstance();


	GNearClippingPlane = 0.1f;
	InitializeGameplayStats();


}

void ARoguelikeChar::SpawnCharInRandomTile()
{

	if (PlayerController)
	{

		FVector SpawnPosition;

		TArray<AActor*> AllStaticMeshes;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllStaticMeshes);


		bool SpawnLocationFound = false;

		while (!SpawnLocationFound)
		{
			int32 RandomIndex = FMath::RandRange(0, AllStaticMeshes.Num() - 1);
			if (AllStaticMeshes[RandomIndex]->ActorHasTag(TEXT("Floor")))
			{
				SpawnLocationFound = true;
				SpawnPosition = AllStaticMeshes[RandomIndex]->GetActorLocation();
			}
		}

		SpawnPosition.Z += 250;

		this->SetActorLocation(SpawnPosition);
	}
}

void ARoguelikeChar::SpawnPowerUps()
{
	if (PlayerController)
	{
		FVector SpawnPosition;
		TArray<AActor*> AllStaticMeshes;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllStaticMeshes);

		URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());

		int32 SpawnedAmmoBoxes = 0;

		//Spawning AmmoBoxes...
		do
		{
			//Selecting a random valid tile
			int32 RandomTileIndex = FMath::RandRange(0, AllStaticMeshes.Num() - 1);

			if (AllStaticMeshes[RandomTileIndex]->ActorHasTag(FName("PowerUpTile")))
			{
				SpawnPosition = AllStaticMeshes[RandomTileIndex]->GetActorLocation();
				SpawnPosition.Z += 50;
				SpawnedAmmoBoxes++;

				//Declaring spawning parameters
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;

				//Spawning the ammo box and updating it's ammo type with a random type

				AAmmoBox* AmmoBox = GetWorld()->SpawnActor<AAmmoBox>(GameInstance->AmmoBoxBP, SpawnPosition, FRotator(), SpawnParams);

				int32 AmmoBoxType = FMath::RandRange(0, 2);
				switch (AmmoBoxType)
				{
				case 1:
				{
					AmmoBox->AmmoType = EAmmoType::Incendiary;
					
					break;
				}
				case 2:
				{
					AmmoBox->AmmoType = EAmmoType::Frost;
					
					break;
				}
				default:
					break;
				}
				AllStaticMeshes[RandomTileIndex]->Tags.Add(FName("HasPowerup"));
				//Updating the visual effects of the ammo box to match the ammo type
				AmmoBox->SetAmmoBoxMaterialAndParticleEmitter();

			}
		} while (SpawnedAmmoBoxes < GameInstance->MaxAmmoBoxes);

		
		GLog->Log("Total ammo boxes spawned:" + FString::FromInt(SpawnedAmmoBoxes));

		

	}
}

void ARoguelikeChar::SpawnHealthKits()
{
	if(PlayerController)
	{
		FVector SpawnPosition;
		TArray<AActor*> AllStaticMeshes;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllStaticMeshes);

		URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());

		int32 SpawnedFirstAidKits = 0;

		do
		{
			int32 RandomTileIndex = FMath::RandRange(0, AllStaticMeshes.Num() - 1);

			AStaticMeshActor* StaticMesh = Cast<AStaticMeshActor>(AllStaticMeshes[RandomTileIndex]);


			if ( (StaticMesh->ActorHasTag(FName("PowerUpTile"))) && !(StaticMesh->Tags.Contains(FName("HasPowerup"))) ) 
			{
				SpawnPosition = AllStaticMeshes[RandomTileIndex]->GetActorLocation();
				SpawnPosition.Z += 50;
				SpawnedFirstAidKits++;

				//Declaring spawning parameters
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;

				AFirstAidKit* Kit = GetWorld()->SpawnActor<AFirstAidKit>(GameInstance->FirstAidKitBP, SpawnPosition, FRotator(), SpawnParams);

				//If the kit is for overtime heal mark it as such and update its material
				/*if(FMath::RandBool())
				{
					Kit->bIsOvertimeHeal = true;
					Kit->UpdateMaterial();
				}*/

			}
		} while (SpawnedFirstAidKits<GameInstance->MaxFirstAidKits);
	}
}

void ARoguelikeChar::SpawnDDs()
{
	if (PlayerController)
	{
		FVector SpawnPosition;
		TArray<AActor*> AllStaticMeshes;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllStaticMeshes);

		URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());

		int32 SpawnedDoubleDamagePowerups = 0;
		//Spawning Double Damage...
		do
		{
			//Selecting a random valid tile
			int32 RandomTileIndex = FMath::RandRange(0, AllStaticMeshes.Num() - 1);


			if (AllStaticMeshes[RandomTileIndex]->ActorHasTag(FName("PowerUpTile")) && !AllStaticMeshes[RandomTileIndex]->Tags.Contains(FName("HasPowerup")))
			{
				SpawnPosition = AllStaticMeshes[RandomTileIndex]->GetActorLocation();
				SpawnPosition.Z += 100;
				SpawnedDoubleDamagePowerups++;

				//Declaring spawning parameters
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;

				ADoubleDamagePowerup* DD = GetWorld()->SpawnActor<ADoubleDamagePowerup>(GameInstance->DoubleDamageBP, SpawnPosition, FRotator(), SpawnParams);
				AllStaticMeshes[RandomTileIndex]->Tags.Add(FName("HasPowerup"));
			}


		} while (SpawnedDoubleDamagePowerups < GameInstance->MaxDDPowerups);

	}
}

void ARoguelikeChar::SpawnZombiesSpawnPoints()
{
	if (PlayerController)
	{
		FVector SpawnPosition;

		TArray<AActor*> AllStaticMeshes;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllStaticMeshes);

		URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());

		int32 SpawnedPoints = 0;

		for (int32 i = 0; i < AllStaticMeshes.Num() && (SpawnedPoints < GameInstance->MaxZombiesSpawnPoints); i++)
		{
			if (AllStaticMeshes[i]->ActorHasTag(FName("Spike")))
			{

				SpawnPosition = AllStaticMeshes[i]->GetActorLocation();
				SpawnPosition.Z += 270;

				SpawnedPoints++;

				FActorSpawnParameters SpawnParams;
				//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;

				GLog->Log("Found a spike");
				AZombieSpawnPoint* SpawnPoint = GetWorld()->SpawnActor<AZombieSpawnPoint>(GameInstance->SpawnPointBP, SpawnPosition, FRotator(0, 0, 0), SpawnParams);
			}
		}


	}
}

void ARoguelikeChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		DeathCameraMovement(DeltaTime);
	}
	else
	{

		PlayerCamera->PostProcessSettings.SceneColorTint = FLinearColor(1.f, CurrentHealth / MaxHealth, CurrentHealth / MaxHealth);

		// Reset Jump Check
		if (PlayerAnimationInstance->bHasJumped)
		{
			PlayerAnimationInstance->bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
			if (PlayerAnimationInstance->bIsGrounded)PlayerAnimationInstance->bHasJumped = false;
		}

		// Aiming Mechanic
		if (bIsAiming)
		{
			if (PlayerCamera->FieldOfView > FOVLowLimit)
			{
				PlayerCamera->FieldOfView -= FOVDropRate * DeltaTime;
			}
		}
		else
		{
			if (PlayerCamera->FieldOfView < FOVHighLimit)
			{
				PlayerCamera->FieldOfView += FOVDropRate * DeltaTime;
			}
		}

		// Sprinting Mechanics
		ChangeStamina(StaminaRate * DeltaTime);


		// Laser-Specific Code
		if (bIsAiming)
			GenerateLaserBeam();


	}
}

#pragma endregion Constructor | BaginPlay | Tick Function

#pragma region	Input

void ARoguelikeChar::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ARoguelikeChar::CharacterMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ARoguelikeChar::CharacterMoveRight);

	InputComponent->BindAxis("Turn", this, &ARoguelikeChar::CharacterMouseTurn);
	InputComponent->BindAxis("LookUp", this, &ARoguelikeChar::CharacterMouseLookUpDown);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ARoguelikeChar::CharacterJump);
	InputComponent->BindAction("Shoot", IE_Pressed, this, &ARoguelikeChar::CharacterShoot);

	InputComponent->BindAction("Aim", IE_Pressed, this, &ARoguelikeChar::CharacterHoldAim);
	InputComponent->BindAction("Aim", IE_Released, this, &ARoguelikeChar::CharacterReleaseAim);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ARoguelikeChar::CharacterHoldSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ARoguelikeChar::CharacterReleaseSprint);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ARoguelikeChar::Reload);
	InputComponent->BindAction("Cycle", IE_Pressed, this, &ARoguelikeChar::Cycle);
}


// ----------- Movement Input -------------- //


void ARoguelikeChar::CharacterMoveForward(float AxisValue)
{
	if (bIsDead)return;
	if (bIsAiming || PlayerAnimationInstance->bIsReloading)
	{
		PlayerAnimationInstance->MovementSpeedX = 0;
		FRotator X = PlayerCamera->GetForwardVector().Rotation();
		CharacterMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
		return;
	}

	MoveDirY = AxisValue;
	float AxisVal = FMath::Abs(AxisValue);
	PlayerAnimationInstance->MovementSpeedX = AxisVal;

	if (AxisVal > 0.05f)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds());


	}

	FRotator X = PlayerCamera->GetForwardVector().Rotation();
	if (bIsSprinting)
	{
		FVector ViewDirection = FVector(MoveDirY, MoveDirX, 0);
		CharacterMesh->SetWorldRotation(FRotator(0, ViewDirection.Rotation().Yaw + X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
	}
	else
	{
		FRotator X = PlayerCamera->GetForwardVector().Rotation();
		CharacterMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
	}

}

void ARoguelikeChar::CharacterMoveRight(float AxisValue)
{
	if (bIsDead)return;
	if (bIsAiming || PlayerAnimationInstance->bIsReloading)
	{
		PlayerAnimationInstance->MovementSpeedY = 0;
		FRotator X = PlayerCamera->GetForwardVector().Rotation();
		CharacterMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
		return;
	}

	MoveDirX = AxisValue;
	float AxisVal = FMath::Abs(AxisValue);
	PlayerAnimationInstance->MovementSpeedY = AxisVal;
	if (AxisVal > 0.05f)
	{
		AddMovementInput(GetActorRightVector(), AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds());


	}
	FRotator X = PlayerCamera->GetForwardVector().Rotation();
	if (bIsSprinting)
	{
		FVector ViewDirection = FVector(MoveDirY, MoveDirX, 0);
		CharacterMesh->SetWorldRotation(FRotator(0, ViewDirection.Rotation().Yaw + X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
	}
	else
	{

		CharacterMesh->SetWorldRotation(FRotator(0, X.Yaw, 0));
		CharacterMesh->AddWorldRotation(FRotator(0, -90, 0));
	}

}


// ----------- Turning Input -------------- //

void ARoguelikeChar::CharacterMouseTurn(float AxisValue)
{
	if (bIsDead)return;
	if (FMath::Abs(AxisValue) > 0.005f)
	{
		AddControllerYawInput(AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds());
	}

}

void ARoguelikeChar::CharacterMouseLookUpDown(float AxisValue)
{
	if (bIsDead)return;
	if (FMath::Abs(AxisValue) > 0.005f)
	{

		float Amt = AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds();

		if (AxisValue > 0)
		{
			if (PlayerSpringArm->GetComponentRotation().Pitch + Amt <= -ClampingAngle)
				return;
			else
			{
				AddControllerPitchInput(Amt);
			}
		}
		else if (AxisValue < 0)
		{
			if (PlayerSpringArm->GetComponentRotation().Pitch + Amt >= ClampingAngle)
				return;
			else
			{
				AddControllerPitchInput(Amt);
			}
		}



	}

}


// ----------- Action Input -------------- //

void ARoguelikeChar::CharacterJump()
{
	if (bIsDead)return;

	if (bIsAiming || PlayerAnimationInstance->bIsReloading)
		return;

	if (bIsSprinting && !PlayerAnimationInstance->bHasJumped)
	{
		ChangeStamina(-20);
	}

	Jump();
	PlayerAnimationInstance->bHasJumped = true;
}

void ARoguelikeChar::CharacterShoot()
{
	if (bIsDead)return;
	if (!bIsAiming)return;

	if (BulletsLeft_A == 0 && CurrentBulletEquipped == 0 && BulletsLeft_A_Total > 0)
	{
		Reload();
	}
	else if (BulletsLeft_B == 0 && CurrentBulletEquipped == 1 && BulletsLeft_B_Total > 0)
	{
		Reload();
	}
	else if (BulletsLeft_C == 0 && CurrentBulletEquipped == 2 && BulletsLeft_C_Total > 0)
	{
		Reload();
	}
}

void ARoguelikeChar::Reload()
{
	if (bIsDead)return;

	// Play Reloading SFX.
	TArray<UAudioComponent*> Comps;
	GetComponents<UAudioComponent>(Comps);
	Comps[0]->Play();

	// Test
	//TakeDamageFromZombie(13);
	//PowerupName = "Double Damage!";

	if ((BulletsLeft_A_Total > 0 && CurrentBulletEquipped == 0 && BulletsLeft_A < MaxAmmoHolderSize)
		|| (BulletsLeft_B_Total > 0 && CurrentBulletEquipped == 1 && BulletsLeft_B < MaxAmmoHolderSize)
		|| (BulletsLeft_C_Total > 0 && CurrentBulletEquipped == 2 && BulletsLeft_C < MaxAmmoHolderSize))
	{
		PlayerAnimationInstance->bIsReloading = true;

		bIsAiming = false;
		bIsSprinting = false;
		PlayerAnimationInstance->bIsAiming = false;
		PlayerAnimationInstance->bIsSprinting = false;
		ToggleLaserVisibility(false);
		MovementSpeed = InitialMovementSpeed;
		PlayerController->UpdateUI();
	}
}

void ARoguelikeChar::Cycle()
{
	if (bIsDead)return;

	if (CurrentBulletEquipped < 2)
		CurrentBulletEquipped++;
	else
		CurrentBulletEquipped = 0;

	PlayerController->UpdateUI();
}

// ---------- Aiming-Related Input --------- //

void ARoguelikeChar::CharacterHoldAim()
{
	if (PlayerAnimationInstance->bIsReloading)return;

	bIsAiming = true;
	PlayerAnimationInstance->bIsAiming = bIsAiming;

	ToggleLaserVisibility(true);

	if (bIsSprinting)
	{
		bIsSprinting = false;
		PlayerAnimationInstance->bIsSprinting = bIsSprinting;
	}

	MovementSpeed = 0;
}

void ARoguelikeChar::CharacterReleaseAim()
{
	if (PlayerAnimationInstance->bIsReloading)return;

	bIsAiming = false;
	PlayerAnimationInstance->bIsAiming = bIsAiming;

	ToggleLaserVisibility(false);

	MovementSpeed = InitialMovementSpeed;
}


// ---------- Sprinting-Related Input --------- //

void ARoguelikeChar::CharacterHoldSprint()
{
	if (bIsOnCooldown || CurrentStamina < SprintStaminaRequiredMinimum)return;
	if (PlayerAnimationInstance->bIsReloading)return;
	if (bIsAiming)return;

	bIsSprinting = true;
	PlayerAnimationInstance->bIsSprinting = bIsSprinting;

	StaminaRate = StaminaRate * (-2);

	GetCharacterMovement()->MaxWalkSpeed = InitialMovementSpeed * SprintMultiplier;
	PlayerController->UpdateUI();
}

void ARoguelikeChar::CharacterReleaseSprint()
{
	if (!bIsSprinting)return;
	if (PlayerAnimationInstance->bIsReloading)return;
	if (bIsAiming)return;

	bIsSprinting = false;
	PlayerAnimationInstance->bIsSprinting = bIsSprinting;

	StaminaRate = StaminaRate * (-0.5f);

	GetCharacterMovement()->MaxWalkSpeed = InitialMovementSpeed;
	PlayerController->UpdateUI();
}

#pragma endregion Input-Related C++ Functions & Bindings

#pragma region Gameplay Mechanics


#pragma region Initialization

// ------- Initialization Method ----------- //

void ARoguelikeChar::InitializeGameplayStats()
{
	BulletsLeft_A = (User_BulletsLeft_A > 0) ? MaxAmmoHolderSize : 0;
	BulletsLeft_B = (User_BulletsLeft_B > 0) ? MaxAmmoHolderSize : 0;
	BulletsLeft_C = (User_BulletsLeft_C > 0) ? MaxAmmoHolderSize : 0;

	BulletsLeft_A_Total = User_BulletsLeft_A;
	BulletsLeft_B_Total = User_BulletsLeft_B;
	BulletsLeft_C_Total = User_BulletsLeft_C;

	MaxStamina = 100.0f;
	CurrentStamina = MaxStamina;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	Kills = 0;

	Wave = 1;
	PlayerAnimationInstance->bCanShoot = true;

	DeathCamera->Deactivate();
	ToggleLaserVisibility(false);

	InitialMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
	bIsAiming = bIsSprinting = false;

	PlayerController->UpdateUI();

}


void ARoguelikeChar::GameplayComponentReferencing()
{
	TArray<UCameraComponent*> Cams;
	GetComponents<UCameraComponent>(Cams);
	for (int32 i = 0; i < Cams.Num(); i++)
	{
		if (Cams[i]->ComponentHasTag("MainCamera"))
		{
			PlayerCamera = Cams[i];
			PlayerCamera->Activate();
		}
	}

	TArray<USpringArmComponent*> Springs;
	GetComponents<USpringArmComponent>(Springs);
	for (int32 i = 0; i < Springs.Num(); i++)
	{
		if (Springs[i]->ComponentHasTag("MainSpring"))
		{
			PlayerSpringArm = Springs[i];
		}
	}

	TArray<UParticleSystemComponent*> Particles;
	GetComponents<UParticleSystemComponent>(Particles);
	for (int32 i = 0; i < Particles.Num(); i++)
	{
		if (Particles[i]->ComponentHasTag("Lazor"))
		{
			PlayerLaser = Particles[i];
		}
	}

	TArray<UChildActorComponent*> Sources;
	GetComponents<UChildActorComponent>(Sources);
	for (int32 i = 0; i < Sources.Num(); i++)
	{
		if (Sources[i]->ComponentHasTag("Source"))
		{
			LaserSource = Sources[i];
		}
	}

	TArray<UPointLightComponent*> Targets;
	GetComponents<UPointLightComponent>(Targets);
	for (int32 i = 0; i < Targets.Num(); i++)
	{
		if (Targets[i]->ComponentHasTag("Target"))
		{
			LaserTarget = Targets[i];
		}
	}
}


#pragma endregion Gameplay Initialization


#pragma region Shooting


// ------- Anything Ammo-Related ------------ //

void ARoguelikeChar::RefillAmmo()
{
	int32 BulletsNeeded = 0;
	int32 AvailableBullets = 0;

	switch (CurrentBulletEquipped)
	{
	case 0:
		BulletsNeeded = MaxAmmoHolderSize - BulletsLeft_A;
		AvailableBullets = BulletsLeft_A_Total;

		if (BulletsNeeded < AvailableBullets)
		{
			BulletsLeft_A += BulletsNeeded;
			BulletsLeft_A_Total -= BulletsNeeded;
		}
		else
		{
			BulletsLeft_A += AvailableBullets;
			BulletsLeft_A_Total = 0;
		}

		break;
	case 1:
		BulletsNeeded = MaxAmmoHolderSize - BulletsLeft_B;
		AvailableBullets = BulletsLeft_B_Total;

		if (BulletsNeeded < AvailableBullets)
		{
			BulletsLeft_B += BulletsNeeded;
			BulletsLeft_B_Total -= BulletsNeeded;
		}
		else
		{
			BulletsLeft_B += AvailableBullets;
			BulletsLeft_B_Total = 0;
		}

		break;
	case 2:
		BulletsNeeded = MaxAmmoHolderSize - BulletsLeft_C;
		AvailableBullets = BulletsLeft_C_Total;

		if (BulletsNeeded < AvailableBullets)
		{
			BulletsLeft_C += BulletsNeeded;
			BulletsLeft_C_Total -= BulletsNeeded;
		}
		else
		{
			BulletsLeft_C += AvailableBullets;
			BulletsLeft_C_Total = 0;
		}

		break;
	default:
		break;
	}

	PlayerAnimationInstance->bIsReloading = false;
	PlayerAnimationInstance->bCanShoot = true;
	PlayerController->UpdateUI();
}

void ARoguelikeChar::ShootABullet()
{

	FHitResult Hit;
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = LaserSource->GetComponentLocation();
	FVector ForwardVector = PlayerCamera->GetForwardVector();
	FVector Destination = Origin + (ForwardVector * 5000);

	PlayerController->ClientPlayCameraShake(OurShake, ShakePower);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
	{

		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Hit.GetActor());
		if (Zombie)
		{
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UPlayerDamageType::StaticClass());

			//Determine the effect that will be applied to the zombie based on currently equipped ammo
			UPlayerDamageType* DamageType = Cast<UPlayerDamageType>(ValidDamageTypeClass.GetDefaultObject());
			switch (CurrentBulletEquipped)
			{
			case 1:
			{
				DamageType->PlayerDamageType = EPlayerDamageType::Dot;
				break;
			}
			case 2:
			{
				DamageType->PlayerDamageType = EPlayerDamageType::Slow;
				break;
			}
			default:
			{
				DamageType->PlayerDamageType = EPlayerDamageType::Standard;
				break;
			}

			}

			FDamageEvent DamageEvent(ValidDamageTypeClass);

			float DamageAmount = CalculateBulletDamage(CurrentBulletEquipped);
			Zombie->TakeDamage(DamageAmount, DamageEvent, Zombie->GetController(), this);
			/*if (Zombie->IsDead())
			{
				Kills++;
				PlayerController->UpdateUI();
			}*/

		}



		//if (Hit.Actor != GetOwner())
		//{
		//	if (Hit.Component->ComponentHasTag("Zombie"))
		//	{
		//		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		//		Hit.GetActor()->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
		//		UZombieAnimInstance* EnemyAnimInstance = Cast<UZombieAnimInstance>(SkeletalMeshes[0]->GetAnimInstance());

		//		if (!EnemyAnimInstance->bIsDead)
		//		{
		//			float DamageToDo = CalculateBulletDamage(CurrentBulletEquipped);

		//			// TO BE REMOVED AFTER ORFEUS COMPLETES ZOMBIE CRAP.
		//			TestHits++;

		//			if (TestHits > 3)
		//			{
		//				EnemyAnimInstance->bIsDead = true;
		//				Kills++;

		//				PlayerController->UpdateUI();
		//			}
		//			else
		//			{
		//				EnemyAnimInstance->GetHurt();
		//			}
		//		}
		//	}
		//}
	}


	switch (CurrentBulletEquipped)
	{
	case 0:
		if (BulletsLeft_A > 0)
		{
			BulletsLeft_A--;
		}
		break;
	case 1:
		if (BulletsLeft_B > 0)
		{
			BulletsLeft_B--;
		}
		break;
	case 2:
		if (BulletsLeft_C > 0)
		{
			BulletsLeft_C--;
		}
		break;

	default:
		break;
	}

	if (BulletsLeft_A == 0 && CurrentBulletEquipped == 0)
	{
		PlayerAnimationInstance->bCanShoot = false;
	}
	else if (BulletsLeft_B == 0 && CurrentBulletEquipped == 1)
	{
		PlayerAnimationInstance->bCanShoot = false;
	}
	else if (BulletsLeft_C == 0 && CurrentBulletEquipped == 2)
	{
		PlayerAnimationInstance->bCanShoot = false;
	}

	PlayerController->UpdateUI();
}

void ARoguelikeChar::IncreaseBulletsLeft(EAmmoType BulletsType, int32 Bullets)
{
	switch (BulletsType)
	{
	case EAmmoType::Incendiary:
	{
		BulletsLeft_B_Total += Bullets;
		break;
	}
	case EAmmoType::Frost:
	{
		BulletsLeft_C_Total += Bullets;
		break;
	}
	default:
		BulletsLeft_A_Total += Bullets;
		break;
	}
}

float ARoguelikeChar::CalculateBulletDamage(int32 BulletID)
{
	float Damage = 0;
	if (BulletID == 0)
	{
		Damage = FMath::RandRange(MinBulletDamageOrdinary, MaxBulletDamageOrdinary);
	}
	else if (BulletID == 1)
	{
		Damage = FMath::RandRange(MinBulletDamageIncinerary, MaxBulletDamageIncinerary);
	}
	else if (BulletID == 2)
	{
		Damage = FMath::RandRange(MinBulletDamageFrost, MaxBulletDamageFrost);
	}

	if (bIsDoubleDamageEnabled)
	{
		Damage *= 2;
	}

	return Damage;
}


#pragma endregion 


#pragma region Health and Stamina Mechanics

void ARoguelikeChar::IncreaseHealth(float HpToAdd)
{
	if (CurrentHealth + HpToAdd > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	else CurrentHealth += HpToAdd;
}

void ARoguelikeChar::TakeDamageFromZombie(float Damage)
{
	PlayerAnimationInstance->GetHurt();
	PlayerController->ClientPlayCameraShake(OurShake, ShakePower);
	CurrentHealth -= Damage;

	//Play Damaging SFX
	TArray<UAudioComponent*> Comps;
	GetComponents<UAudioComponent>(Comps);
	Comps[2]->Play();

	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
		DiePainfully();
	}
	PlayerController->UpdateUI();
}

void ARoguelikeChar::ChangeStamina(float Amount)
{
	CurrentStamina += Amount;

	if (bIsOnCooldown)
	{
		if (CurrentStamina > CooldownResetLim)
		{
			bIsOnCooldown = false;
		}
	}

	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;

		bIsSprinting = false;
		PlayerAnimationInstance->bIsSprinting = bIsSprinting;
		StaminaRate = StaminaRate * (-0.5f);
		GetCharacterMovement()->MaxWalkSpeed = InitialMovementSpeed;

		bIsOnCooldown = true;

	}
	else if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}

	PlayerController->UpdateUI();
}


#pragma endregion 


#pragma region Death Mechanics

void ARoguelikeChar::DiePainfully()
{
	bIsDead = true;
	PlayerAnimationInstance->bIsDead = true;

	bIsSprinting = false;
	bIsAiming = false;

	PlayerAnimationInstance->bIsSprinting = false;
	PlayerAnimationInstance->bIsAiming = false;
	PlayerAnimationInstance->MovementSpeedX = 0;
	PlayerAnimationInstance->MovementSpeedY = 0;
	PlayerAnimationInstance->bHasJumped = false;
	PlayerAnimationInstance->bIsReloading = false;

	PlayerCamera->Deactivate();
	DeathCamera->Activate();

	URoguelikeGameInstance* GameInstance = Cast<URoguelikeGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->StopGame();
	}


}

void ARoguelikeChar::DeathCameraMovement(float DeltaTime)
{
	DeathCamera->AddRelativeLocation(FVector(0, 0, DeathCamTranslateRate * DeltaTime));
	DeathCamera->AddRelativeRotation(FRotator(0, 0, DeathCamSpinRate * DeltaTime));
}

#pragma endregion 


#pragma region Laser Mechanics


void ARoguelikeChar::GenerateLaserBeam()
{
	FHitResult Hit;
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);

	FVector Origin = LaserSource->GetComponentLocation();
	FVector ForwardVector = PlayerCamera->GetForwardVector();
	FVector Destination = Origin + (ForwardVector * 5000);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Destination, ECollisionChannel::ECC_WorldDynamic, ColParams))
	{
		LaserTargetLocation = Hit.Location;
	}
	else
	{
		LaserTargetLocation = Destination;
	}

	PlayerLaser->SetBeamSourcePoint(0, Origin, 0);
	PlayerLaser->SetBeamTargetPoint(0, LaserTargetLocation, 0);
	LaserTarget->SetWorldLocation(LaserTargetLocation - LaserTarget->GetForwardVector());

}

void ARoguelikeChar::ToggleLaserVisibility(bool Toggle)
{
	if (Toggle)
	{
		PlayerLaser->Activate();
		LaserSource->Activate();
		LaserTarget->Activate();

		PlayerLaser->SetVisibility(true);
		LaserSource->SetVisibility(true);
		LaserTarget->SetVisibility(true);
	}
	else
	{
		PlayerLaser->SetVisibility(false);
		LaserSource->SetVisibility(false);
		LaserTarget->SetVisibility(false);

		PlayerLaser->Deactivate();
		LaserSource->Deactivate();
		LaserTarget->Deactivate();
	}


}


#pragma endregion


#pragma endregion 
