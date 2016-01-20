
#include "MonsterCombat.h"
#include "CombatController.h"
#include "AIController.h"

ACombatController::ACombatController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShouldPerformFullTickWhenPaused = false;
}

void ACombatController::BeginPlay()
{

	Super::BeginPlay();

	if (UI_Holder)
	{
		MyActionHUD = Cast<UActionHUD>(CreateWidget<UUserWidget>(this, UI_Holder));
		MyActionHUD->AddToViewport();

		MyActionHUD->SetVisibility(ESlateVisibility::Collapsed);
		bShowMouseCursor = true;

		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(MyActionHUD->GetCachedWidget());
		SetInputMode(Mode);
	}

	MyActionHUD->MonsterNames.AddDefaulted(6);
	MyActionHUD->MonsterHPs_Max.AddDefaulted(6);
	MyActionHUD->MonsterHPs_Current.AddDefaulted(6);
	MyActionHUD->MonsterMPs_Current.AddDefaulted(6);
	MyActionHUD->MonsterMPs_Max.AddDefaulted(6);
	MyActionHUD->MonsterPortraits.AddDefaulted(6);
	
	MyActionHUD->AbilityMPCosts.AddDefaulted(3);
	MyActionHUD->AbilityNames.AddDefaulted(3);
	MyActionHUD->AbilityTargets.AddDefaulted(3);
	MyActionHUD->AbilityTooltips.AddDefaulted(3);

	MyActionHUD->ItemNames.AddDefaulted(4);
	MyActionHUD->ItemQuantities.AddDefaulted(4);
	MyActionHUD->ItemTargets.AddDefaulted(4);
	MyActionHUD->ItemTooltips.AddDefaulted(4);

	IndexOfCurrentPlayingMonster = -1;
	ReferenceCameras();
	DetermineParties();
	DetermineTurnOrder();
	InitializeInventoryStock();

	// Wiring up the delegate that will take care of destroying an audio comp when a sound has finished playing.
	AudioFinishDelegate.BindUFunction(this, "DestroyFinishedAudio");

	EndTurn();
}

void ACombatController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MyActionHUD)return; // This indicates that the game has not started yet, and keeps Controller out of looping while in Editor.

	if (bIsOverheadCamera)
	{
		CameraOverhead->SetActorRotation(FindLookAtRotation(CameraOverhead->GetActorLocation(), TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation()));
	}


	switch (CurrentPhase)
	{
	case EActionPhasesType::Undefined:
		break;

#pragma region Attack Mechanic

	case EActionPhasesType::Attack:

		
		if (CurrentAttackPhase == EActionPhasesAttack::TargetSelection)
		{
			MyActionHUD->CurrentMonsterID = -1;
			FHitResult HitResult;
			if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
			{
				AActor* LeActor = HitResult.Actor.Get();

				if (Cast<ACustomMonsterActor>(LeActor))
				{
					MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;
				}
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::CameraOnActor)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				InitialPosition = Cast<AActor>(TurnOrder[IndexOfCurrentPlayingMonster])->GetActorLocation();
				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
				CurrentAttackPhase = EActionPhasesAttack::MovingTowardsEnemy;
				ChangeCamera(ECameras::FreeRoamCamera);
				MyActionHUD->bIsFadingOut = true;
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::MovingTowardsEnemy)
		{
			FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
			if (FVector::Dist(CurPos, ReceiverPos) < 300)
			{
				CurrentAttackPhase = EActionPhasesAttack::RestAfterMoveFinished;
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID),CurPos);
				FindLookAtRotation(CurPos, GetEnemyPosByID(MyActionHUD->CurrentMonsterID));
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::RestAfterMoveFinished)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentAttackPhase = EActionPhasesAttack::MeleeAttack;

				//Melee Attack
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->TargetActor = GetEnemyActorByID(MyActionHUD->CurrentMonsterID);
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->AttackAnim();
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::MeleeAttack)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentAttackPhase = EActionPhasesAttack::RestAfterAttack;
				
			}
		
		}
		if (CurrentAttackPhase == EActionPhasesAttack::RestAfterAttack)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				
				FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
				
				UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
				ReceiverPos = TurnOrder[IndexOfCurrentPlayingMonster]->InitPos;
				NavSys->SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;
				CurrentAttackPhase = EActionPhasesAttack::MoveTowardsInitialPosition;
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::MoveTowardsInitialPosition)
		{
			FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
			if (FVector2D::Distance(FVector2D(CurPos.X,CurPos.Y), FVector2D(ReceiverPos.X, ReceiverPos.Y)) < 50)
			{
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);

				switch (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID)
				{
					case 0:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(5)));
						break;
					case 1:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(4)));
						break;
					case 2:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(3)));
						break;
					case 3:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(2)));
						break;
					case 4:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(1)));
						break;
					case 5:
						TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(0)));
						break;
				}
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				CurrentAttackPhase = EActionPhasesAttack::ResetTurn;
				
			}
		}
		if (CurrentAttackPhase == EActionPhasesAttack::ResetTurn)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentPhase = EActionPhasesType::Undefined;
				CurrentAttackPhase = EActionPhasesAttack::TargetSelection;
				EndTurn();
			}
		}

		break;

