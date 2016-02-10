// Fill out your copyright notice in the Description page of Project Settings.

#include "CavemanTestField.h"
#include "MyCharacter_FirstTry.h"
#include "CavemanGameMode.h"
#include "DrawDebugHelpers.h"
#include "WolfCharacter.h"
#include "public/StaticFuncLib.h"
#include "EnemyStatsComponent.h"


#pragma region Core Functionality

ACavemanCharacter::ACavemanCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	NoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitterComp"));
	OurGreatClubComponent3rdPerson = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyGreatClub"));
	OurGreatBowComponent3rdPerson = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyGreatBow"));
	GNearClippingPlane = 0.1f;

}

void ACavemanCharacter::BeginPlay()
{
	Super::BeginPlay();

	#pragma region Referencing Components

	SFXManager = FindComponentByClass<class USoundEffectManager>();

	TArray<UCavemanRecipeListComponent*> RecipeComp;
	GetComponents<UCavemanRecipeListComponent>(RecipeComp);
	for (int32 i = 0; i < RecipeComp.Num(); i++)
	{
		RecipeList = RecipeComp[0];
	}

	for (int32 i = 0; i < Data->GetRowNames().Num(); i++)
	{
		FCraftingItemsStruct* Row = Data->FindRow<FCraftingItemsStruct>(FName(*FString::FromInt(i)), TEXT("ItemName"));
		if (Row)
		{
			RecipeList->Items_Recipes.Add(Row->ItemName);
			RecipeList->Recipe_Knowledge.Add(true);
		}
	}

	for (int32 i = 0; i < CookData->GetRowNames().Num(); i++)
	{

		FCraftingItemsStruct* Row = CookData->FindRow<FCraftingItemsStruct>(FName(*FString::FromInt(i)), TEXT("ItemName"));
		if (Row)
		{
			RecipeList->Cook_Items_Recipes.Add(Row->ItemName);
			RecipeList->Cook_Recipe_Knowledge.Add(true);
		}
	}
	

	TArray<UAudioComponent*> Audios;
	GetComponents<UAudioComponent>(Audios);
	for (int32 i = 0; i < Audios.Num(); i++)
	{
		if (Audios[i]->ComponentHasTag("FIREBRAND"))
		{
			Firebrand = Audios[i];
			Firebrand->Stop();
		}
	}

	TArray<UCameraComponent*> CamComps;
	GetComponents<UCameraComponent>(CamComps);
	for (int32 i = 0; i < CamComps.Num(); i++)
	{
		if (CamComps[i]->ComponentHasTag("FirstPersonCamera"))
		{
			FirstPersonCamera = CamComps[i];
		}
		if (CamComps[i]->ComponentHasTag("ThirdPersonCamera"))
		{
			ThirdPersonCamera = CamComps[i];
		}

	}


	TArray<USpringArmComponent*> Springs;
	GetComponents<USpringArmComponent>(Springs);
	for (int32 i = 0; i < Springs.Num(); i++)
	{
		if (Springs[i]->ComponentHasTag("OurSpringArm"))
		{
			SpringArmComponent = Springs[i];
		}
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (int32 i = 0; i < SkeletalMeshes.Num(); i++)
	{
		if (SkeletalMeshes[i]->ComponentHasTag("TheCavemanModel"))
		{
			TheCavemanSkeletalMesh = SkeletalMeshes[i];
			LastKnownRotation = TheCavemanSkeletalMesh->GetComponentRotation();
			MyAnimInstance = Cast<UCavemanAnimInstance>(TheCavemanSkeletalMesh->GetAnimInstance());

		}
		if (SkeletalMeshes[i]->ComponentHasTag("CavemanHand"))
		{
			HandSkeletalMesh = SkeletalMeshes[i];
		}
		if (SkeletalMeshes[i]->ComponentHasTag("CavemanHand_Range"))
		{
			HandSkeletalMeshBow = SkeletalMeshes[i];
		}

		TArray<UStaticMeshComponent*> Meshes;
		GetComponents<UStaticMeshComponent>(Meshes);
		for (int32 i = 0; i < Meshes.Num(); i++)
		{
			if (Meshes[i]->ComponentHasTag("FirstPersonClub"))
			{
				OurGreatClubComponent1stPerson = Meshes[i];
				OurGreatClubComponent1stPerson->AttachTo(HandSkeletalMesh, "HUHU", EAttachLocation::SnapToTarget, false);
				OurGreatClubComponent1stPerson->SetWorldScale3D(MyClubScale1st);
				OurGreatClubComponent1stPerson->SetWorldRotation(FRotator(MyClubRotator1st.Pitch, LastKnownRotation.Yaw - 180, MyClubRotator1st.Roll));
				OurGreatClubComponent1stPerson->SetRelativeLocation(MyClubLocation1st);
			}

			if (Meshes[i]->ComponentHasTag("FirstPersonBow"))
			{
				OurGreatBowComponent1stPerson = Meshes[i];
				OurGreatBowComponent1stPerson->AttachTo(HandSkeletalMeshBow, "HUHU", EAttachLocation::SnapToTarget, true);
				OurGreatBowComponent1stPerson->SetWorldScale3D(MyBowScale1st);
				OurGreatBowComponent1stPerson->SetWorldRotation(FRotator(MyBowRotator1st.Pitch, LastKnownRotation.Yaw - 180, MyBowRotator1st.Roll));
				OurGreatBowComponent1stPerson->SetRelativeLocation(MyBowLocation1st);
			}
			if (Meshes[i]->ComponentHasTag("FingerArrow"))
			{
				TheArrowThatWeHoldBeforeWeFire = Meshes[i];
				TheArrowThatWeHoldBeforeWeFire->AttachTo(TheCavemanSkeletalMesh, "ArrowSocket", EAttachLocation::SnapToTarget, true);
				TheArrowThatWeHoldBeforeWeFire->SetOwnerNoSee(true);
			}
		}
	}

	#pragma endregion 

	Controller = Cast<ACavemanController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	#pragma region Gameplay Initialization

	// Setup main traits and update the UI

	CurCDTimerQuickSlot = 0;
	Health_Current = Health_Max;
	Heat_Current = Heat_Max;
	Stamina_Current = Stamina_Max;
	Hunger_Current = Hunger_Max;

	UpdateUIQuickSlots(ItemInSlot1, ItemInSlot2, ItemInSlot3, ItemInSlot4);
	Controller->InventoryWidget->OnAwesomeness.Broadcast();
	Controller->InventoryWidget->OnInteractionDokFinished.Broadcast();
	Controller->UberWidget->Health = FString::SanitizeFloat((Health_Current / Health_Max) * 100) + FString("%");
	Controller->UberWidget->Heat = FString::SanitizeFloat((Heat_Current / Heat_Max) * 100) + FString("%");
	Controller->UberWidget->Hunger = FString::SanitizeFloat((Hunger_Current / Hunger_Max) * 100) + FString("%");
	Controller->UberWidget->Stamina = FString::SanitizeFloat((Stamina_Current / Stamina_Max) * 100) + FString("%");
	Controller->UberWidget->HealthBar = Health_Current / Health_Max;
	Controller->UberWidget->HeatBar = Heat_Current / Heat_Max;
	Controller->UberWidget->HungerBar = Hunger_Current / Hunger_Max;
	Controller->UberWidget->StaminaBar = Stamina_Current / Stamina_Max;
	Controller->UberWidget->Temperature = FString::SanitizeFloat(Temperature);

	NormalWalkSpeed = GetCharacterMovement()->GetMaxSpeed();

	// -- Collision Setup -- //

	Cast<UPrimitiveComponent>(GetRootComponent())->OnComponentBeginOverlap.AddDynamic(this, &ACavemanCharacter::OnCavemanBeginOverlap);
	Cast<UPrimitiveComponent>(GetRootComponent())->OnComponentEndOverlap.AddDynamic(this, &ACavemanCharacter::OnCavemanEndOverlap);

	#pragma endregion

#pragma region Weapon Initialization (For Third Person Mode)
	
	OurGreatClubComponent3rdPerson->StaticMesh = OurGreatClub;
	OurGreatClubComponent3rdPerson->AttachTo(TheCavemanSkeletalMesh, "RightHandSocket", EAttachLocation::SnapToTarget, false);
	OurGreatClubComponent3rdPerson->SetWorldScale3D(MyClubScale);
	OurGreatClubComponent3rdPerson->SetWorldRotation(FRotator(MyClubRotator.Pitch, LastKnownRotation.Yaw - MyClubRotator.Yaw, MyClubRotator.Roll));
	OurGreatClubComponent3rdPerson->SetRelativeLocation(FVector(MyClubLocation.X, MyClubLocation.Y, MyClubLocation.Z));

	OurGreatBowComponent3rdPerson->StaticMesh = OurGreatBow;
	OurGreatBowComponent3rdPerson->AttachTo(TheCavemanSkeletalMesh, "RightHandSocket", EAttachLocation::SnapToTarget, false);
	OurGreatBowComponent3rdPerson->SetWorldScale3D(MyBowScale3rd);
	OurGreatBowComponent3rdPerson->SetWorldRotation(FRotator(MyBowRotator3rd.Pitch, LastKnownRotation.Yaw - MyBowRotator3rd.Yaw, MyBowRotator3rd.Roll));
	OurGreatBowComponent3rdPerson->SetRelativeLocation(FVector(MyBowLocation3rd.X, MyBowLocation3rd.Y, MyBowLocation3rd.Z));
	OurGreatBowComponent3rdPerson->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	HandSkeletalMesh->SetOwnerNoSee(true);
	HandSkeletalMeshBow->SetOwnerNoSee(true);

	#pragma endregion

	#pragma region Weapon Initialization

	// Since the game is Single Player, we can just change visibilities via "SetOwnerNoSee"

	if (!bIs1stPerson)
	{
		FirstPersonCamera->Deactivate();

		if (bIsMelee)
		{
			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(false);
		}
		else
		{
			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(false);
		}
	}
	else
	{
		FirstPersonCamera->Activate();

		if (bIsMelee)
		{
			HandSkeletalMesh->SetOwnerNoSee(false);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(false);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
		}
		else
		{
			HandSkeletalMeshBow->SetOwnerNoSee(false);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(false);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
		}
	}

	#pragma endregion

}

void ACavemanCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ******************************************************** //
	// **** Combat Mode Handling: Are we in combat or not? **** //
	// ******************************************************** //
	
	if (bIsInCombat && (GetWorld()->TimeSeconds - CombatStartSeconds > CombatResetSeconds))
	{
		SetCombatStatus(false);
	}


	// ********************************** //
	// **** Meter Counter on Falling **** //
	// ********************************** //

	if (bShouldCountFallingMeters || MyAnimInstance->bStartCountingMeters)
	{
		Meters += 1;
		MyAnimInstance->MetersFallen = Meters;
	}

	// ********************************************* //
	// **** Shooting Logic on releasing the bow **** //
	// ********************************************* //

	if (bCanNowAttackRanged && bBowReleased)
	{
		bCanNowAttackRanged = false;
		bBowReleased = false;

		HandSkeletalMeshBow->AnimationData.AnimToPlay = FirstPersonRelease_Bow;
		HandSkeletalMeshBow->Stop();
		HandSkeletalMeshBow->PlayAnimation(FirstPersonRelease_Bow, false);

		ShootTheArrow();
	}

	// ****************************************** //
	// **** Stamina Dropping Mechanic System **** //
	// ****************************************** //

	if (isSprinting && (X_AxisValue != 0 || Y_AxisValue != 0))
	{
		Stamina_Down(Stamina_DropRate);
	}
	else
	{
		Stamina_Up(Stamina_IncreaseRate);
	}

	// ************************************************************ //
	// **** Rotate the Character Mesh Based on Input Direction **** //
	// ************************************************************ //

	if (X_AxisValue != 0 || Y_AxisValue != 0)
	{
		if (bHasLockedTarget) // Extra logic if we are locked on target.
		{
			FRotator Rotation = (EnemyTarget->GetActorLocation() - GetActorLocation()).Rotation();
			Rotation.Pitch = 0;
			LastKnownRotation = Rotation - FRotator(0, 90, 0);
		}
		else
		{
			ModelRotatorHelper = (GetActorForwardVector() * X_AxisValue) + (GetActorRightVector() * Y_AxisValue);
			LastKnownRotation = ModelRotatorHelper.Rotation() - FRotator(0, 90, 0);
		}
	}
	else if (MyAnimInstance->b3rdPersonHolding)
	{
		LastKnownRotation = FRotator(0.f, (ThirdPersonCamera->GetForwardVector().Rotation() - FRotator(0, 84, 0)).Yaw, 0.f);
	}

	TheCavemanSkeletalMesh->SetWorldRotation(LastKnownRotation);
	TheCavemanSkeletalMesh->SetWorldLocation(this->GetActorLocation() - FVector(0, 0, 90));
	
	

	// ****************************************************************** //
	// **** Spring arm comp rotation set (Based on Lock On Mechanic) **** //
	// ****************************************************************** //

	if (bHasLockedTarget)
	{
		SpringArmComponent->bUsePawnControlRotation = false;
		FRotator TempRot = LastKnownRotation;
		TempRot.Pitch -= 30;
		TempRot.Yaw += 90;
		SpringArmComponent->SetWorldRotation(TempRot);
	}

	// **************************************** //
	// **** Trait System Over Time Effects **** //
	// **************************************** //

	if (!bIsResting)
	{
		HeatOverTime(DeltaTime);
		HungerOverTime(DeltaTime);

		if (bStarvationHasStarted)
		{
			StarvationOverTime(DeltaTime);
		}
	}

	// ************************************** //
	// **** Gameplay Functions for Tick  **** //
	// ************************************** //

	AttackRaycasting();
	CavemanAnimationHandling();
	InteractionHandling();
}

