// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "PointAndClick.h"
#include "PointAndClickPlayerController.h"
#include "EngineUtils.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "PointAndClickCharacter.h"
#include "AI/Navigation/NavigationSystem.h"

#pragma region Core Functions

APointAndClickPlayerController::APointAndClickPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	
}

void APointAndClickPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UI_Holder)
	{
		LetterboxUI = Cast<ULetterboxDialogues>(CreateWidget<UUserWidget>(this, UI_Holder));
		LetterboxUI->AddToViewport();
		LetterboxUI->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (UI_Holder2)
	{
		MyInventoryUI = Cast<UInventoryUI>(CreateWidget<UUserWidget>(this, UI_Holder2));
		MyInventoryUI->AddToViewport();

		for (size_t i = 0; i < 10; i++)
		{
			FInventoryItem NewItem = FInventoryItem();
			NewItem.ItemID = -1;
			MyInventoryUI->ItemsOnInventory.Add(NewItem);
		}

		MyInventoryUI->SetVisibility(ESlateVisibility::Visible);
	}


	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->ActorHasTag("CursorThing"))
		{
			CursorMan = *ActorItr;
			TArray<UWidgetComponent*> Comps;
			CursorMan->GetComponents<UWidgetComponent>(Comps);

			TooltipUI = Cast<UUserWidget>(Comps[0]->GetUserWidgetObject());

			const FName TextControlName = FName(TEXT("CustomAdTooltip"));
			
			if (MyCursorTextBlockControl == nullptr)
				MyCursorTextBlockControl = (UTextBlock*)(TooltipUI->WidgetTree->FindWidget(TextControlName));

			MyAnimInstance = Cast<UCharacterAnimInstance>(GetCharacter()->GetMesh()->GetAnimInstance());
		}
		if (ActorItr->ActorHasTag("VisualHelper"))
		{
			VisualHelper = ActorItr->FindComponentByClass<class UStaticMeshComponent>();
		}
	}

	TArray<UWidgetComponent*> Comps;
	GetCharacter()->GetComponents<UWidgetComponent>(Comps);

	const FName TextControlName = FName(TEXT("MainText"));

	if (MySubtitleTextBlockControl == nullptr)
		MySubtitleTextBlockControl = (UTextBlock*)(Cast<UUserWidget>(Comps[0]->GetUserWidgetObject())->WidgetTree->FindWidget(TextControlName));

	TArray<UDialogueSystem*> Systems;
	GetCharacter()->GetComponents<UDialogueSystem>(Systems);

	TArray<UAudioComponent*> AudComp;
	GetCharacter()->GetComponents<UAudioComponent>(AudComp);
	UAudioComponent* SfxComp = AudComp[0];
	GlobalAudioComponent = AudComp[1];

	DialogueSystem = Systems[0];
	DialogueSystem->InitializeTypewriter(MySubtitleTextBlockControl,SfxComp,LetterboxUI);
}

void APointAndClickPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CharacterAndAnimationPointers();
	MainInteraction();
	CursorFunctionality();
}

#pragma endregion

#pragma region Input

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 											Input Functionality												   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void APointAndClickPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &APointAndClickPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &APointAndClickPlayerController::OnSetDestinationReleased);

	InputComponent->BindAction("RMB", IE_Pressed, this, &APointAndClickPlayerController::RMB);
	InputComponent->BindAction("LMB", IE_Pressed, this, &APointAndClickPlayerController::LMB);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &APointAndClickPlayerController::MoveToTouchLocation);
	//InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &APointAndClickPlayerController::MoveToTouchLocation);
}

void APointAndClickPlayerController::RMB()
{
	if (bIsInDialogueMode)return;

	if (MyInventoryUI->CurrentItemEquipped != "")
	{
		int32 Index = MyInventoryUI->GetIndexOfItemInArrayByName(MyInventoryUI->CurrentItemEquipped);
		MyInventoryUI->ButtonsSelected[Index] = false;

		MyInventoryUI->CurrentItemEquipped = "";
	}

	if (!DialogueSystem->DialogueQueue.IsEmpty())
	{
		return;
	}

	MyInventoryUI->RMB();

	AInteractiveObject* Hovered = nullptr;

	Hovered = GetInteractiveHoveredActor();

	if (Hovered != nullptr)
	{
			InteractiveObjectLocation = Hovered->GetActorLocation();
			InteractionRange = Hovered->InspectionRange;
			SetNewMoveDestination(Cast<AActor>(Hovered));
			bTravellingToInteractiveObject = true;
			LastInteractedActor = Hovered;

			bLastInteractionWas_Interact = false;
			bLastInteractionWas_Inspect = true;
		
	}
}