#pragma endregion

#pragma region Ability Mechanic
	case EActionPhasesType::Ability:
		if (CurrentAbilityPhase == EActionPhasesAbility::TargetSelectionAbility)
		{
			if (MyActionHUD->CurrentTarget == "Single Ally" || MyActionHUD->CurrentTarget == "Single Enemy")
			{
				MyActionHUD->CurrentMonsterID = -1;
				FHitResult HitResult;
				if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
				{
					AActor* LeActor = HitResult.Actor.Get();

					if (Cast<ACustomMonsterActor>(LeActor))
					{
						MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;
					}
				}
			}
		}
		if (CurrentAbilityPhase == EActionPhasesAbility::CameraOnActorAbility)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;

				// Cast Spell, Play Animation
				GLog->Log(FString::FromInt(MyActionHUD->CurrentMonsterID));
				PlaySound(MyActionHUD->LatestAbility.SpellShout);
				MyActionHUD->bIsFadingOut = true;
				CurrentAbilityPhase = EActionPhasesAbility::ActorCastingSpell;
			}
		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::ActorCastingSpell)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentAbilityPhase = EActionPhasesAbility::ActorSpellCastRest;
			}
		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::ActorSpellCastRest)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				if (MyActionHUD->CurrentTarget == "All Allies")
				{
					ChangeCamera(ECameras::Camera_Party_A);
					CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				}
				else if (MyActionHUD->CurrentTarget == "All Enemies")
				{
					ChangeCamera(ECameras::Camera_Party_B);
					CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				}
				else if (MyActionHUD->CurrentTarget == "Single Enemy")
				{
					ChangeCamera(MyActionHUD->CurrentMonsterID); 

					CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				}
				else if (MyActionHUD->CurrentTarget == "Single Ally")
				{
					ChangeCamera(MyActionHUD->CurrentMonsterID); 

					CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				}
				else if (MyActionHUD->CurrentTarget == "Self")
				{
					ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);

					CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				}

			}
		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::TargetActorCameraRest)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
				// SPELL EFFECT ON DUDE + SPELL ANIMATION
			}
		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::TargetActorCameraEffect)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				CurrentAbilityPhase = EActionPhasesAbility::ResetTurnAbility;

			}
		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::SpellFinalizing)
		{

		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::SpellFinalized)
		{

		}
		else if (CurrentAbilityPhase == EActionPhasesAbility::ResetTurnAbility)
		{
			CurrentPhase = EActionPhasesType::Undefined;
			CurrentAbilityPhase = EActionPhasesAbility::TargetSelectionAbility;
			EndTurn();
		}

		break;

#pragma endregion