#pragma endregion

#pragma region Interaction | Attacking | Animation Handling

// The Raycasting system for Interaction
void ACavemanCharacter::InteractionHandling()
{
	FHitResult Hit;
	FVector StartLocation = (bIs1stPerson) ? FirstPersonCamera->GetComponentLocation() : ThirdPersonCamera->GetComponentLocation();
	FVector EndLocation = (bIs1stPerson) ? (FirstPersonCamera->GetForwardVector() * InteractionDistance) + StartLocation : (ThirdPersonCamera->GetForwardVector() * InteractionDistance*1.5f) + StartLocation;
	FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
	ColParams.AddIgnoredActor(GetOwner());
	ColParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_WorldDynamic, ColParams))
	{
		if (Hit.GetActor() != this && (Hit.GetActor()->GetRootComponent()->ComponentHasTag("Enemy")))
		{
			UEnemyStatsComponent* MyAWESOMEComp = nullptr;
			TArray<UEnemyStatsComponent*> Comp;
			Hit.GetActor()->GetComponents<UEnemyStatsComponent>(Comp);

			bool bCanLoot = false;
			for (int32 i = 0; i < Comp.Num(); i++)
			{
				MyAWESOMEComp = Comp[0];

				for (int32 j = 0; j < MyAWESOMEComp->LootTable.Num(); j++)
				{
					if (MyAWESOMEComp->LootTable[j] != "")
					{
						bCanLoot = true;
					}
				}
			}

			if (MyAWESOMEComp->bIsDead && bCanLoot)
			{
				InteractionTarget = Hit.GetActor();
				InteractionTargetName = Hit.GetActor()->Tags[0].ToString();

				LastAccessedCharacter = Cast<ACharacter>(MyAWESOMEComp->GetOwner());
				if (LastAccessedCharacter)
				{
					LastAccessedCharacter->GetMesh()->SetRenderCustomDepth(true);
				}


				bWillLoot = true;

			}
			else
			{
				if (InteractionTarget != nullptr)
				{
					if (LastAccessedCharacter)
					{
						LastAccessedCharacter->GetMesh()->SetRenderCustomDepth(false);
					}

				}
				InteractionTarget = nullptr;
				bWillLoot = false;
			}
			InteractionMode = "";
		}
		else if (Hit.GetActor() != this && Hit.GetActor()->GetRootComponent()->ComponentHasTag("Interactable"))
		{

			bWillLoot = false;
			if (Hit.GetActor()->GetRootComponent()->ComponentHasTag("Campfire"))
			{
				InteractionMode = "Campfire";
				InteractionTarget = Hit.GetActor();
				InteractionTargetName = "Inactive Campfire";
				
				
				Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(true);
				if (Hit.GetActor()->GetRootComponent()->ComponentHasTag("CampfireActive"))
				{
					InteractionMode = "CampfireActive";
					InteractionTarget = Hit.GetActor();
					InteractionTargetName = "Campfire";
					Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(true);
				}
				return;
			}
			else if (Hit.GetActor()->GetRootComponent()->ComponentHasTag("Harvestable"))
			{
				InteractionMode = "Harvestable";
				InteractionTarget = Hit.GetActor();
				UEnemyStatsComponent* MyAWESOMEComp = nullptr;
				TArray<UEnemyStatsComponent*> Comp;
				Hit.GetActor()->GetComponents<UEnemyStatsComponent>(Comp);

				bool bCanLoot = false;
				for (int32 i = 0; i < Comp.Num(); i++)
				{
					MyAWESOMEComp = Comp[0];

					for (int32 j = 0; j < MyAWESOMEComp->LootTable.Num(); j++)
					{
						if (MyAWESOMEComp->LootTable[j] != "")
						{
							bCanLoot = true;
						}
					}
				}
				bWillLoot = bCanLoot;
				if (bWillLoot)
				{
					InteractionTarget = Hit.GetActor();
					InteractionTargetName = Hit.GetActor()->Tags[0].ToString();
					Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(true);
				}
				else
				{
					if (InteractionTarget != nullptr)
					{
						if (LastAccessedCharacter)
						{
							LastAccessedCharacter->GetMesh()->SetRenderCustomDepth(false);
							LastAccessedCharacter = nullptr;
						}
						else Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(false);
					}

					InteractionTarget = nullptr;
					InteractionTargetName = "";
					bWillLoot = false;
					InteractionMode = "";

				}
			}
			else
			{
				InteractionMode = "";
			}
		}
		else
		{
			if (InteractionTarget != nullptr)
			{
				if (LastAccessedCharacter)
				{
					LastAccessedCharacter->GetMesh()->SetRenderCustomDepth(false);
					LastAccessedCharacter = nullptr;
				}
				else Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(false);

				

			}
			InteractionTarget = nullptr;
			InteractionTargetName = "";
			bWillLoot = false;
			InteractionMode = "";
		}
	}
	else
	{
		

		if (InteractionTarget != nullptr)
		{
			if (LastAccessedCharacter)
			{
				LastAccessedCharacter->GetMesh()->SetRenderCustomDepth(false);
				LastAccessedCharacter = nullptr;
			}
			else Cast<UPrimitiveComponent>(InteractionTarget->GetRootComponent())->SetRenderCustomDepth(false);
			
		}

		InteractionTarget = nullptr;
		InteractionTargetName = "";
		bWillLoot = false;
		InteractionMode = "";
	}

}