void APointAndClickPlayerController::LMB()
{
	if (bIsInDialogueMode)return;

	if (!DialogueSystem->DialogueQueue.IsEmpty())
	{
		DialogueSystem->SkipToNextDialoguePart();
		return;
	}

	AInteractiveObject* Hovered = nullptr;

	Hovered = GetInteractiveHoveredActor();

	if (Hovered != nullptr)
	{
		GLog->Log("Interact With Interactive Item!");
	}
}

#pragma endregion

#pragma region Navigation

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 										Navigation Helper Functions											   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void APointAndClickPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, ECollisionChannel::ECC_WorldDynamic, true, HitResult);


	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void APointAndClickPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	bTravellingToInteractiveObject = false;
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			LastClickedLocation = DestLocation;
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void APointAndClickPlayerController::SetNewMoveDestination(const AActor* TargetActor)
{
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(TargetActor->GetActorLocation(), Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			LastClickedLocation = TargetActor->GetActorLocation();
			NavSys->SimpleMoveToActor(this, TargetActor);
		}
	}
}

void APointAndClickPlayerController::OnSetDestinationPressed()
{
	if (bIsInDialogueMode)return;

	if (!DialogueSystem->DialogueQueue.IsEmpty())
	{
		return;
	}

	AInteractiveObject* Hovered = nullptr;

	Hovered = GetInteractiveHoveredActor();

	if (Hovered == nullptr)
	{
		// set flag to keep updating destination until released
		bMoveToMouseCursor = true;
		MoveToMouseCursor();
	}
	else
	{

		InteractiveObjectLocation = Hovered->GetActorLocation();
		InteractionRange = Hovered->InteractionRange;
		SetNewMoveDestination(Cast<AActor>(Hovered));
		bTravellingToInteractiveObject = true;
		LastInteractedActor = Hovered;
		bLastInteractionWas_Interact = true;
		bLastInteractionWas_Inspect = false;
	}


}

void APointAndClickPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void APointAndClickPlayerController::MoveToMouseCursor()
{
	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	if (Hit.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(Hit.ImpactPoint);
	}
}

#pragma endregion

#pragma region Typewriter Functions

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 										Typewriter System Calls												   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void APointAndClickPlayerController::EnqueueTextForTypewriter(FString Text, USoundWave* Sfx)
{
	DialogueSystem->DialogueQueue.Enqueue(Text);
	DialogueSystem->SfxQueue.Enqueue(Sfx);

	bTravellingToInteractiveObject = false;
	InteractiveObjectLocation = FVector(0);

	GetWorld()->GetNavigationSystem()->SimpleMoveToLocation(this, GetCharacter()->GetActorLocation());
}

void APointAndClickPlayerController::EnqueueTextForTypewriter(FString Text, USoundWave* Sfx, EAdventureCharacterIDs Name)
{
	DialogueSystem->DialogueQueue.Enqueue(Text);
	DialogueSystem->SfxQueue.Enqueue(Sfx);

	if (Name == EAdventureCharacterIDs::Timmy)
	{
		DialogueSystem->CharsQueue.Enqueue(WorldCharacters[0]);
	}
	else if (Name == EAdventureCharacterIDs::Jimbo)
	{
		DialogueSystem->CharsQueue.Enqueue(WorldCharacters[1]);
	}
	else if (Name == EAdventureCharacterIDs::Roberto)
	{
		DialogueSystem->CharsQueue.Enqueue(WorldCharacters[3]);
	}
	else if (Name == EAdventureCharacterIDs::Claire)
	{
		DialogueSystem->CharsQueue.Enqueue(WorldCharacters[2]);
	}

	

	bTravellingToInteractiveObject = false;
	InteractiveObjectLocation = FVector(0);



	GetWorld()->GetNavigationSystem()->SimpleMoveToLocation(this, GetCharacter()->GetActorLocation());
}

#pragma endregion

#pragma region Helper Functions

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 											Helper Functions												   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FRotator APointAndClickPlayerController::FindLookAtRotation(FVector Start, FVector Target)
{
	FVector Direction = Target - Start;
	FRotator FinalRotation = Direction.Rotation();
	return FinalRotation;
}