#pragma region Item Mechanic

	case EActionPhasesType::Item:

		if (CurrentItemPhase == EActionPhasesItem::TargetSelection_Friendly)
		{
			if (MyActionHUD->CurrentTarget == "Single")
			{
				MyActionHUD->CurrentMonsterID = -1;
				FHitResult HitResult;
				if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
				{
					AActor* LeActor = HitResult.Actor.Get();

					if (Cast<ACustomMonsterActor>(LeActor))
					{
						MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;
					}
				}
			}
			else
			{
				
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::CameraOnActor_Item)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				InitialPosition = Cast<AActor>(TurnOrder[IndexOfCurrentPlayingMonster])->GetActorLocation();

				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
				CurrentItemPhase = EActionPhasesItem::MovingTowardsFriend;
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;
				ChangeCamera(ECameras::FreeRoamCamera);
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::MovingTowardsFriend)
		{
			FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
			if (FVector::Dist(CurPos, ReceiverPos) < 250)
			{
				// TODO: APPLY ITEM STUFF

				CurrentItemPhase = EActionPhasesItem::AppliedItemRest;
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), CurPos);
				FindLookAtRotation(CurPos, GetEnemyPosByID(MyActionHUD->CurrentMonsterID));
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::AppliedItemRest)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentItemPhase = EActionPhasesItem::MoveTowardsInitialPosition_Item;
				MyActionHUD->bIsFadingOut = true;
				UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
				ReceiverPos = TurnOrder[IndexOfCurrentPlayingMonster]->InitPos;
				NavSys->SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;

			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::MoveTowardsInitialPosition_Item)
		{
			FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
			if (FVector2D::Distance(FVector2D(CurPos.X, CurPos.Y), FVector2D(ReceiverPos.X, ReceiverPos.Y)) < 50)
			{
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
				UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);

				switch (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID)
				{
				case 0:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(5)));
					break;
				case 1:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(4)));
					break;
				case 2:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(3)));
					break;
				case 3:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(2)));
					break;
				case 4:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(1)));
					break;
				case 5:
					TurnOrder[IndexOfCurrentPlayingMonster]->SetActorRotation(FindLookAtRotation(CurPos, GetEnemyPosByID(0)));
					break;
				}
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				CurrentItemPhase = EActionPhasesItem::ResetTurnItem;
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::ResetTurnItem)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentPhase = EActionPhasesType::Undefined;
				CurrentItemPhase = EActionPhasesItem::TargetSelection_Friendly;
				EndTurn();
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::GlobalItemUsageWait)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;

				// USE ITEM

				CurrentItemPhase = EActionPhasesItem::GlobalItemUsedWait;
			}
		}
		else if (CurrentItemPhase == EActionPhasesItem::GlobalItemUsedWait)
		{
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				CurrentItemPhase = EActionPhasesItem::ResetTurnItem;
			}
		}

		break;

#pragma endregion

#pragma region Defense Mechanic

	case EActionPhasesType::Defend:

		switch (CurrentDefendPhase)
		{
		case EActionPhasesDefend::DefendingParticle:
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				MyActionHUD->bIsFadingOut = true;
				HelperPhaseTimer = 0;
				CurrentDefendPhase = EActionPhasesDefend::ResetTurnDef;
			}
			break;
		case EActionPhasesDefend::ResetTurnDef:
			HelperPhaseTimer += DeltaTime;
			if (HelperPhaseTimer > GlobalWaitingTimer)
			{
				HelperPhaseTimer = 0;
				CurrentPhase = EActionPhasesType::Undefined;
				CurrentDefendPhase = EActionPhasesDefend::DefendingParticle;
				EndTurn();
			}
			break;
		}

#pragma endregion

		break;


	}
}

void ACombatController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("RMB", IE_Pressed, this, &ACombatController::RMB);
	InputComponent->BindAction("LMB", IE_Pressed, this, &ACombatController::LMB);
}