// Depending on what we look, gameplay changes when we press "E".
void ACavemanCharacter::Interact()
{
	if (MyAnimInstance)
	{
		if (MyAnimInstance->Montage_IsPlaying(FallAnimHard) || bHasDied)return;
		if (Controller->bIsInventoryOpen) return;
		if (InteractionTarget == nullptr)return;
		if (bIsFireStartingWindowEnabled)return;
		if (bIsFireMainMenuEnabled)return;
		if (bIsCraftingWindowOpen)return;
		if (bIsCraftingAtTheMoment)return;
		if (bIsStartingFireAtTheMoment)return;
		if (bIsResting)return;

	}
	else
	{
		return;
	}

	if (InteractionMode == "Campfire")
	{
		Controller->OpenCrafts(false, false);
		bIsFireStartingWindowEnabled = true;
		SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_WindowOpen);
		return;
	}
	if (InteractionMode == "CampfireActive")
	{
		Controller->OpenCrafts(false, true);
		bIsFireMainMenuEnabled = true;
		SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_WindowOpen);
		return;
	}

	if (bLootWindowOpen)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->LootWidget->LootAllDelegate.Broadcast();
			SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_Loot);
		}

		return;
	}


	if (bWillLoot)
	{
		Controller->LastDiedEnemyThatWeLooted = InteractionTarget;
		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->OpenLootWindow();
		}
		Controller->InventoryWidget->OnAwesomeness.Broadcast();
		
		if (InteractionTarget->ActorHasTag("Conveniently Placed Tech Demo Bush"))
		{
			SFXManager->PlaySoundEffect(ESoundEffects::SFX_BushInteracted);
		}
		else
		{
			SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_WindowOpen);
		}
	}

}

// Spawn an Arrow, shoot it, add Velocity to it and hope for the best!
void ACavemanCharacter::ShootTheArrow()
{
	ArrowsLeft--;
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = Instigator;
	
	if (bIs1stPerson)
	{
		MyAnimInstance->bCanMoveWithInput = true;
		ACavemanArrow* OurNewObject = GetWorld()->SpawnActor<ACavemanArrow>(OurGreatArrow, FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector().GetSafeNormal() * 0.5f, FirstPersonCamera->GetForwardVector().Rotation(), spawnParams);
		OurNewObject->MyComp->SetPhysicsLinearVelocity(FirstPersonCamera->GetForwardVector().GetSafeNormal()*ArrowForce*1.3f, false);
	}
	else
	{
		ACavemanArrow* OurNewObject = GetWorld()->SpawnActor<ACavemanArrow>(OurGreatArrow, FirstPersonCamera->GetComponentLocation() + TheCavemanSkeletalMesh->GetForwardVector().GetSafeNormal() * 20.f, (FRotator(ThirdPersonCamera->GetAttachParent()->GetComponentRotation().Pitch, (LastKnownRotation + FRotator(0, 90, 0)).Yaw, 0)), spawnParams);
		OurNewObject->MyComp->SetPhysicsLinearVelocity(FRotator(ThirdPersonCamera->GetAttachParent()->GetComponentRotation().Pitch, (LastKnownRotation + FRotator(0, 90, 0)).Yaw, 0).Vector().GetSafeNormal()*ArrowForce, false);
	}
}

// Our caveman gets hurt from enemies
void ACavemanCharacter::PlayHitAnimation()
{
	MyAnimInstance->GetHurt();

	// Test, Debugging (This is a very simple input scheme, for easy testing)
	//Controller->AddItemToInventory("Flint");
	//Controller->AddItemToInventory("Tinder");
	//Controller->AddItemToInventory("leaf");

}

// Emit Raycasts from the Base Sockets of our weapons to calculate Melee Hits.
void ACavemanCharacter::AttackRaycasting()
{
	if (bIsMelee)
	{
		FVector curBase;
		FVector curTip;
		if (!bIs1stPerson)
		{
			curBase = TheCavemanSkeletalMesh->GetSocketLocation(FName("RightHandSocket"));
			curTip = TheCavemanSkeletalMesh->GetSocketLocation(FName("RightHandSocketDur"));
		}
		else
		{

			curBase = HandSkeletalMesh->GetSocketLocation(FName("HUHU"));
			curTip = HandSkeletalMesh->GetSocketLocation(FName("HUHU_Tip"));
			
		}
		const int sub = 32;
		float curLength = (curBase - curTip).Size() * 2;
		float prevLength = (prevBase - prevTip).Size();
		for (int i = 1; i < sub; i++)
		{
			FVector tmpBase = FMath::Lerp(curBase, prevBase, i / float(sub));
			FVector tmpTip = FMath::Lerp(curTip, prevTip, i / float(sub));
			FVector tmpOff = (tmpTip - tmpBase);
			tmpOff.Normalize();
		}
		prevBase = curBase;
		prevTip = curTip;

		FHitResult Hit;
		FCollisionQueryParams ColParams = FCollisionQueryParams(FName("Tsirla"), false, GetOwner());
		ColParams.AddIgnoredActor(GetOwner());
		ColParams.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, curBase, curTip, ECollisionChannel::ECC_WorldDynamic, ColParams))
		{
			if (Hit.Actor != GetOwner() && bCanInflictDamageOnFoes)
			{
				bCanInflictDamageOnFoes = false;
				DamageEnemy(Hit.GetActor());
				SFXManager->PlaySoundEffect(ESoundEffects::SFX_ClubHit);
			}
		}
	}
}

void ACavemanCharacter::CavemanAnimationHandling()
{
	if (bHasDied)return;

	if (Controller->bIsInventoryOpen)
	{
		MyAnimInstance->X_Axis = 0;
		MyAnimInstance->Y_Axis = 0;
	}

	MyAnimInstance->bIsGrounded = GetCharacterMovement()->IsMovingOnGround();

	if (MyAnimInstance->bIsGrounded)
	{
		if (MyAnimInstance->bHasJumped)MyAnimInstance->bHasJumped = false;

		if (bShouldCountFallingMeters || MyAnimInstance->bStartCountingMeters)
		{
			bShouldCountFallingMeters = false;
			MyAnimInstance->bStartCountingMeters = false;

			if (Meters > 65)
			{
				MyAnimInstance->bWillHardLand = true;
				Health_Down(Meters - 65);
			}
			Meters = 0;
		}
	}
}

#pragma endregion

void ACavemanCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACavemanCharacter::MyCustomAction);
	InputComponent->BindAction("TogglePOV", IE_Pressed, this, &ACavemanCharacter::TogglePOVFunc);

	InputComponent->BindAction("InventoryUp", IE_Pressed, this, &ACavemanCharacter::InventoryUp);
	InputComponent->BindAction("InventoryDown", IE_Pressed, this, &ACavemanCharacter::InventoryDown);
	InputComponent->BindAction("InventoryRight", IE_Pressed, this, &ACavemanCharacter::InventoryRight);
	InputComponent->BindAction("InventoryLeft", IE_Pressed, this, &ACavemanCharacter::InventoryLeft);
	InputComponent->BindAction("InventoryUse", IE_Pressed, this, &ACavemanCharacter::InventoryUse);
	InputComponent->BindAction("InventoryDrop", IE_Pressed, this, &ACavemanCharacter::InventoryDrop);
	InputComponent->BindAction("InventoryAssign", IE_Pressed, this, &ACavemanCharacter::InventoryAssign);

	InputComponent->BindAction("Cancel(PC)", IE_Pressed, this, &ACavemanCharacter::CancelFromPC);

	InputComponent->BindAction("Interact", IE_Pressed, this, &ACavemanCharacter::Interact);
	InputComponent->BindAction("Crafting", IE_Pressed, this, &ACavemanCharacter::CraftingWindowToggle);

	InputComponent->BindAction("QuickSlot1", IE_Pressed, this, &ACavemanCharacter::UseQuickSlot1);
	InputComponent->BindAction("QuickSlot2", IE_Pressed, this, &ACavemanCharacter::UseQuickSlot2);
	InputComponent->BindAction("QuickSlot3", IE_Pressed, this, &ACavemanCharacter::UseQuickSlot3);
	InputComponent->BindAction("QuickSlot4", IE_Pressed, this, &ACavemanCharacter::UseQuickSlot4);

	InputComponent->BindAction("Attack", IE_Pressed, this, &ACavemanCharacter::Attack);
	InputComponent->BindAction("Attack", IE_Released, this, &ACavemanCharacter::AttackRelease);

	InputComponent->BindAction("WeaponSwap", IE_Pressed, this, &ACavemanCharacter::ToggleWeaponMode);

	InputComponent->BindAction("Sprinting", IE_Repeat, this, &ACavemanCharacter::SprintingON);
	InputComponent->BindAction("Sprinting", IE_Released, this, &ACavemanCharacter::SprintingOFF);

	InputComponent->BindAction("Test", IE_Pressed, this, &ACavemanCharacter::PlayHitAnimation);

	InputComponent->BindAction("UI Toggle", IE_Pressed, this, &ACavemanCharacter::ToggleInventoryUI);

	InputComponent->BindAction("PlaceTrap", IE_Pressed, this, &ACavemanCharacter::PlaceTrap);

	InputComponent->BindAxis("MoveForward", this, &ACavemanCharacter::MoveXAxis);
	InputComponent->BindAxis("MoveRight", this, &ACavemanCharacter::MoveYAxis);
	InputComponent->BindAxis("Turn", this, &ACavemanCharacter::MouseTurn);
	InputComponent->BindAxis("TurnUp", this, &ACavemanCharacter::MouseTurnUp);
}