AInteractiveObject* APointAndClickPlayerController::GetInteractiveHoveredActor()
{
	FHitResult HitResult;
	switch (MyCursorTextBlockControl->GetVisibility())
	{
	case ESlateVisibility::Visible:
		if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
		{
			CursorMan->SetActorLocation(HitResult.Location);
			AActor* LeActor = HitResult.Actor.Get();
			return Cast<AInteractiveObject>(LeActor);
		}
		break;
	case ESlateVisibility::Collapsed:
		return nullptr;
		break;
	}

	return nullptr;
}

void APointAndClickPlayerController::UpdateLetterboxUIVisibility(ESlateVisibility NewVisibility)
{
	LetterboxUI->SetVisibility(NewVisibility);
	MyInventoryUI->SetVisibility((LetterboxUI->GetVisibility() == ESlateVisibility::Visible) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (NewVisibility == ESlateVisibility::Visible)
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(LetterboxUI->GetCachedWidget());
		SetInputMode(Mode);
	}
}

ULetterboxDialogues * APointAndClickPlayerController::GetLetterboxUI()
{
	return LetterboxUI;
}

#pragma endregion

#pragma region Gameplay Function

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 										Typewriter System Calls												   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void APointAndClickPlayerController::Special_Item_Checker(FString Item)
{
	if ((LastInteractedActor->ObjectName == "Well" && Item == "Golden Coin") || 
		(LastInteractedActor->ObjectName == "Claire" && Item == "Lost Package") || 
		(LastInteractedActor->ObjectName == "Powerbox" && Item == "Big Ol' Wrench"))
	{
		bIsInDialogueMode = true;
		DialogueSystem->LastInteractedActor = LastInteractedActor;

		GetCharacter()->SetActorLocation(FVector(Cast<AInteractiveObject>(DialogueSystem->LastInteractedActor)->CharacterPositionNode.X,
			Cast<AInteractiveObject>(DialogueSystem->LastInteractedActor)->CharacterPositionNode.Y,
			GetCharacter()->GetActorLocation().Z));

		GetCharacter()->SetActorRotation(
			FRotator(GetCharacter()->GetActorRotation().Pitch,
				FindLookAtRotation(GetCharacter()->GetActorLocation(), LastInteractedActor->GetActorLocation()).Yaw,
				GetCharacter()->GetActorRotation().Roll)
			);

		LastInteractedActor->InitiateDialogue();
	}
	else
	{
		int32 Rand = FMath::RandRange(0, 6);
		EnqueueTextForTypewriter(DialogueSystem->DialogueBanches[0].ThingsToSay[Rand], DialogueSystem->DialogueBanches[0].SoundsToPlay[Rand]);
	}

	int32 Index = MyInventoryUI->GetIndexOfItemInArrayByName(MyInventoryUI->CurrentItemEquipped);
	MyInventoryUI->ButtonsSelected[Index] = false;

	MyInventoryUI->CurrentItemEquipped = "";
}