void ACombatController::LMB()
{
	switch (CurrentPhase)
	{
	case EActionPhasesType::Undefined:
		break;
	case EActionPhasesType::Attack:

		switch (CurrentAttackPhase)
		{
		case EActionPhasesAttack::TargetSelection:

			if (MyActionHUD->CurrentMonsterID != -1)
			{
				MyActionHUD->DepthLevel = 3;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);
				MyActionHUD->bIsFadingOut = false;
				MyActionHUD->CentralTooltip = "Attack";
				CurrentAttackPhase = EActionPhasesAttack::CameraOnActor;
			}

			break;
		case EActionPhasesAttack::CameraOnActor:



			break;
		case EActionPhasesAttack::MovingTowardsEnemy:

			

			break;
		case EActionPhasesAttack::RestAfterMoveFinished:
			break;
		case EActionPhasesAttack::MeleeAttack:
			break;
		case EActionPhasesAttack::RestAfterAttack:
			break;
		case EActionPhasesAttack::MoveTowardsInitialPosition:
			break;
		case EActionPhasesAttack::ResetTurn:
			break;
		}
		break;
	case EActionPhasesType::Ability:
		if (CurrentAbilityPhase == EActionPhasesAbility::TargetSelectionAbility)
		{
			if (MyActionHUD->CurrentMonsterID != -1 && (MyActionHUD->CurrentTarget == "Single Ally" || MyActionHUD->CurrentTarget == "Single Enemy"))
			{
				MyActionHUD->DepthLevel = 3;
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);


				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			}
			else if (MyActionHUD->CurrentTarget == "All Enemies" || MyActionHUD->CurrentTarget == "All Allies")
			{
				MyActionHUD->DepthLevel = 3;
				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
				
				MyActionHUD->CurrentMenu = EActiveMenus::None;

				MyActionHUD->bIsCustomTooltip = false;
				MyActionHUD->BottomTooltip = "";

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
			}
			else if (MyActionHUD->CurrentTarget == "Self")
			{
				MyActionHUD->DepthLevel = 3;
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);

				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			}
			MyActionHUD->bIsFadingOut = false;
			MyActionHUD->CentralTooltip = MyActionHUD->LatestAbility.Name;
		}
		break;
	case EActionPhasesType::Item:

		if (CurrentItemPhase == EActionPhasesItem::TargetSelection_Friendly)
		{
			if (MyActionHUD->CurrentMonsterID != -1)
			{
				MyActionHUD->DepthLevel = 3;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);

				CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
			}
			else if (MyActionHUD->CurrentTarget == "All")
			{
				MyActionHUD->DepthLevel = 3;
				ChangeCamera(ECameras::Camera_Party_A);
				MyActionHUD->CurrentMenu = EActiveMenus::None;

				MyActionHUD->bIsCustomTooltip = false;
				MyActionHUD->BottomTooltip = "";

				CurrentItemPhase = EActionPhasesItem::GlobalItemUsageWait;
			}
			MyActionHUD->bIsFadingOut = false;
			MyActionHUD->CentralTooltip = MyActionHUD->LatestItem.Name;
		}

		break;
	case EActionPhasesType::Defend:

		switch (CurrentDefendPhase)
		{
		case EActionPhasesDefend::DefendingParticle:
			
			break;
		case EActionPhasesDefend::ResetTurnDef:
			break;
		}

		break;
	}
}

void ACombatController::RMB()
{
	if(MyActionHUD->DepthLevel>0)
	MyActionHUD->Cancel();
}

// ******************

void ACombatController::ReferenceCameras()
{
	MonsterStaticCams.AddUninitialized(6);

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->ActorHasTag("CameraPartyA"))
		{
			CameraPartyA = *ActorItr;
		}
		if (ActorItr->ActorHasTag("CameraPartyB"))
		{
			CameraPartyB = *ActorItr;
		}
		if (ActorItr->ActorHasTag("CameraOverhead"))
		{
			CameraOverhead = *ActorItr;
		}

		if (ActorItr->ActorHasTag("MonsterCam1"))
		{
			MonsterStaticCams[0] = *ActorItr;
		}
		if (ActorItr->ActorHasTag("MonsterCam2"))
		{
			MonsterStaticCams[1] = *ActorItr;
		}
		if (ActorItr->ActorHasTag("MonsterCam3"))
		{
			MonsterStaticCams[2] = *ActorItr;
		}
		if (ActorItr->ActorHasTag("MonsterCam4"))
		{
			MonsterStaticCams[3] = *ActorItr;
		}
		if (ActorItr->ActorHasTag("MonsterCam5"))
		{
			MonsterStaticCams[4] = *ActorItr;
		}
		if (ActorItr->ActorHasTag("MonsterCam6"))
		{
			MonsterStaticCams[5] = *ActorItr;
		}
	}
}

void ACombatController::ChangeCamera(ECameras CamType)
{
	bIsOverheadCamera = false;

	switch (CamType)
	{
	case Camera_Party_A:
		SetViewTarget(CameraPartyA);
		break;
	case Camera_Party_B:
		SetViewTarget(CameraPartyB);
		break;
	case MonsterCamA1:
		break;
	case MonsterCamA2:
		break;
	case MonsterCamA3:
		break;
	case MonsterCamB1:
		break;
	case MonsterCamB2:
		break;
	case MonsterCamB3:
		break;
	case FreeRoamCamera:
		bIsOverheadCamera = true;
		SetViewTarget(CameraOverhead);
		break;
	case NoneCam:
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);
		break;
	default:
		break;
	}
}

void ACombatController::ChangeCamera(int32 MonsterID)
{
	bIsOverheadCamera = false;
	SetViewTarget(MonsterStaticCams[MonsterID]);
}

void ACombatController::DetermineParties()
{
	for (TActorIterator<ACustomMonsterActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->Party == EPartyType::PlayerControlled)
		{
			PartyA.Add(*ActorItr);
		}
		if (ActorItr->Party == EPartyType::AI_Controlled)
		{
			PartyB.Add(*ActorItr);
		}
	}
}