#pragma region Input Mechanics Setup

void ACavemanCharacter::CancelFromPC()
{
	if (bIsCraftingAtTheMoment)
	{
		Controller->CraftWidget->InterruptCrafting.Broadcast();
	}

	if (bIsStartingFireAtTheMoment)
	{
		Controller->CraftWidget->InterruptStartingFire.Broadcast();
	}

	if (bIsCraftingWindowOpen)
	{
		Controller->CloseCrafts();
	}

	if (bLootWindowOpen)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->CloseLootWindow();
		}
		bLootWindowOpen = false;
		return;
	}

	if (Controller->bIsInventoryOpen)
	{
		Controller->CloseInventory();
	}

	if (Controller->bIsAssignmentMenuOpen)
	{
		Controller->InventoryWidget->OnAssignmentViaGamepadCancel.Broadcast();
	}
}

void ACavemanCharacter::CraftingWindowToggle()
{
	if (bHasDied)return;

	if (HandSkeletalMesh->AnimationData.AnimToPlay == FirstPersonAttack)return;

	if (MyAnimInstance->bIsAttacking)return;

	if (HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonPull_Bow || HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonRelease_Bow || bBowReleased) return;

	if (MyAnimInstance->b3rdPersonHolding || MyAnimInstance->b3rdPersonPulling || MyAnimInstance->b3rdPersonReleasing)return;

	if (bLootWindowOpen) return;

	if (Controller->bIsAssignmentMenuOpen) return;

	if (Controller->bIsInventoryOpen) return;

	if (bIsResting)return;

	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;

	if (!bIsFireMainMenuEnabled)
		(bIsCraftingWindowOpen) ? Controller->CloseCrafts() : Controller->OpenCrafts(true, false);

	SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_WindowOpen);
}

void ACavemanCharacter::UseQuickSlot1()
{
	if (bHasDied)return;
	if (Controller->bIsInventoryOpen) return;
	if (Controller->UberWidget->bIsInQuickShotCD) return;
	if (bLootWindowOpen)return;
	if (bIsCraftingWindowOpen)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsResting)return;

	UseItemThroughSlot(ItemInSlot1);
}

void ACavemanCharacter::UseQuickSlot2()
{
	if (bHasDied)return;
	if (Controller->bIsInventoryOpen) return;
	if (Controller->UberWidget->bIsInQuickShotCD) return;
	if (bLootWindowOpen)return;
	if (bIsCraftingWindowOpen)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsResting)return;

	UseItemThroughSlot(ItemInSlot2);
}

void ACavemanCharacter::UseQuickSlot3()
{
	if (bHasDied)return;
	if (Controller->bIsInventoryOpen) return;
	if (Controller->UberWidget->bIsInQuickShotCD) return;
	if (bLootWindowOpen)return;
	if (bIsCraftingWindowOpen)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsResting)return;

	UseItemThroughSlot(ItemInSlot3);
}

void ACavemanCharacter::UseQuickSlot4()
{
	if (bHasDied)return;
	if (Controller->bIsInventoryOpen) return;
	if (Controller->UberWidget->bIsInQuickShotCD) return;
	if (bLootWindowOpen)return;
	if (bIsCraftingWindowOpen)return;

	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsResting)return;


	UseItemThroughSlot(ItemInSlot4);
}

void ACavemanCharacter::UseItem(FString ItemName)
{
	bool WillUseItem = false;
	if (ItemName == "Wolf Meat")
	{
		Hunger_Up(20);
		Health_Up(5);
		WillUseItem = true;
		SFXManager->PlaySoundEffect(ESoundEffects::SFX_CharacterNOMNOMNOM);
	}
	if (ItemName == "Wolf Steak")
	{
		Hunger_Up(80);
		Health_Up(30);
		WillUseItem = true;
		SFXManager->PlaySoundEffect(ESoundEffects::SFX_CharacterNOMNOMNOM);
	}

	if (WillUseItem)
	{
		if (Controller->IsTheItemSlotted(ItemName))
		{
			if (Controller->GetNumberOfItemsInInventory(ItemName) > 1)
			{
				Controller->RemoveItemFromInventory(Controller->InventoryWidget->CurrentlySelectedSlot);
			}
			else if (Controller->GetNumberOfItemsInInventory(ItemName) == 1)
			{
				int32 Slot = Controller->GetTheSlotThatTheItemIsAssignedTo(ItemName);
				AssignNameToSlot("", Slot);
				Controller->RemoveItemFromInventory(Controller->InventoryWidget->CurrentlySelectedSlot);
			}
		}
		else
		{
			Controller->RemoveItemFromInventory(Controller->InventoryWidget->CurrentlySelectedSlot);
		}
	}


}

void ACavemanCharacter::UseItemThroughSlot(FString ItemName)
{
	if (ItemName == "brick")
	{
		GLog->Log("Used a Big Brick!");
	}

	if (ItemName == "brick2")
	{
		GLog->Log("Used a Bigger Brick!");
	}

	int32 FirstOccurence = Controller->FindFirstOccurenceOfItemInInventory(ItemName);
	Controller->RemoveItemFromInventory(FirstOccurence);
}

void ACavemanCharacter::AssignNameToSlot(FString Name, int32 SlotID)
{

	RemoveAnyPossibleSlotConflicts(Name, SlotID);

	if (SlotID == 1)
	{
		ItemInSlot1 = Name;
	}
	else if (SlotID == 2)
	{
		ItemInSlot2 = Name;
	}
	else if (SlotID == 3)
	{
		ItemInSlot3 = Name;
	}
	else if (SlotID == 4)
	{
		ItemInSlot4 = Name;
	}

	UpdateUIQuickSlots(ItemInSlot1, ItemInSlot2, ItemInSlot3, ItemInSlot4);


}

void ACavemanCharacter::UpdateUIQuickSlots(FString Slot1, FString Slot2, FString Slot3, FString Slot4)
{
	FSlateBrush BrushForImage1;
	FSlateBrush BrushForImage2;
	FSlateBrush BrushForImage3;
	FSlateBrush BrushForImage4;


	for (int32 i = 0; i < Controller->ItemNames.Num(); i++)
	{
		if (Controller->ItemNames[i] == Slot1)
		{
			BrushForImage1 = Controller->ItemSprites[i];
		}
		if (Controller->ItemNames[i] == Slot2)
		{
			BrushForImage2 = Controller->ItemSprites[i];
		}
		if (Controller->ItemNames[i] == Slot3)
		{
			BrushForImage3 = Controller->ItemSprites[i];
		}
		if (Controller->ItemNames[i] == Slot4)
		{
			BrushForImage4 = Controller->ItemSprites[i];
		}
	}

	BrushForImage1.ImageSize = FVector2D(80.f, 80.f);
	BrushForImage2.ImageSize = FVector2D(80.f, 80.f);
	BrushForImage3.ImageSize = FVector2D(80.f, 80.f);
	BrushForImage4.ImageSize = FVector2D(80.f, 80.f);


	Controller->UberWidget->QuickSlot1Brush = BrushForImage1;
	Controller->UberWidget->QuickSlot2Brush = BrushForImage2;
	Controller->UberWidget->QuickSlot3Brush = BrushForImage3;
	Controller->UberWidget->QuickSlot4Brush = BrushForImage4;

}

void ACavemanCharacter::RemoveAnyPossibleSlotConflicts(FString Name, int32 SlotID)
{
	if (Name == "")return;
	if (Controller->IsTheItemSlotted(Name))
	{
		int32 SlotToClear = Controller->GetTheSlotThatTheItemIsAssignedTo(Name);
		AssignNameToSlot("", SlotToClear);
	}
}