void APointAndClickPlayerController::MainInteraction()
{
	if (bTravellingToInteractiveObject)
	{
		if (FVector::Dist(GetCharacter()->GetActorLocation(), InteractiveObjectLocation) < InteractionRange)
		{
			bTravellingToInteractiveObject = false;
			InteractiveObjectLocation = FVector(0);
			GetWorld()->GetNavigationSystem()->SimpleMoveToLocation(this, GetCharacter()->GetActorLocation());

			if (LastInteractedActor)
			{
				if (bLastInteractionWas_Interact)
				{
					if (LastInteractedActor->bIsHuman)
					{
						if (MyInventoryUI->CurrentItemEquipped != "")
						{
							Special_Item_Checker(MyInventoryUI->CurrentItemEquipped);
						}
						else
						{
							if (!LastInteractedActor->RequiresSpecialItemToBegin)
							{
								bIsInDialogueMode = true;
								DialogueSystem->LastInteractedActor = LastInteractedActor;

								GetCharacter()->SetActorLocation(FVector(Cast<AInteractiveObject>(DialogueSystem->LastInteractedActor)->CharacterPositionNode.X,
									Cast<AInteractiveObject>(DialogueSystem->LastInteractedActor)->CharacterPositionNode.Y,
									GetCharacter()->GetActorLocation().Z));

								GetCharacter()->SetActorRotation(
									FRotator(GetCharacter()->GetActorRotation().Pitch,
										FindLookAtRotation(GetCharacter()->GetActorLocation(), LastInteractedActor->GetActorLocation()).Yaw,
										GetCharacter()->GetActorRotation().Roll)
									);

								LastInteractedActor->InitiateDialogue();
							}
							else
							{
								GetCharacter()->SetActorRotation(
									FRotator(GetCharacter()->GetActorRotation().Pitch,
										FindLookAtRotation(GetCharacter()->GetActorLocation(), LastInteractedActor->GetActorLocation()).Yaw,
										GetCharacter()->GetActorRotation().Roll));

								DialogueSystem->LastInteractedActor = LastInteractedActor;
								int32 ID = LastInteractedActor->InteractBranchID;
								for (size_t i = 0; i < LastInteractedActor->InteractBranches[ID].ThingsToSay.Num(); i++)
								{
									EnqueueTextForTypewriter(LastInteractedActor->InteractBranches[ID].ThingsToSay[i], LastInteractedActor->InteractBranches[ID].SoundsToPlay[i]);
								}
							}
						}
					}
					else
					{
						if (MyInventoryUI->CurrentItemEquipped != "")
						{
							Special_Item_Checker(MyInventoryUI->CurrentItemEquipped);
						}
						else
						{
							if (!LastInteractedActor->RequiresSpecialItemToBegin)
							{
								GetCharacter()->SetActorRotation(
									FRotator(GetCharacter()->GetActorRotation().Pitch,
										FindLookAtRotation(GetCharacter()->GetActorLocation(), LastInteractedActor->GetActorLocation()).Yaw,
										GetCharacter()->GetActorRotation().Roll));

								DialogueSystem->LastInteractedActor = LastInteractedActor;
								int32 ID = LastInteractedActor->InteractBranchID;
								for (size_t i = 0; i < LastInteractedActor->InteractBranches[ID].ThingsToSay.Num(); i++)
								{
									EnqueueTextForTypewriter(LastInteractedActor->InteractBranches[ID].ThingsToSay[i], LastInteractedActor->InteractBranches[ID].SoundsToPlay[i]);
								}
							}
						}
					}
				}
				else if (bLastInteractionWas_Inspect)
				{
					GetCharacter()->SetActorRotation(
						FRotator(GetCharacter()->GetActorRotation().Pitch,
							FindLookAtRotation(GetCharacter()->GetActorLocation(), LastInteractedActor->GetActorLocation()).Yaw,
							GetCharacter()->GetActorRotation().Roll));

					DialogueSystem->LastInteractedActor = LastInteractedActor;
					int32 ID = LastInteractedActor->InspectBranchID;
					for (size_t i = 0; i < LastInteractedActor->InspectBranches[ID].ThingsToSay.Num(); i++)
					{
						EnqueueTextForTypewriter(LastInteractedActor->InspectBranches[ID].ThingsToSay[i], LastInteractedActor->InspectBranches[ID].SoundsToPlay[i]);
					}
				}
			}
		}
	}
}

void APointAndClickPlayerController::CharacterAndAnimationPointers()
{
	float Length = 0;
	FVector Dir;
	if (GetCharacter() && MyAnimInstance)
	{
		GetCharacter()->GetVelocity().ToDirectionAndLength(Dir, Length);
		MyAnimInstance->bIsWalking = (Length > 0) ? true : false;

		VisualHelper->SetWorldLocation(LastClickedLocation + FVector(0, 0, 20));
		VisualHelper->SetVisibility((MyAnimInstance->bIsWalking));
	}
}

void APointAndClickPlayerController::CursorFunctionality()
{
	CurrentObjectName = "";
	FHitResult HitResult;

	if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
	{
		CursorMan->SetActorLocation(HitResult.Location);
		AActor* LeActor = HitResult.Actor.Get();

		if (Cast<AInteractiveObject>(LeActor) && !bIsInDialogueMode)
		{
			if (TooltipUI)
			{
				if (MyCursorTextBlockControl != nullptr)
				{
					MyCursorTextBlockControl->SetVisibility(ESlateVisibility::Visible);
					MyCursorTextBlockControl->SetText(FText::FromString(Cast<AInteractiveObject>(LeActor)->ObjectName));
					CurrentObjectName = Cast<AInteractiveObject>(LeActor)->ObjectName;
				}
			}
		}
		else
		{
			if (TooltipUI)
			{
				if (MyCursorTextBlockControl != nullptr)
				{
					CurrentObjectName = "";
					MyCursorTextBlockControl->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
}

#pragma endregion