// ******************

void ACombatController::DetermineTurnOrder()
{
	TArray<ACustomMonsterActor*> Monsters;

	for (size_t i = 0; i < 3; i++)
	{
		Monsters.Add(PartyA[i]);
		Monsters.Add(PartyB[i]);
	}

	Monsters.Sort([](const ACustomMonsterActor& A, const ACustomMonsterActor& B) {
		return A.Speed > B.Speed;
	});

	for (size_t i = 0; i < 6; i++)
	{
		TurnOrder.Add(Monsters[i]);
	}
}

void ACombatController::EndTurn()
{
	if(IndexOfCurrentPlayingMonster < 6)
		IndexOfCurrentPlayingMonster++;
	else
	{
		IndexOfCurrentPlayingMonster = 0;
		GLog->Log("Index: " + FString::FromInt(IndexOfCurrentPlayingMonster));
	}

	if (TurnOrder[IndexOfCurrentPlayingMonster]->Party == EPartyType::PlayerControlled)
	{
		TurnOrder[IndexOfCurrentPlayingMonster]->MonsterCameraRoot->SetRelativeRotation(TurnOrder[IndexOfCurrentPlayingMonster]->InitialCameraRot);
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);
		CurrentPhase = EActionPhasesType::Undefined;

		MyActionHUD->CurrentMenu = EActiveMenus::MainMenu;
		MyActionHUD->SetVisibility(ESlateVisibility::Visible);
		MyActionHUD->DepthLevel = 0;
	}
	else
	{

	}
}


// ******************

FVector ACombatController::GetEnemyMeleeReceiverPosByID(int32 ID)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (TurnOrder[i]->MonsterID == ID)
		{
			return TurnOrder[i]->GetActorLocation() - TurnOrder[i]->GetActorLocation().ForwardVector * 150;
		}
	}

	return FVector(0);
}

FVector ACombatController::GetEnemyPosByID(int32 ID)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (TurnOrder[i]->MonsterID == ID)
		{
			return TurnOrder[i]->GetActorLocation();
		}
	}

	return FVector(0);
}

AController* ACombatController::GetEnemyControllerByID(int32 ID)
{

	for (size_t i = 0; i < 6; i++)
	{
		if (TurnOrder[i]->MonsterID == ID)
		{
			return TurnOrder[i]->GetController();
		}
	}

	return nullptr;
}

AActor* ACombatController::GetEnemyActorByID(int32 ID)
{

	for (size_t i = 0; i < 6; i++)
	{
		GLog->Log(FString::FromInt(TurnOrder[i]->MonsterID));
		if (TurnOrder[i]->MonsterID == ID)
		{
			return Cast<AActor>(TurnOrder[i]);
		}
	}

	return nullptr;
}

FRotator ACombatController::FindLookAtRotation(FVector Start, FVector Target)
{
	FVector Direction = Target - Start;
	FRotator FinalRotation = Direction.Rotation();

	return FinalRotation;
}

// *******************

void ACombatController::ApplyDamage(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim)
{
	if (Victim)
	{
		Victim->CurrentHealth -= 5;
		Victim->AnimInstance->GetHurt();

		if (Victim->CurrentHealth <= 0)
		{
			Victim->MonsterDeath();
		}

		MyActionHUD->MonsterHPs_Current[Victim->MonsterID] = Victim->CurrentHealth;
	}
}

// *******************

void ACombatController::InitializeInventoryStock()
{
	GroupAInventory = GroupInventories;
	GroupBInventory = GroupInventories;

	MyActionHUD->GroupAInventory = GroupAInventory;
	MyActionHUD->GroupBInventory = GroupBInventory;
}

// *******************

void ACombatController::PlaySound(USoundWave* SoundEffect)
{
	UAudioComponent* Audio = NewObject<UAudioComponent>(this);
	Audio->RegisterComponent();

	Audio->OnAudioFinished.Add(AudioFinishDelegate);

	Audio->Sound = SoundEffect;
	Audio->Play();

	SoundEffects.Add(Audio);
}

void  ACombatController::DestroyFinishedAudio()
{
	bool WillRerun = false;
	for (size_t i = 0; i < SoundEffects.Num(); i++)
	{
		if (!SoundEffects[i]->IsPlaying())
		{
			SoundEffects[i]->DestroyComponent();
			SoundEffects.RemoveAt(i);
			WillRerun = true;
			break;
		}
	}

	if (WillRerun)DestroyFinishedAudio();
}