void ACavemanCharacter::InventoryUse()
{
	if (bHasDied)return;
	if (!Controller->bIsInventoryOpen && !bLootWindowOpen && !bIsCraftingWindowOpen) return;
	if (Controller->bIsAssignmentMenuOpen) return;
	if (bLootWindowOpen)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;

	if (bIsCraftingAtTheMoment) return;
	if (bIsStartingFireAtTheMoment)return;

	if (bIsRestingWindowOpen)
	{
		Controller->CraftWidget->GamepadRestConfirm.Broadcast();
		return;
	}
	if (bIsFireMainMenuEnabled)
	{
		Controller->CraftWidget->GamepadCook.Broadcast();
		return;
	}
	else if (bIsFireStartingWindowEnabled)
	{
		Controller->CraftWidget->GamepadStartFire.Broadcast();
		return;
	}
	else if (bIsCraftingWindowOpen)
	{
		Controller->CraftsConfirmCraft();
		return;
	}
	else if (bLootWindowOpen)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->LootWidget->LootSingleDelegate.Broadcast();
		}
	}
	else
	{
		UseItem(Controller->MyInventory[Controller->InventoryWidget->CurrentlySelectedSlot]);
	}


}

void ACavemanCharacter::InventoryDrop()
{
	if (bHasDied)return;
	if (Controller->bIsAssignmentMenuOpen) return;
	if (bLootWindowOpen)return;

	if (bIsCraftingAtTheMoment)
	{
		Controller->CraftWidget->InterruptCrafting.Broadcast();
	}

	if (bIsStartingFireAtTheMoment)
	{
		Controller->CraftWidget->InterruptStartingFire.Broadcast();
	}

	if (bIsFireStartingWindowEnabled || bIsFireMainMenuEnabled)
	{
		Controller->CraftWidget->GamepadRestCancel.Broadcast();
		Controller->CloseCrafts();
		return;
	}
	if (bIsCraftingWindowOpen)return;

	if (!Controller->bIsInventoryOpen) return;
	Controller->RemoveItemFromInventory(Controller->InventoryWidget->CurrentlySelectedSlot);
}

void ACavemanCharacter::InventoryAssign()
{
	if (bHasDied)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bLootWindowOpen)return;
	if (bIsFireStartingWindowEnabled)return;

	if (bLootWindowOpen) return;


	if (bIsFireMainMenuEnabled)
	{
		GLog->Log("Tsirla");
		Controller->CraftWidget->GamepadRest.Broadcast();
		return;
	}
	if (bIsCraftingWindowOpen)return;
	if (!Controller->bIsInventoryOpen) return;
	if (bIsCraftingWindowOpen)return;


	if (!Controller->bIsAssignmentMenuOpen)
	{
		if (Controller->GetNameBySlot(Controller->InventoryWidget->CurrentlySelectedSlot) != "")
			Controller->InventoryWidget->OnAssignmentViaGamepad.Broadcast();
	}
	else
	{
		Controller->InventoryWidget->OnAssignmentViaGamepadCancel.Broadcast();
	}

}

void ACavemanCharacter::InventoryDown()
{
	if (bHasDied)return;
	if (!Controller->bIsInventoryOpen && !bLootWindowOpen && !bIsCraftingWindowOpen) return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;

	if (bIsCraftingWindowOpen)
	{
		Controller->CraftsGoDown();
		return;
	}
	if (bLootWindowOpen)
	{

		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[0]->LootWidget->CurrentlySelectedSlot <= 11)
			{
				CamComps[0]->LootWidget->CurrentlySelectedSlot += 4;
				CamComps[0]->LootWidget->OnInteractionDokFinished.Broadcast();
			}
		}
	}
	else if (!Controller->bIsAssignmentMenuOpen)
	{
		if (Controller->InventoryWidget->CurrentlySelectedSlot <= 31)
		{
			Controller->InventoryWidget->CurrentlySelectedSlot += 8;
			Controller->InventoryWidget->OnInteractionDokFinished.Broadcast();
		}
	}
	else
	{
		if (Controller->GetTheSlotThatTheItemIsAssignedTo(Controller->InventoryWidget->CurrentlySelectedSlot) == 4)
		{

		}
		else
		{
			Controller->InventoryWidget->OnAssignmentViaGamepadSlot4.Broadcast();
		}
	}
}

void ACavemanCharacter::InventoryUp()
{
	if (bHasDied)return;
	if (!Controller->bIsInventoryOpen && !bLootWindowOpen && !bIsCraftingWindowOpen) return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsCraftingWindowOpen)
	{
		Controller->CraftsGoUp();
		return;
	}
	if (bLootWindowOpen)
	{

		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[0]->LootWidget->CurrentlySelectedSlot >= 3)
			{
				CamComps[0]->LootWidget->CurrentlySelectedSlot -= 4;
				CamComps[0]->LootWidget->OnInteractionDokFinished.Broadcast();
			}
		}
	}
	else if (!Controller->bIsAssignmentMenuOpen)
	{
		if (Controller->InventoryWidget->CurrentlySelectedSlot >= 8)
		{
			Controller->InventoryWidget->CurrentlySelectedSlot -= 8;
			Controller->InventoryWidget->OnInteractionDokFinished.Broadcast();
		}
	}
	else
	{
		if (Controller->GetTheSlotThatTheItemIsAssignedTo(Controller->InventoryWidget->CurrentlySelectedSlot) == 1)
		{

		}
		else
		{
			Controller->InventoryWidget->OnAssignmentViaGamepadSlot1.Broadcast();
		}
	}
}

void ACavemanCharacter::InventoryLeft()
{
	if (bHasDied)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;

	if (bIsFireStartingWindowEnabled)return;


	if (bIsRestingWindowOpen && !bIsResting)
	{
		Controller->CraftWidget->GamepadRestSliderLeft.Broadcast();
		return;
	}
	if (bIsFireMainMenuEnabled)return;
	if (!Controller->bIsInventoryOpen && !bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bLootWindowOpen)
	{

		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[0]->LootWidget->CurrentlySelectedSlot > 0)
			{
				CamComps[0]->LootWidget->CurrentlySelectedSlot -= 1;
				CamComps[0]->LootWidget->OnInteractionDokFinished.Broadcast();
			}
		}
	}
	else if (!Controller->bIsAssignmentMenuOpen)
	{
		if (Controller->InventoryWidget->CurrentlySelectedSlot > 0)
		{
			Controller->InventoryWidget->CurrentlySelectedSlot -= 1;
			Controller->InventoryWidget->OnInteractionDokFinished.Broadcast();
		}
	}
	else
	{
		if (Controller->GetTheSlotThatTheItemIsAssignedTo(Controller->InventoryWidget->CurrentlySelectedSlot) == 3)
		{

		}
		else
		{
			Controller->InventoryWidget->OnAssignmentViaGamepadSlot3.Broadcast();
		}
	}
}

void ACavemanCharacter::InventoryRight()
{
	if (bHasDied)return;

	if (bIsFireStartingWindowEnabled)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;

	if (bIsRestingWindowOpen && !bIsResting)
	{
		Controller->CraftWidget->GamepadRestSliderRight.Broadcast();
		return;
	}
	if (bIsFireMainMenuEnabled)return;
	if (!Controller->bIsInventoryOpen && !bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bLootWindowOpen)
	{

		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[0]->LootWidget->CurrentlySelectedSlot < 15)
			{
				CamComps[0]->LootWidget->CurrentlySelectedSlot += 1;
				CamComps[0]->LootWidget->OnInteractionDokFinished.Broadcast();
			}
		}
	}
	else if (!Controller->bIsAssignmentMenuOpen)
	{
		if (Controller->InventoryWidget->CurrentlySelectedSlot < 39)
		{
			Controller->InventoryWidget->CurrentlySelectedSlot += 1;
			Controller->InventoryWidget->OnInteractionDokFinished.Broadcast();
		}
	}
	else
	{
		if (Controller->GetTheSlotThatTheItemIsAssignedTo(Controller->InventoryWidget->CurrentlySelectedSlot) == 2)
		{

		}
		else
		{
			Controller->InventoryWidget->OnAssignmentViaGamepadSlot2.Broadcast();
		}
	}
}

void ACavemanCharacter::ToggleInventoryUI()
{
	if (bHasDied)return;

	if (HandSkeletalMesh->AnimationData.AnimToPlay == FirstPersonAttack)return;

	if (MyAnimInstance->bIsAttacking)return;

	if (HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonPull_Bow || HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonRelease_Bow || bBowReleased) return;

	if (MyAnimInstance->b3rdPersonHolding || MyAnimInstance->b3rdPersonPulling || MyAnimInstance->b3rdPersonReleasing)return;

	if (bIsResting)return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;

	if (bIsCraftingWindowOpen || bIsFireStartingWindowEnabled || bIsFireMainMenuEnabled)
	{
		Controller->CloseCrafts();
		return;
	}



	if (bLootWindowOpen)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		Controller->LastDiedEnemyThatWeLooted->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->CloseLootWindow();
		}
		bLootWindowOpen = false;
		return;
	}


	if (Controller->bIsAssignmentMenuOpen)Controller->InventoryWidget->OnAssignmentViaGamepadCancel.Broadcast();
	else (Controller->bIsInventoryOpen) ? Controller->CloseInventory() : Controller->OpenInventory();

	SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_WindowOpen);

}

void ACavemanCharacter::Attack()
{
	if (MyAnimInstance)
	{
		if (MyAnimInstance->Montage_IsPlaying(FallAnimHard) || bHasDied)return;
		if (Controller->bIsInventoryOpen) return;
		if (bLootWindowOpen) return;
		if (bIsCraftingWindowOpen)return;

		if (bIsFireStartingWindowEnabled)return;
		if (bIsFireMainMenuEnabled)return;

	}
	else
	{
		return;
	}

	if (MyAnimInstance->bIsGrounded && MyAnimInstance->bCanMoveWithInput)
	{
		if (bIsMelee)
		{
			if (bIs1stPerson && HandSkeletalMesh->AnimationData.AnimToPlay != FirstPersonAttack)
			{
				HandSkeletalMesh->AnimationData.AnimToPlay = FirstPersonAttack;
				HandSkeletalMesh->PlayAnimation(FirstPersonAttack, false);
			}
			else if (!bIs1stPerson && !MyAnimInstance->bIsAttacking)
			{
				MyAnimInstance->bIsAttacking = true;
				MyAnimInstance->Attack(MyAnimInstance->bIsAttacking);
			}
		}
		else
		{
			if (bIs1stPerson && HandSkeletalMeshBow->AnimationData.AnimToPlay != FirstPersonPull_Bow && HandSkeletalMesh->AnimationData.AnimToPlay != FirstPersonRelease_Bow)
			{
				if (!bCannotReAttack && ArrowsLeft > 0)
				{
					SFXManager->PlaySoundEffect(ESoundEffects::SFX_ArrowPulled);
					HandSkeletalMeshBow->AnimationData.AnimToPlay = FirstPersonPull_Bow;
					HandSkeletalMeshBow->PlayAnimation(FirstPersonPull_Bow, false);
					bCannotReAttack = true;
					bBowReleased = false;
					MyAnimInstance->bCanMoveWithInput = false;
				}
			}
			else if (!bIs1stPerson && !MyAnimInstance->b3rdPersonPulling && !MyAnimInstance->b3rdPersonReleasing)
			{

				if (ArrowsLeft > 0)
				{
					SFXManager->PlaySoundEffect(ESoundEffects::SFX_ArrowPulled);
					MyAnimInstance->PullTheBow();
					MyAnimInstance->b3rdPersonPulling = true;
					TheArrowThatWeHoldBeforeWeFire->SetOwnerNoSee(false);
				}
			}
		}
	}

}

void ACavemanCharacter::AttackRelease()
{
	if (MyAnimInstance)
	{
		if (MyAnimInstance->Montage_IsPlaying(FallAnimHard) || bHasDied)return;
		if (Controller->bIsInventoryOpen) return;
		if (bLootWindowOpen) return;
		if (bIsCraftingWindowOpen)return;

		if (bIsFireStartingWindowEnabled)return;
		if (bIsFireMainMenuEnabled)return;
	}
	else
	{
		return;
	}


	if (!bIsMelee)
	{
		if (bIs1stPerson && HandSkeletalMesh->AnimationData.AnimToPlay != FirstPersonRelease_Bow)
		{
			if (ArrowsLeft > 0)bBowReleased = true;
			else bBowReleased = false;

		}
		else if (!bIs1stPerson && !MyAnimInstance->b3rdPersonReleasing)
		{
			MyAnimInstance->ReleaseTheBow();
			TheArrowThatWeHoldBeforeWeFire->SetOwnerNoSee(true);
		}
	}
}

void ACavemanCharacter::SprintingON()
{
	if (Stamina_Current / Stamina_Max > Stamina_Threshold)
	{
		isSprinting = true;
		MyAnimInstance->bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = 2 * NormalWalkSpeed;
	}
}

void ACavemanCharacter::SprintingOFF()
{
	isSprinting = false;
	MyAnimInstance->bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
}

void ACavemanCharacter::TogglePOVFunc()
{
	if (Controller->bIsInventoryOpen) return;
	if (bIsCraftingAtTheMoment)return;
	if (bIsStartingFireAtTheMoment)return;
	if (bIsResting)return;
	if (bLootWindowOpen) return;

	if (bHasDied)return;

	if (HandSkeletalMesh->AnimationData.AnimToPlay == FirstPersonAttack)return;

	if (MyAnimInstance->bIsAttacking)return;

	if (HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonPull_Bow || HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonRelease_Bow || bBowReleased) return;

	if (MyAnimInstance->b3rdPersonHolding || MyAnimInstance->b3rdPersonPulling || MyAnimInstance->b3rdPersonReleasing)return;

	if (bIsCraftingWindowOpen)return;

	if (bIsFireStartingWindowEnabled || bIsFireMainMenuEnabled) return;

	bIsChangingCameras = true;

	bIs1stPerson = !bIs1stPerson;

	if (!bIs1stPerson)
	{
		FVector FWD = FirstPersonCamera->GetForwardVector();
		FirstPersonCamera->Deactivate();
		//ThirdPersonCamera->SetWorldRotation(FWD.Rotation());

		//ThirdPersonCamera->Activate();


		LastKnownRotation = FRotator(0, FWD.Rotation().Yaw, 0) - FRotator(0, 90, 0);
		TheCavemanSkeletalMesh->SetWorldRotation(LastKnownRotation);

		TheCavemanSkeletalMesh->SetOwnerNoSee(false);
		if (bIsMelee)
		{
			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
			HandSkeletalMeshBow->SetOwnerNoSee(true);

			OurGreatClubComponent3rdPerson->SetOwnerNoSee(false);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			HandSkeletalMesh->SetOwnerNoSee(true);
		}
		else
		{
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			HandSkeletalMesh->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);

			OurGreatBowComponent3rdPerson->SetOwnerNoSee(false);

			HandSkeletalMeshBow->SetOwnerNoSee(true);
		}

	}
	else
	{
		//ThirdPersonCamera->Deactivate();
		FirstPersonCamera->Activate();


		TheCavemanSkeletalMesh->SetOwnerNoSee(true);
		if (bIsMelee)
		{
			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
			HandSkeletalMeshBow->SetOwnerNoSee(true);

			HandSkeletalMesh->SetOwnerNoSee(false);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
			OurGreatClubComponent1stPerson->SetOwnerNoSee(false);
		}
		else
		{
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			HandSkeletalMesh->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(false);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
			HandSkeletalMeshBow->SetOwnerNoSee(false);
		}
	}

	bIsChangingCameras = false;

}

void ACavemanCharacter::MoveXAxis(float AxisValue)
{
	if (Controller->bIsInventoryOpen) return;
	if (bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;
	if (bIsCraftingWindowOpen)return;
	if (bIsInTrapMode) return;

	if (MyAnimInstance)
	{
		if (!MyAnimInstance->bCanMoveWithInput || bHasDied)
		{
			X_AxisValue = 0;
			return;
		}
	}
	else
	{
		return;
	}

	X_AxisValue = AxisValue;

	if (MyAnimInstance)
		MyAnimInstance->X_Axis = X_AxisValue;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue);
	}
}

void ACavemanCharacter::MoveYAxis(float AxisValue)
{
	if (Controller->bIsInventoryOpen) return;
	if (bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;
	if (bIsCraftingWindowOpen)return;
	if (bIsInTrapMode) return;

	if (MyAnimInstance)
	{
		if (!MyAnimInstance->bCanMoveWithInput || bHasDied)
		{
			Y_AxisValue = 0;
			return;
		}
	}
	else
	{
		return;
	}

	Y_AxisValue = AxisValue;

	if (MyAnimInstance)
		MyAnimInstance->Y_Axis = Y_AxisValue;

	if (FMath::Abs(AxisValue) > 0.05f)
	{
		AddMovementInput(GetActorRightVector(), AxisValue);
	}
}

void ACavemanCharacter::MouseTurn(float AxisValue)
{
	if (Controller->bIsInventoryOpen) return;
	if (bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;
	if (bIsCraftingWindowOpen)return;

	if (bIsChangingCameras) return;

	if (FMath::Abs(AxisValue) > 0.005f)
	{
		AddControllerYawInput(AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds());
		SetNewTrapLocation();
	}
}

void ACavemanCharacter::MouseTurnUp(float AxisValue)
{
	if (Controller->bIsInventoryOpen) return;
	if (bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;
	if (bIsFireStartingWindowEnabled)return;
	if (bIsFireMainMenuEnabled)return;
	if (bIsCraftingWindowOpen)return;

	if (bHasDied)return;

	if (bIsChangingCameras) return;

	if (FMath::Abs(AxisValue) > 0.005f)
	{
		AddControllerPitchInput(AxisValue * TurningSpeed * GetWorld()->GetDeltaSeconds());
		SetNewTrapLocation();
	}

}

void ACavemanCharacter::MyCustomAction()
{
	if (Controller->bIsInventoryOpen) return;
	if (bLootWindowOpen) return;
	if (bIsCraftingWindowOpen)return;

	if (MyAnimInstance)
	{
		if (MyAnimInstance->Montage_IsPlaying(FallAnimHard) || bHasDied)return;
	}
	else
	{
		return;
	}

	if (MyAnimInstance->bIsGrounded && !MyAnimInstance->bHasJumped)
	{

		//	TheCavemanSkeletalMesh->AnimationData.AnimToPlay = JumpAnim;
		//TheCavemanSkeletalMesh->PlayAnimation(JumpAnim, false);
		//CharacterMovement->fly

		if (bShouldCountFallingMeters || MyAnimInstance->bStartCountingMeters) return;

		SFXManager->PlaySoundEffect(ESoundEffects::SFX_CharacterJump);
		Jump();
		Meters = 0;
		MyAnimInstance->MetersFallen = 0;
		MyAnimInstance->bHasJumped = true;
		//CharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying);
	}
}

void ACavemanCharacter::ToggleWeaponMode()
{

	if (Controller->bIsInventoryOpen) return;

	if (bLootWindowOpen) return;

	if (bHasDied)return;

	if (HandSkeletalMesh->AnimationData.AnimToPlay == FirstPersonAttack)return;

	if (MyAnimInstance->bIsAttacking)return;

	if (HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonPull_Bow || HandSkeletalMeshBow->AnimationData.AnimToPlay == FirstPersonRelease_Bow || bBowReleased) return;

	if (MyAnimInstance->b3rdPersonHolding || MyAnimInstance->b3rdPersonPulling || MyAnimInstance->b3rdPersonReleasing)return;
	if (bIsCraftingWindowOpen)return;

	if (bIsFireStartingWindowEnabled || bIsFireMainMenuEnabled)return;

	Controller->UberWidget->OnToggle.Broadcast();

	bIsMelee = !bIsMelee;

	WeaponName = (bIsMelee) ? "Worn Club" : "Short Bow";
	SFXManager->PlaySoundEffect(ESoundEffects::SFX_UI_Swap);
	HandSkeletalMesh->SetOwnerNoSee(true);
	HandSkeletalMeshBow->SetOwnerNoSee(true);

	if (!bIs1stPerson)
	{
		if (bIsMelee)
		{
			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(false);
		}
		else
		{
			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(false);
		}
	}
	else
	{
		if (bIsMelee)
		{
			HandSkeletalMesh->SetOwnerNoSee(false);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(false);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
		}
		else
		{
			HandSkeletalMeshBow->SetOwnerNoSee(false);

			OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
			OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);

			OurGreatBowComponent1stPerson->SetOwnerNoSee(false);
			OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
		}
	}

}

#pragma endregion

#pragma region Character Trait Updating Mechanics

void ACavemanCharacter::Health_Up(float Heal)
{
	Health_Current += Heal;
	if (Health_Current >= Health_Max)
	{
		Health_Current = Health_Max;
	}
	Controller->UberWidget->Health = UStaticFuncLib::GetFloatAsStringWithPrecision(((Health_Current / Health_Max) * 100),2) + FString("%");
	Controller->UberWidget->HealthBar = Health_Current / Health_Max;
}

void ACavemanCharacter::Health_Down(float Damage)
{
	Health_Current -= Damage;
	if (Health_Current <= 0)
	{
		Controller->UberWidget->RemoveFromViewport();
		KillTheCharacter();
	}
	Controller->UberWidget->Health = UStaticFuncLib::GetFloatAsStringWithPrecision(((Health_Current / Health_Max) * 100),2) + FString("%");
	Controller->UberWidget->HealthBar = Health_Current / Health_Max;

}

void ACavemanCharacter::Hunger_Up(float HungerPts)
{
	Hunger_Current += HungerPts;
	bStarvationHasStarted = false;

	if (Hunger_Current >= Hunger_Max)
	{
		Hunger_Current = Hunger_Max;
	}
	Controller->UberWidget->Hunger = UStaticFuncLib::GetFloatAsStringWithPrecision(((Hunger_Current / Hunger_Max) * 100), 2) + FString("%");
	Controller->UberWidget->HungerBar = Hunger_Current / Hunger_Max;
}

void ACavemanCharacter::Hunger_Down(float HungerPts)
{
	if (bStarvationHasStarted)return;

	Hunger_Current -= HungerPts;
	if (Hunger_Current <= 0 && !bStarvationHasStarted)
	{
		Hunger_Current = 0;
		bStarvationHasStarted = true;
	}

	Controller->UberWidget->Hunger = UStaticFuncLib::GetFloatAsStringWithPrecision(((Hunger_Current / Hunger_Max) * 100),2) + FString("%");
	Controller->UberWidget->HungerBar = Hunger_Current / Hunger_Max;
}

void ACavemanCharacter::Heat_Up(float HeatPts)
{
	Heat_Current += HeatPts;
	if ((Heat_Current / Heat_Max) * 100 > 20)
	{
		bIsDiseased = false;
	}
	if (Heat_Current > Heat_Max)
	{
		Heat_Current = Heat_Max;
	}
	Controller->UberWidget->Heat = UStaticFuncLib::GetFloatAsStringWithPrecision(((Heat_Current / Heat_Max) * 100),2) + FString("%");
	Controller->UberWidget->HeatBar = Heat_Current / Heat_Max;
}

void ACavemanCharacter::Heat_Down(float HeatPts)
{
	Heat_Current -= HeatPts;
	if ((Heat_Current / Heat_Max) * 100 <= 20)
	{
		// pros to paron MPALES.
		bIsDiseased = true;
	}
	Controller->UberWidget->Heat = FString::SanitizeFloat((Heat_Current / Heat_Max) * 100) + FString("%");
	Controller->UberWidget->HeatBar = Heat_Current / Heat_Max;
}

void ACavemanCharacter::Stamina_Up(float StaPts)
{
	Stamina_Current += StaPts;
	if (Stamina_Current > Stamina_Max)
	{
		Stamina_Current = Stamina_Max;
	}
	Controller->UberWidget->Stamina = UStaticFuncLib::GetFloatAsStringWithPrecision(((Stamina_Current / Stamina_Max) * 100),2) + FString("%");
	Controller->UberWidget->StaminaBar = Stamina_Current / Stamina_Max;
}

void ACavemanCharacter::Stamina_Down(float StaPts)
{
	Stamina_Current -= StaPts;
	if (Stamina_Current < 0)
	{
		Stamina_Current = 0;
		isSprinting = false;
		MyAnimInstance->bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	}

	
	Controller->UberWidget->Stamina = UStaticFuncLib::GetFloatAsStringWithPrecision(((Stamina_Current / Stamina_Max) * 100),2) + FString("%");
	Controller->UberWidget->StaminaBar = Stamina_Current / Stamina_Max;
}

void ACavemanCharacter::HungerOverTime(float DeltaTime)
{
	Hunger_Drop_Timer += DeltaTime;
	if (Hunger_Drop_Timer > Hunger_Drop_CD)
	{
		Hunger_Drop_Timer = 0;
		Hunger_Down(Hunger_DropRate * (bIsDiseased) ? 1.5f : 1.f);
	}
	Controller->UberWidget->Stamina = UStaticFuncLib::GetFloatAsStringWithPrecision(((Stamina_Current / Stamina_Max) * 100),2) + FString("%");
	Controller->UberWidget->StaminaBar = Stamina_Current / Stamina_Max;
}

void ACavemanCharacter::HeatOverTime(float DeltaTime)
{
	if (bIsInFireRadius)
	{
		Heat_Drop_Timer += DeltaTime;
		if (Heat_Drop_Timer > Heat_Drop_CD)
		{
			Heat_Drop_Timer = 0;
			Heat_Up(Heat_DropRate * 2);
		}
		Controller->UberWidget->Heat = UStaticFuncLib::GetFloatAsStringWithPrecision(((Heat_Current / Heat_Max) * 100),2) + FString("%");
		Controller->UberWidget->HeatBar = Heat_Current / Heat_Max;
	}
	else
	{
		Heat_Drop_Timer += DeltaTime;
		if (Heat_Drop_Timer > Heat_Drop_CD)
		{
			Heat_Drop_Timer = 0;
			Heat_Down(Heat_DropRate);
		}
		Controller->UberWidget->Heat = UStaticFuncLib::GetFloatAsStringWithPrecision(((Heat_Current / Heat_Max) * 100),2) + FString("%");
		Controller->UberWidget->HeatBar = Heat_Current / Heat_Max;
	}
}

void ACavemanCharacter::StarvationOverTime(float DeltaTime)
{
	Health_Drop_Timer += DeltaTime;
	if (Health_Drop_Timer > Health_Drop_CD)
	{
		Health_Drop_Timer = 0;
		Health_Down(Health_DropRate);
	}
}

#pragma endregion

#pragma region Lock On Mechanics

bool ACavemanCharacter::TryToLockOn()
{
	if (!bIs1stPerson)
	{

		TArray<FHitResult> HitActors;

		FCollisionShape Shape;
		Shape.ShapeType = ECollisionShape::Sphere;
		Shape.SetSphere(MaxLockOnRange);
		GLog->Log("Trying to lock on.");

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		if (GetWorld()->SweepMultiByChannel(HitActors, GetActorLocation(), GetActorLocation() + FVector(0, 0, 200),
			FQuat::FQuat(), ECollisionChannel::ECC_WorldDynamic, Shape, CollisionParams))
		{
			GLog->Log("Found smth to lock on");
			float MinRange = MaxLockOnRange;
			for (auto It = HitActors.CreateIterator(); It; It++)
			{
				if (!(*It).GetActor()->GetRootComponent()->ComponentHasTag("Enemy"))
				{
					continue;
				}
				//Calculate the distance for each enemy
				FVector TempActorLoc = (*It).GetActor()->GetActorLocation();
				float DistanceFromCaveman = (TempActorLoc - GetActorLocation()).Size();
				if (DistanceFromCaveman < MinRange)
				{
					MinRange = DistanceFromCaveman;
					EnemyTarget = (*It).GetActor();
				}
			}

			if (EnemyTarget)
			{
				bHasLockedTarget = true;
				GLog->Log("Current Lock-On Target: " + EnemyTarget->GetName());
			}
			else
			{
				bHasLockedTarget = false;
			}
		}
		GLog->Log("Trying to visualize max lock on sphere");
		//Sphere visualization
		FVector CenterOfSphere = FVector(0, 0, 100) + GetActorLocation();

		DrawDebugSphere(GetWorld(), CenterOfSphere, MaxLockOnRange, 100, FColor::Blue, false, 2.f, 2.f);
	}
	return false;
}

void ACavemanCharacter::ClearLockOn()
{
	bHasLockedTarget = false;
	SpringArmComponent->bUsePawnControlRotation = true;
	EnemyTarget = nullptr;
}

#pragma endregion

#pragma region Damaging Mechanics

void ACavemanCharacter::DamageEnemy(AActor* HitEnemy)
{
	if (HitEnemy)
	{
		//Get the game mode and damage the enemy using the game mode
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameMode::StaticClass(), Actors);
		ACavemanGameMode* GameMode = Cast<ACavemanGameMode>(Actors[0]);

		TArray<FName> EnemyTags;
		EnemyTags = HitEnemy->Tags;
		GLog->Log("1");
		DamageGivenEnemy(HitEnemy, this->GetController()->GetPawn());

	}
}

void ACavemanCharacter::DamageGivenEnemy(AActor* HitEnemy, APawn* PawnWhoPerformedTheHit)
{
	if (HitEnemy)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		HitEnemy->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			if (CamComps[i]->ComponentHasTag("TheStatComp"))
			{
				if (CamComps[i]->CurrentHealth > 0)
				{
					CamComps[i]->TakeDamageFromEnemy(CauseDamageByMelee(), PawnWhoPerformedTheHit);

					//Enable combat
					SetCombatStatus(true);
				}
			}
		}
	}
}

void ACavemanCharacter::TakeDamageFromEnemy(float Damage)
{
	float DmgToTake = Damage - (BaseDef * Damage);
	Health_Down(DmgToTake);
	MyAnimInstance->GetHurt();
	//Enable combat
	SetCombatStatus(true);
}

float ACavemanCharacter::CauseDamageByMelee()
{
	float Damage = 0;

	Damage = BaseDamageMelee + FMath::RandRange(MinDamageMelee, MaxDamageMelee);

	return Damage;
}

float ACavemanCharacter::CauseDamageByRanged()
{
	float Damage = 0;

	Damage = BaseDamageRanged + FMath::RandRange(MinDamageRanged, MaxDamageRanged);

	return Damage;
}

void ACavemanCharacter::KillTheCharacter()
{
	if (bHasDied) return;
	bHasDied = true;

	FirstPersonCamera->Deactivate();
	ThirdPersonCamera->Activate();
	OurGreatClubComponent1stPerson->SetOwnerNoSee(true);
	OurGreatClubComponent3rdPerson->SetOwnerNoSee(true);
	HandSkeletalMesh->SetOwnerNoSee(true);
	HandSkeletalMeshBow->SetOwnerNoSee(true);
	OurGreatBowComponent1stPerson->SetOwnerNoSee(true);
	OurGreatBowComponent3rdPerson->SetOwnerNoSee(true);
	Controller->CloseInventory();
	Controller->CloseCrafts();

	SFXManager->PlaySoundEffect(ESoundEffects::SFX_CharacterDeath);

	if (bLootWindowOpen)
	{
		TArray<UEnemyStatsComponent*> CamComps;
		InteractionTarget->GetComponents<UEnemyStatsComponent>(CamComps);
		for (int32 i = 0; i < CamComps.Num(); i++)
		{
			CamComps[0]->CloseLootWindow();
		}
		bLootWindowOpen = false;
		return;
	}
	TheCavemanSkeletalMesh->Stop();
	TheCavemanSkeletalMesh->SetOwnerNoSee(false);
	TheCavemanSkeletalMesh->SetSimulatePhysics(true);
	TheCavemanSkeletalMesh->SetAllBodiesSimulatePhysics(true);
	TheCavemanSkeletalMesh->WakeAllRigidBodies();
	TheCavemanSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
	SetCombatStatus(false);
}

#pragma endregion

#pragma region Combat State Oriented Mechanics

void ACavemanCharacter::SetCombatStatus(bool IsInCombat)
{
	bIsInCombat = IsInCombat;
	if (bIsInCombat)
	{
		//GLog->Log("Combat has been enabled.");
		CombatStartSeconds = GetWorld()->TimeSeconds;

		if (!Firebrand->IsPlaying())
			Firebrand->Play();

		if (bIsCraftingAtTheMoment)
		{
			Controller->CraftWidget->InterruptCrafting.Broadcast();
		}

		if (bIsStartingFireAtTheMoment)
		{
			Controller->CraftWidget->InterruptStartingFire.Broadcast();
		}
	}
	else
	{
		GLog->Log("Combat has been disabled.");



		Firebrand->Stop();
	}
}

void ACavemanCharacter::CavemanMakeNoise(float Loudness)
{
	MakeNoise(Loudness, this, GetActorLocation());
}

#pragma endregion

#pragma region Fire Starter Mechanics

void ACavemanCharacter::EnableTheFireOnThisCampfire(bool BigFire)
{
	TArray<UParticleSystemComponent*> Comps;
	InteractionTarget->GetComponents<UParticleSystemComponent>(Comps);
	for (int32 i = 0; i < Comps.Num(); i++)
	{
		if (Comps[i]->ComponentTags.Contains("FireBebe"))
		{
			if (!BigFire)
				Comps[i]->SetVisibility(true);
		}
		if (Comps[i]->ComponentTags.Contains("FireBig"))
		{
			if (BigFire)
			{
				Comps[i]->SetVisibility(true);
				Comps[i]->GetAttachmentRoot()->ComponentTags.Add("CampfireActive");
			}
		}
	}
}

void ACavemanCharacter::DisableTheFireOnThisCampfire(bool BigFire)
{
	TArray<UParticleSystemComponent*> Comps;
	InteractionTarget->GetComponents<UParticleSystemComponent>(Comps);
	for (int32 i = 0; i < Comps.Num(); i++)
	{
		if (Comps[i]->ComponentTags.Contains("FireBebe"))
		{
			if (!BigFire)
				Comps[i]->SetVisibility(false);
		}
		if (Comps[i]->ComponentTags.Contains("FireBig"))
		{
			if (BigFire)
				Comps[i]->SetVisibility(false);
		}
	}
}

#pragma endregion

#pragma region Collision Mechanics

void ACavemanCharacter::OnCavemanEndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp->GetAttachmentRoot()->ComponentTags.Contains("CampfireActive"))
	{
		bIsInFireRadius = false;
	}
}

void ACavemanCharacter::OnCavemanBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherComp->GetAttachmentRoot()->ComponentTags.Contains("CampfireActive"))
	{
		bIsInFireRadius = true;
	}
}

#pragma endregion

#pragma region Trap Mechanics

void ACavemanCharacter::PlaceTrap()
{
	if (bIsInTrapMode)
	{
		bIsInTrapMode = false;
		//GLog->Log("Placed Trap");
		//Disable reference to the newly placed trap.
		TrapToBePlaced = nullptr;
	}
	else
	{
		bIsInTrapMode = true;
		//GLog->Log("Spawned trap");
	
		FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * SpawnDistance);
		TrapToBePlaced = GetWorld()->SpawnActor<ATrap>(TrapToSpawn, SpawnLocation, FRotator(0));
		SetNewTrapLocation();
	}

}

void ACavemanCharacter::SetNewTrapLocation()
{
	if (TrapToBePlaced)
	{
		FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * SpawnDistance);
		SpawnLocation.Y -= TrapToBePlaced->TrapCollider->GetUnscaledBoxExtent().Y;

		FVector EndLocation = SpawnLocation;
		//We want a vertical vector in order to raycast on landscape
		EndLocation.Z = 0;
		
		FHitResult Hit;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(TrapToBePlaced);
		//Sweep in order to find the landscape impact point (the trap must always be placed into the ground)
		GetWorld()->SweepSingleByChannel(Hit, SpawnLocation, EndLocation, FQuat::Identity, ECollisionChannel::ECC_WorldStatic, FCollisionShape::FCollisionShape(), CollisionQueryParams);
		
		TrapToBePlaced->SetActorLocation(Hit.ImpactPoint);
	}
}

#pragma endregion