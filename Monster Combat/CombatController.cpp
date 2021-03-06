
////////////////////////////////////////////////////////////////////////////////////////////
//				Turn Based System (Including Combat, Cameras, Input and Phases)			  //
////////////////////////////////////////////////////////////////////////////////////////////

#include "MonsterCombat.h"
#include "CombatController.h"
#include "AIController.h"
#include "CombatMeters.h"
#include "MonsterAIController.h"

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

	for (size_t i = 0; i < Sounds_TXT.Num(); i++)
	{
		SFX_Dictionary.Add(Sounds_TXT[i], Sounds_SFX[i]);
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
}

void ACombatController::InitializerGrande()
{
	IndexOfCurrentPlayingMonster = -1;
	
	ReferenceCameras();
	DetermineParties();
	DetermineTurnOrder();
	InitializeInventoryStock();

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->ActorHasTag("Pooler"))
		{
			GlobalPool = Cast<AObjectPooler>(*ActorItr);
		}
	}

	EndTurn();
}

void ACombatController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInitialized)
	{
		int32 CoolCount = 0;
		for (TActorIterator<ACustomMonsterActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->AllDone)
			{
				CoolCount++;
			}
		}
		if (CoolCount == 6)
		{
			bIsInitialized = true;
			InitializerGrande();
		}

		return;
	}

	if (!MyActionHUD) return; // This indicates that the game has not started yet, and keeps Controller out of looping while in Editor.

	if (WinningCondition != 0 && bWinCountdown)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 3)
		{
			if (WinningCondition == 1)
				SetBottomTooltip(true, "VICTORY!");
			else
			{
				SetBottomTooltip(true, "DEFEAT!");
			}
		}
	}

	if (bIsAI_PseudoThink)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 2)
		{
			bIsAI_PseudoThink = false;
			HelperPhaseTimer = 0;
			AI_ExecuteAction();
		}
	}


	if (bWillChangeAfterDeath)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 2)
		{
			bWillChangeAfterDeath = false;
			HelperPhaseTimer = 0;
			EndTurn();
		}
	}

	if (bIsOverheadCamera)
		CameraOverhead->SetActorRotation(FindLookAtRotation(CameraOverhead->GetActorLocation(), TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation()));
	


	switch (CurrentPhase)
	{
	case EActionPhasesType::Attack:
		Attacking(DeltaTime);
		break;

	case EActionPhasesType::Ability:
		UsingAbilities(DeltaTime);
		break;

	case EActionPhasesType::Item:
		UsingItems(DeltaTime);
		break;

	case EActionPhasesType::Defend:
		Defending(DeltaTime);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//								Tick Helper Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////

void ACombatController::Attacking(float DeltaTime)
{
	if (CurrentAttackPhase == EActionPhasesAttack::TargetSelection)
	{
		MyActionHUD->CurrentMonsterID = -1;
		FHitResult HitResult;

		for (size_t i = 0; i < 6; i++)
		{
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
		}
		
		if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
		{
			AActor* LeActor = HitResult.Actor.Get();

			if (Cast<ACustomMonsterActor>(LeActor))
			{
				if (!Cast<ACustomMonsterActor>(LeActor)->bIsDead)
					MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;

				if (MyActionHUD->CurrentMonsterID != -1)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->bRenderCustomDepth = true;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->MarkRenderStateDirty();
				}
			}
		}
	}
	if (CurrentAttackPhase == EActionPhasesAttack::CameraOnActor)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 2)
		{
			HelperPhaseTimer = 0;
			InitialPosition = Cast<AActor>(TurnOrder[IndexOfCurrentPlayingMonster])->GetActorLocation();
			UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), FVector(ReceiverPos.X, ReceiverPos.Y, -770.0f));
			CurrentAttackPhase = EActionPhasesAttack::MovingTowardsEnemy;
			ChangeCamera(ECameras::FreeRoamCamera);
			MyActionHUD->bIsFadingOut = true;
			TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;
			bHasFinishedPath = false;
			SetBottomTooltip(true, TurnOrder[IndexOfCurrentPlayingMonster]->MonsterName + " is attacking " + MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID] + "!");
		}
	}
	if (CurrentAttackPhase == EActionPhasesAttack::MovingTowardsEnemy)
	{
		FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
		if (bHasFinishedPath)
		{
			CurrentAttackPhase = EActionPhasesAttack::RestAfterMoveFinished;
			TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
			UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), CurPos);
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
			bHasFinishedPath = false;
		}
	}
	if (CurrentAttackPhase == EActionPhasesAttack::MoveTowardsInitialPosition)
	{
		FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
		if (bHasFinishedPath)
		{
			TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
			

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
		if (HelperPhaseTimer > 3)
		{
			SetBottomTooltip(false, "");
			HelperPhaseTimer = 0;
			CurrentPhase = EActionPhasesType::Undefined;
			CurrentAttackPhase = EActionPhasesAttack::TargetSelection;
			EndTurn();
		}
	}
}

void ACombatController::UsingItems(float DeltaTime)
{
	if (CurrentItemPhase == EActionPhasesItem::TargetSelection_Friendly)
	{
		for (size_t i = 0; i < 6; i++)
		{
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
		}

		if (MyActionHUD->CurrentTarget == "Single")
		{
			MyActionHUD->CurrentMonsterID = -1;
			FHitResult HitResult;

			if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
			{
				AActor* LeActor = HitResult.Actor.Get();

				if (Cast<ACustomMonsterActor>(LeActor))
				{
					if((!Cast<ACustomMonsterActor>(LeActor)->bIsDead && !MyActionHUD->LatestItem.Resurrection) || (Cast<ACustomMonsterActor>(LeActor)->bIsDead && MyActionHUD->LatestItem.Resurrection))
					MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;

					if (MyActionHUD->CurrentMenu != EActiveMenus::AbilitiesMenu && MyActionHUD->CurrentMenu != EActiveMenus::ItemsMenu && MyActionHUD->CurrentMonsterID!=-1)
					{
						Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->bRenderCustomDepth = true;
						Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->MarkRenderStateDirty();
						
					}
				}
			}
		}
		else if (MyActionHUD->CurrentTarget == "All")
		{
			for (size_t i = 0; i < 3; i++)
			{
				if (!Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->bIsDead)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = true;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}

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

			FString TargetString = (MyActionHUD->LatestItem.Target.Contains("Single")) ? MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID] : "the whole party!";
			SetBottomTooltip(true, TurnOrder[IndexOfCurrentPlayingMonster]->MonsterName + " is using " + MyActionHUD->LatestItem.Name + " on " + TargetString);
		}
	}
	else if (CurrentItemPhase == EActionPhasesItem::MovingTowardsFriend)
	{
		FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
		if (FVector::Dist(CurPos, ReceiverPos) < 350)
		{
			
			if (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID < 3)
			{
				for (size_t i = 0; i < GroupAInventory.Items.Num(); i++)
				{
					if (GroupAInventory.Items[i].Name == MyActionHUD->LatestItem.Name)
					{
						GroupAInventory.Items[i].Quantity--;
					}
				}
			}
			else
			{
				for (size_t i = 0; i < GroupBInventory.Items.Num(); i++)
				{
					if (GroupBInventory.Items[i].Name == MyActionHUD->LatestItem.Name)
					{
						GroupBInventory.Items[i].Quantity--;
					}
				}
			}

			GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ItemUsed));
			ApplyItem(MyActionHUD->LatestItem,Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID)));

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
			if (MyActionHUD->CurrentMonsterID != TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID)
			{
				NavSys->SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
				bHasFinishedPath = false;
				TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = true;
			}
			else
			{
				bHasFinishedPath = true;
			}

		}
	}
	else if (CurrentItemPhase == EActionPhasesItem::MoveTowardsInitialPosition_Item)
	{
		FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
		if (bHasFinishedPath)
		{
			TurnOrder[IndexOfCurrentPlayingMonster]->AnimInstance->bIsMoving = false;
			

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
			SetBottomTooltip(false, "");
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

			MyActionHUD->CurrentMonsterID = -1;
			MyActionHUD->bIsFadingOut = true;
			// USE ITEM
			GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ItemUsed));
			if (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID < 3)
			{
				for (size_t i = 0; i < GroupAInventory.Items.Num(); i++)
				{
					if (GroupAInventory.Items[i].Name == MyActionHUD->LatestItem.Name)
					{
						GroupAInventory.Items[i].Quantity--;
					}
				}
			}
			else
			{
				for (size_t i = 0; i < GroupBInventory.Items.Num(); i++)
				{
					if (GroupBInventory.Items[i].Name == MyActionHUD->LatestItem.Name)
					{
						GroupBInventory.Items[i].Quantity--;
					}
				}
			}

			if (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID < 3)
			{
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(0)));
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(1)));
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(2)));
			}
			else
			{
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(3)));
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(4)));
				ApplyItem(MyActionHUD->LatestItem, Cast<ACustomMonsterActor>(GetEnemyActorByID(5)));
			}

			CurrentItemPhase = EActionPhasesItem::GlobalItemUsedWait;
		}
	}
	else if (CurrentItemPhase == EActionPhasesItem::GlobalItemUsedWait)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 3)
		{
			HelperPhaseTimer = 0;
			ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
			CurrentItemPhase = EActionPhasesItem::ResetTurnItem;
		}
	}
}

void ACombatController::UsingAbilities(float DeltaTime)
{
	if (CurrentAbilityPhase == EActionPhasesAbility::TargetSelectionAbility)
	{
		for (size_t i = 0; i < 6; i++)
		{
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
		}

		if (MyActionHUD->CurrentTarget == "Single Ally" || MyActionHUD->CurrentTarget == "Single Enemy")
		{
			MyActionHUD->CurrentMonsterID = -1;
			FHitResult HitResult;

			if (GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, HitResult))
			{
				AActor* LeActor = HitResult.Actor.Get();

				if (Cast<ACustomMonsterActor>(LeActor))
				{
					if (!Cast<ACustomMonsterActor>(LeActor)->bIsDead)
					MyActionHUD->CurrentMonsterID = Cast<ACustomMonsterActor>(LeActor)->MonsterID;

					if (MyActionHUD->CurrentMenu != EActiveMenus::AbilitiesMenu && MyActionHUD->CurrentMenu != EActiveMenus::ItemsMenu && MyActionHUD->CurrentMonsterID != -1)
					{
						Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->bRenderCustomDepth = true;
						Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID))->GetMesh()->MarkRenderStateDirty();
					}
				}
			}
		}
		else if (MyActionHUD->CurrentTarget == "All Allies")
		{
			for (size_t i = 0; i < 3; i++)
			{
				if (!Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->bIsDead)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = true;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
		}
		else if (MyActionHUD->CurrentTarget == "All Enemies")
		{
			for (size_t i = 3; i < 6; i++)
			{
				if (!Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->bIsDead)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = true;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
		}
		else if (MyActionHUD->CurrentTarget == "Self")
		{
			
			Cast<ACustomMonsterActor>(GetEnemyActorByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID))->GetMesh()->bRenderCustomDepth = true;
			Cast<ACustomMonsterActor>(GetEnemyActorByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID))->GetMesh()->MarkRenderStateDirty();
		}
	}
	if (CurrentAbilityPhase == EActionPhasesAbility::CameraOnActorAbility)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > GlobalWaitingTimer)
		{
			HelperPhaseTimer = 0;

			// Cast Spell, Play Animation
			FString TargetString;

			if (MyActionHUD->LatestAbility.Target == "Single Ally" || MyActionHUD->LatestAbility.Target == "Single Enemy")
			{
				TargetString = MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID];
			}
			else if (MyActionHUD->LatestAbility.Target == "Self")
			{
				TargetString = "himself";
			}
			else if (MyActionHUD->LatestAbility.Target == "All Enemies")
			{
				TargetString = "all the enemies";
			}
			else if (MyActionHUD->LatestAbility.Target == "All Allies")
			{
				TargetString = "the whole party";
			}

			SetBottomTooltip(true, TurnOrder[IndexOfCurrentPlayingMonster]->MonsterName + " is using " + MyActionHUD->LatestAbility.Name + " on " + TargetString + "!");
			
			GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellShout);
			GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellStartSFX);
			GlobalPool->SpawnParticleAtLocation(TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation(),MyActionHUD->LatestAbility.ParticleSystem_Start);

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
				ChangeCamera((TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID>2) ? ECameras::Camera_Party_B : ECameras::Camera_Party_A);
				CurrentAbilityPhase = EActionPhasesAbility::TargetActorCameraEffect;
			}
			else if (MyActionHUD->CurrentTarget == "All Enemies")
			{
				ChangeCamera((TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID>2) ? ECameras::Camera_Party_A : ECameras::Camera_Party_B);
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
	else if (CurrentAbilityPhase == EActionPhasesAbility::TargetActorCameraEffect)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > GlobalWaitingTimer)
		{
			HelperPhaseTimer = 0;
			CurrentAbilityPhase = EActionPhasesAbility::SpellFinalizing;

			TurnOrder[IndexOfCurrentPlayingMonster]->CurrentMana -= MyActionHUD->LatestAbility.ManaCost;
			MyActionHUD->MonsterMPs_Current[TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID] = TurnOrder[IndexOfCurrentPlayingMonster]->CurrentMana;

			int32 BonusIndex = 0;
			if (TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID > 2)
			{
				BonusIndex = 3;
			}

			if (MyActionHUD->CurrentTarget == "All Allies")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(0 + BonusIndex)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(1 + BonusIndex)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(2 + BonusIndex)), MyActionHUD->LatestAbility);
			}
			else if (MyActionHUD->CurrentTarget == "All Enemies")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(3 - BonusIndex)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(4 - BonusIndex)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(5 - BonusIndex)), MyActionHUD->LatestAbility);
			}
			else if (MyActionHUD->CurrentTarget == "Single Enemy")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID)), MyActionHUD->LatestAbility);
			}
			else if (MyActionHUD->CurrentTarget == "Single Ally")
			{
				
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(MyActionHUD->CurrentMonsterID)), MyActionHUD->LatestAbility);
			}
			else if (MyActionHUD->CurrentTarget == "Self")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], TurnOrder[IndexOfCurrentPlayingMonster], MyActionHUD->LatestAbility);
			}
		}
	}
	else if (CurrentAbilityPhase == EActionPhasesAbility::SpellFinalizing)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 2)
		{
			HelperPhaseTimer = 0;
			ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
			CurrentAbilityPhase = EActionPhasesAbility::ResetTurnAbility;

		}
	}
	else if (CurrentAbilityPhase == EActionPhasesAbility::SpellFinalized)
	{

	}
	else if (CurrentAbilityPhase == EActionPhasesAbility::ResetTurnAbility)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > 3)
		{
			SetBottomTooltip(false, "");
			CurrentPhase = EActionPhasesType::Undefined;
			CurrentAbilityPhase = EActionPhasesAbility::TargetSelectionAbility;
			EndTurn();
		}
	}
}

void ACombatController::Defending(float DeltaTime)
{
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
		if (HelperPhaseTimer > 2)
		{
			HelperPhaseTimer = 0;
			CurrentPhase = EActionPhasesType::Undefined;
			CurrentDefendPhase = EActionPhasesDefend::DefendingParticle;
			TurnOrder[IndexOfCurrentPlayingMonster]->bIsDefending = true;
			EndTurn();
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//								Input Related Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////

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
				HelperPhaseTimer = 0;
				MyActionHUD->DepthLevel = 3;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);
				MyActionHUD->bIsFadingOut = false;
				MyActionHUD->CentralTooltip = "Attack";
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}

				CurrentAttackPhase = EActionPhasesAttack::CameraOnActor;
			}

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
				HelperPhaseTimer = 0;
				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				MyActionHUD->bIsFadingOut = false;
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				MyActionHUD->CentralTooltip = MyActionHUD->LatestAbility.Name;
				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;

				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
			else if (MyActionHUD->CurrentTarget == "All Enemies" || MyActionHUD->CurrentTarget == "All Allies")
			{
				HelperPhaseTimer = 0;
				MyActionHUD->DepthLevel = 3;
				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
				
				MyActionHUD->CurrentMenu = EActiveMenus::None;

				MyActionHUD->bIsCustomTooltip = false;
				MyActionHUD->BottomTooltip = "";
				MyActionHUD->bIsFadingOut = false;
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				MyActionHUD->CentralTooltip = MyActionHUD->LatestAbility.Name;

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);

				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
			else if (MyActionHUD->CurrentTarget == "Self")
			{
				HelperPhaseTimer = 0;
				MyActionHUD->DepthLevel = 3;
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);

				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
				MyActionHUD->bIsFadingOut = false;
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				MyActionHUD->CentralTooltip = MyActionHUD->LatestAbility.Name;

				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
			
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

				MyActionHUD->bIsFadingOut = false;
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				MyActionHUD->CentralTooltip = MyActionHUD->LatestItem.Name;

				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
			else if (MyActionHUD->CurrentTarget == "All")
			{
				MyActionHUD->DepthLevel = 3;
				ChangeCamera(ECameras::Camera_Party_A);
				MyActionHUD->CurrentMenu = EActiveMenus::None;

				MyActionHUD->bIsCustomTooltip = false;
				MyActionHUD->BottomTooltip = "";

				FString TargetString = (MyActionHUD->LatestItem.Target.Contains("Single")) ? MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID] : "the whole party!";
				SetBottomTooltip(true, TurnOrder[IndexOfCurrentPlayingMonster]->MonsterName + " is using " + MyActionHUD->LatestItem.Name + " on " + TargetString);

				CurrentItemPhase = EActionPhasesItem::GlobalItemUsageWait;

				MyActionHUD->bIsFadingOut = false;
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
				MyActionHUD->CentralTooltip = MyActionHUD->LatestItem.Name;

				for (size_t i = 0; i < 6; i++)
				{
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
					Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
				}
			}
			
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
	if (MyActionHUD->DepthLevel > 0)
	{
		for (size_t i = 0; i < 6; i++)
		{
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->bRenderCustomDepth = false;
			Cast<ACustomMonsterActor>(GetEnemyActorByID(i))->GetMesh()->MarkRenderStateDirty();
		}
		GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Cancel));
		MyActionHUD->Cancel();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//								Camera | Party System									  //
////////////////////////////////////////////////////////////////////////////////////////////

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
		SetViewTargetWithBlend(CameraPartyA, 1);
		break;
	case Camera_Party_B:
		SetViewTargetWithBlend(CameraPartyB,1);
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
		SetViewTargetWithBlend(CameraOverhead, 1);
		break;
	case NoneCam:
		SetViewTargetWithBlend(TurnOrder[IndexOfCurrentPlayingMonster], 1);
		break;
	default:
		break;
	}
}

void ACombatController::ChangeCamera(int32 MonsterID)
{
	bIsOverheadCamera = false;
	SetViewTargetWithBlend(MonsterStaticCams[MonsterID],1);
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

////////////////////////////////////////////////////////////////////////////////////////////
//									Turns System										  //
////////////////////////////////////////////////////////////////////////////////////////////

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
	if(IndexOfCurrentPlayingMonster < 5)
		IndexOfCurrentPlayingMonster++;
	else
		IndexOfCurrentPlayingMonster = 0;
	
	
	// Wrapping up some variables.
	MyActionHUD->AbilityBox->ClearChildren();
	MyActionHUD->ItemBox->ClearChildren();
	MyActionHUD->CurrentTarget = "";
	MyActionHUD->CurrentMonsterID = -1;

	
	if (WinningCondition != 0)
	{
		bWinCountdown = true;
		ChangeCamera(ECameras::NoneCam);
		return;
	}

	if (!TurnOrder[IndexOfCurrentPlayingMonster]->bIsDead)
	{

		TurnOrder[IndexOfCurrentPlayingMonster]->MonsterCameraRoot->SetRelativeRotation(TurnOrder[IndexOfCurrentPlayingMonster]->InitialCameraRot);
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);
		TurnOrder[IndexOfCurrentPlayingMonster]->bIsDefending = false;

		bool WillRemove = true;
		while (WillRemove)
		{
			WillRemove = false;
			for (size_t i = 0; i < TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs.Num(); i++)
			{
				TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].TurnsLifetime--;

				if (TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].HealPerTurn>0)
				{
					TurnOrder[IndexOfCurrentPlayingMonster]->UpdateHealth(false, TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].HealPerTurn);
				}
				else
				{
					TurnOrder[IndexOfCurrentPlayingMonster]->UpdateHealth(false, -TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].DamagePerTurn);
					
					if(-TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].DamagePerTurn!=0)
					GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_DOT));
				}

				if (!WillRemove && TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs.Num() > i)
				{
					if (TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].TurnsLifetime == 0)
					{
						TurnOrder[IndexOfCurrentPlayingMonster]->RemoveBuff(TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i]);
						WillRemove = true;
						break;
					}
				}
			}
		}
		

		
	}

	if (TurnOrder[IndexOfCurrentPlayingMonster]->bIsDead)
	{
		ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
		bWillChangeAfterDeath = true;
		HelperPhaseTimer = 0;
	}
	else
	{
		for (size_t i = 0; i < TurnOrder[IndexOfCurrentPlayingMonster]->ActiveAbnormalities.Num(); i++)
		{
			if (TurnOrder[IndexOfCurrentPlayingMonster]->ActiveAbnormalities[i] == EStatusAbnormality::Stun)
			{
				bWillChangeAfterDeath = true;
				HelperPhaseTimer = 0;
				return;
			}
		}

		if (TurnOrder[IndexOfCurrentPlayingMonster]->Party == EPartyType::PlayerControlled)
		{
			CurrentPhase = EActionPhasesType::Undefined;
			MyActionHUD->CurrentMenu = EActiveMenus::MainMenu;
			MyActionHUD->SetVisibility(ESlateVisibility::Visible);
			MyActionHUD->DepthLevel = 0;

		}
		else
		{
			bIsAI_PseudoThink = true;
		}
	}

	MyActionHUD->UpdateTurnOrder();
}

void ACombatController::CheckForWinConditions()
{
	if (PartyA[0]->bIsDead && PartyA[1]->bIsDead && PartyA[2]->bIsDead)
	{
		// AI WINS
		WinningCondition = 2;
	}
	else if (PartyB[0]->bIsDead && PartyB[1]->bIsDead && PartyB[2]->bIsDead)
	{
		// YOU WIN
		WinningCondition = 1;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//								Combat Helper Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////

FVector ACombatController::GetEnemyMeleeReceiverPosByID(int32 ID)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (TurnOrder[i]->MonsterID == ID)
		{
			if (ID > 2)
				return TurnOrder[i]->GetActorLocation() - TurnOrder[i]->GetActorLocation().ForwardVector * 250;
			if (ID < 3)
			{
				return TurnOrder[i]->GetActorLocation() + TurnOrder[i]->GetActorLocation().ForwardVector * 250;
			}
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

////////////////////////////////////////////////////////////////////////////////////////////
//								Combat Main Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////

void ACombatController::ApplyDamage(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim)
{
	if (Victim)
	{
		if (Victim->bIsDead)return;

		bool WillCrit=false;

		if (Damager->HitChance >= FMath::RandRange(0, 100))
		{
			float DamageBonus = (Damager->GetAttack() / 5) * (FMath::FRandRange(Damager->AttackBonus_Min, Damager->AttackBonus_Max) / 100);
			float Damage = (Damager->GetAttack() / 5) + DamageBonus;
			
			Damage -= (Victim->GetDefense() / 500) * Damage;
			Damage -= (Victim->bIsDefending) ? ((Victim->GetDefense()*2) / 500) * Damage / 2 : 0;

			if (Damager->AttackCritChance >= FMath::RandRange(0, 100))
			{
				Damage *= Damager->AttackCritMultiplier;
				WillCrit = true;
			}

			if (Victim->ActiveAbnormalities.Contains(EStatusAbnormality::Reflect))
			{
				Damager->UpdateHealth(WillCrit, -Damage / 10);
			}

			Victim->UpdateHealth(WillCrit, -Damage);
			Victim->AnimInstance->GetHurt();

			FString DeathText = "";
			if (Victim->bIsDead)
			{
				DeathText = Victim->MonsterName + " has died!";
				Victim->ActiveAbnormalities.Empty();
				Victim->ActiveBuffs.Empty();
			}


			if (WillCrit)
			{
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Critical));
				SetBottomTooltip(true, "CRITICAL HIT! " + Victim->MonsterName + " takes " + FString::FromInt(FMath::Abs(Damage)) + " Physical Damage!" + " " + DeathText);
			}
			else
			{
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Hit));
				SetBottomTooltip(true, Victim->MonsterName + " takes " + FString::FromInt(FMath::Abs(Damage)) + " Physical Damage!" + " " + DeathText);
			}
		}
		else
		{
			GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
			Cast<UCombatMeters>(Victim->MyHealthWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(false, FLinearColor(1, 1, 1, 1), "Missed!");
			SetBottomTooltip(true, Damager->MonsterName + " Missed!");
		}
		
	}
}

void ACombatController::ApplyItem(FItem ItemToApply, ACustomMonsterActor* Target)
{
	if (Target)
	{
		FString EffectString;
		int32 EffectCount = 0;

		if (ItemToApply.Resurrection)
		{
			Target->MonsterRez();
			EffectString = "been resurrected back to life";
			EffectCount++;
		}

		if (Target->bIsDead)return;

		if (!Target->bIsDead)
		{
			if (ItemToApply.HP_Restored_Quantity > 0)
			{
				Target->UpdateHealth(false, ItemToApply.HP_Restored_Quantity);

				if (EffectCount > 0)
					EffectString += ", ";

				EffectString += "gained +" + FString::FromInt(ItemToApply.HP_Restored_Quantity) + " HP";
				EffectCount++;
			}
			if (ItemToApply.MP_Restored_Quantity > 0)
			{
				Target->UpdateMana(false, ItemToApply.MP_Restored_Quantity);

				if (EffectCount > 0)
					EffectString += ", ";

				if (EffectString.Contains("gained"))
					EffectString += " +" + FString::FromInt(ItemToApply.MP_Restored_Quantity) + " MP";
				else
					EffectString += "gained +" + FString::FromInt(ItemToApply.MP_Restored_Quantity) + " MP";

				EffectCount++;
			}
			if (ItemToApply.HP_Restored_Percentage > 0)
			{
				int32 HP = Target->MaxHealth * (ItemToApply.HP_Restored_Percentage / 100);
				Target->UpdateHealth(false, HP);

				if (EffectCount > 0)
					EffectString += ", ";

				EffectString += "gained +" + FString::FromInt(ItemToApply.HP_Restored_Percentage) + "% HP";
				EffectCount++;
			}
			if (ItemToApply.MP_Restored_Percentage > 0)
			{
				int32 MP = FMath::RoundToInt((Target->MaxMana) * (ItemToApply.MP_Restored_Percentage / 100));
				Target->UpdateMana(false, MP);

				if (EffectCount > 0)
					EffectString += ", ";

				if (EffectString.Contains("gained"))
					EffectString += " +" + FString::FromInt(ItemToApply.MP_Restored_Percentage) + "% MP";
				else
					EffectString += "gained +" + FString::FromInt(ItemToApply.MP_Restored_Percentage) + "% MP";

				EffectCount++;
			}
		}

		if (ItemToApply.Status_Recovery)
		{
			TArray<int32> IndexesToRemove;
			for (size_t i = 0; i < Target->ActiveBuffs.Num(); i++)
			{
				if (Target->ActiveBuffs[i].CurrentAbnormality != EStatusAbnormality::NoAbnormality)
				{
					IndexesToRemove.Add(i);
				}
			}
			for (size_t i = IndexesToRemove.Num()-1; i > -1; i--)
			{
				Target->RemoveBuff(Target->ActiveBuffs[i]);
			}

			if (EffectCount > 0)
				EffectString += ", ";

			EffectString += "recovered from all status abnormalities";

		}

		EffectString += "!";

		FString TargetString = (MyActionHUD->LatestItem.Target.Contains("Single")) ? MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID] + " has " : "The whole party has ";
		SetBottomTooltip(true, TargetString + EffectString);
	}
}

void ACombatController::ApplyMagicalEffect(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim, FAbility Spell)
{
	if (Victim)
	{
		if (Victim->bIsDead)return;


		bool WillCrit = false;
		bool WillHit = false;
		FString EleText = "";
		FString StatusText = "";
		FString CritText = "";
		FString DeathText = "";

		int32 CachedHPGained = 0; // Used to change the bottom text should both MP and HP are gained.

		if (Spell.Target == "Single Ally" || Spell.Target == "All Allies" || Spell.Target == "Self")WillHit = true;

		FString TargetString;

		if (Spell.Target == "Single Ally" || Spell.Target == "Single Enemy")
		{
			TargetString = MyActionHUD->MonsterNames[MyActionHUD->CurrentMonsterID];
		}
		else if (Spell.Target == "Self")
		{
			TargetString = TurnOrder[IndexOfCurrentPlayingMonster]->MonsterName;
		}
		else if (Spell.Target == "All Enemies")
		{
			TargetString = "All your enemies";
		}
		else if (Spell.Target == "All Allies")
		{
			TargetString = "The whole party";
		}


		int32 MagicAmount;

		if (Spell.SpellHitChance > FMath::RandRange(0, 100))
		{
			WillHit = true;
		}

		if (Spell.BaseSpellPowerHealth != 0 && !Spell.bHasDot && !Spell.bHasHot && Spell.SpellDamageAfterXTurns == 0)
		{
		if (Spell.bIsDamagingForHealth)
		{
			if (WillHit)
			{
				WillHit = true;
				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
				MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;

				MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;
				MagicAmount -= (Victim->bIsDefending) ? ((Victim->GetMagicDefense() * 2) / 500) * MagicAmount / 2 : 0;

				float Multiplier = GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);
				
				if (Multiplier >= 1.5f)
				{
					EleText = "It's Super Effective!";
				}
				else if (Multiplier == 0.5f)
				{
					EleText = "It's Not very Effective...";
				}
				else if (Multiplier == 0)
				{
					EleText = "The Spell has no effect!";
				}

				MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);

				if (Spell.SpellCritChance > FMath::RandRange(0, 100) && MagicAmount > 0)
				{
					MagicAmount *= Spell.SpellCritMultiplier;
					WillCrit = true;
				}

				if (MagicAmount == 0)
				{
					Cast<UCombatMeters>(Victim->MyHealthWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Immune!");

					GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
					SetBottomTooltip(true, "But " + TargetString + " is immune to the element of the spell!");
				}
				else
				{
					Victim->UpdateHealth(WillCrit, -MagicAmount);
					Victim->AnimInstance->GetHurt();

					if (Spell.BottomTooltipText.Contains("%HP_DMG%"))
						Spell.BottomTooltipText = Spell.BottomTooltipText.Replace(TEXT("%HP_DMG%"), *FString::FromInt(MagicAmount));


					SetBottomTooltip(true, TargetString + " takes " + FString::FromInt(FMath::Abs(MagicAmount)) + " Damage! " + EleText);

					GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellImpactSFX);
					GlobalPool->SpawnParticleAtLocation(Victim->GetActorLocation(), MyActionHUD->LatestAbility.ParticleSystem_Impact);
				}
			}
			else
			{
				GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
				Cast<UCombatMeters>(Victim->MyHealthWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Missed!");
				SetBottomTooltip(true, "...But It missed!");
			}
		}
		else
		{
			WillHit = true;
			int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
			MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
			Victim->UpdateHealth(WillCrit, MagicAmount);
			
			if (Spell.BottomTooltipText.Contains("%HP_HEAL%"))
				Spell.BottomTooltipText = Spell.BottomTooltipText.Replace(TEXT("%HP_HEAL%"), *FString::FromInt(MagicAmount));

			CachedHPGained = MagicAmount;

			GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellImpactSFX);
			GlobalPool->SpawnParticleAtLocation(Victim->GetActorLocation(), MyActionHUD->LatestAbility.ParticleSystem_Impact);
		}
	}

		if (Spell.BaseSpellPowerMana != 0 && !Spell.bHasDot && !Spell.bHasHot && Spell.SpellDamageAfterXTurns == 0)
		{
			if (Spell.bIsDamagingForMana)
			{
				if (WillHit)
				{
					WillHit = true;
					int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) * (FMath::FRandRange(Spell.BaseSpellPowerBonusMana_Min, Spell.BaseSpellPowerBonusMana_Max) / 100);
					MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) + BonusMagicAmount;

					MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;

					float Multiplier = GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);
					
					if (Multiplier >= 1.5f)
					{
						EleText = "It's Super Effective!";
					}
					else if (Multiplier == 0.5f)
					{
						EleText = "It's Not very Effective...";
					}
					else if (Multiplier == 0)
					{
						EleText = "The Spell has no effect!";
					}

					MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);

					Victim->UpdateMana(WillCrit, -MagicAmount);
					Victim->AnimInstance->GetHurt();

					if (MagicAmount <= 0)
					{
						Cast<UCombatMeters>(Victim->MyHealthWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Immune!");

						GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
						SetBottomTooltip(true, "But " + TargetString + " is immune to the element of the spell!");
					}
					else
					{
						Victim->UpdateHealth(WillCrit, -MagicAmount);
						Victim->AnimInstance->GetHurt();

						GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellImpactSFX);
						GlobalPool->SpawnParticleAtLocation(Victim->GetActorLocation(), MyActionHUD->LatestAbility.ParticleSystem_Impact);

						SetBottomTooltip(true, TargetString + " takes " + FString::FromInt(FMath::Abs(MagicAmount)) + " MP Damage! " + EleText);
					}
				}
				else
				{
					GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
					Cast<UCombatMeters>(Victim->MyHealthWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Missed!");
					SetBottomTooltip(true, "...But It missed!");
				}
			}
			else
			{
				WillHit = true;
				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) * (FMath::FRandRange(Spell.BaseSpellPowerBonusMana_Min, Spell.BaseSpellPowerBonusMana_Max) / 100);
				MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
				Victim->UpdateMana(WillCrit, MagicAmount);

				if (CachedHPGained > 0)
				{
					SetBottomTooltip(true, TargetString + " gains " + FString::FromInt(CachedHPGained) + " HP and " + FString::FromInt(FMath::Abs(MagicAmount)) + " MP! ");
				}
				else
				{
					SetBottomTooltip(true, TargetString + " gains " + FString::FromInt(FMath::Abs(MagicAmount)) + " MP! ");
				}

				GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellImpactSFX);
				GlobalPool->SpawnParticleAtLocation(Victim->GetActorLocation(), MyActionHUD->LatestAbility.ParticleSystem_Impact);
			}
		}

		if (WillHit)
		{
			if (Spell.ChanceForStatusAbnormality > FMath::RandRange(0, 100) && Spell.TurnsOfEffect > 0)
			{
				FBuff BuffToApply;
				
				BuffToApply.AttackTempChange = Spell.AlterationOfAttack;
				BuffToApply.DefenseTempChange = Spell.AlterationOfDefence;
				BuffToApply.MagicAttackTempChange = Spell.AlterationOfMagicAttack;
				BuffToApply.MagicDefenseTempChange = Spell.AlterationOfMagicDefense;

				BuffToApply.TurnsLifetime = Spell.TurnsOfEffect;
				BuffToApply.CurrentAbnormality = Spell.StatusAbnormality;

				if (Spell.StatusAbnormality == EStatusAbnormality::Poison)
				{
					StatusText = TargetString + " is now Poisoned!";
				}
				if (Spell.StatusAbnormality == EStatusAbnormality::Paralyze)
				{
					StatusText = TargetString + " is now Paralyzed!";
				}
				if (Spell.StatusAbnormality == EStatusAbnormality::Silence)
				{
					StatusText = TargetString + " is now Silenced!";
				}
				if (Spell.StatusAbnormality == EStatusAbnormality::Stun)
				{
					StatusText = TargetString + " is now Stunned!";
				}

				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
				
				if (Spell.bIsDamagingForHealth && Spell.bHasDot)
				{
					MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
					MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;
					MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);
					MagicAmount /= Spell.TurnsOfEffect;

					BuffToApply.DamagePerTurn = MagicAmount;
					BuffToApply.HealPerTurn = 0;
				}
				else if (!Spell.bIsDamagingForHealth && Spell.bHasHot)
				{
					MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
					MagicAmount /= Spell.TurnsOfEffect;

					BuffToApply.HealPerTurn = MagicAmount;
					BuffToApply.DamagePerTurn = 0;
				}
				else if (!Spell.bHasHot && !Spell.bHasDot)
				{
					BuffToApply.HealPerTurn = 0;
					BuffToApply.DamagePerTurn = 0;
				}


				

				MagicAmount = Spell.SpellDamageAfterXTurns + (Damager->GetMagicAttack() / 5);
				BuffToApply.DamageOnEnd = MagicAmount - (Victim->GetMagicDefense() / 500)*MagicAmount;

				BuffToApply.OriginatingSpellName = Spell.Name;
				BuffToApply.BuffID = Victim->ActiveBuffs.Num();

				Victim->ApplyBuff(BuffToApply);
				Victim->AnimInstance->GetHurt();

				GlobalPool->PlaySound2D(MyActionHUD->LatestAbility.SpellImpactSFX);
				GlobalPool->SpawnParticleAtLocation(Victim->GetActorLocation(), MyActionHUD->LatestAbility.ParticleSystem_Impact);

			}
			

			Cast<UCombatMeters>(Victim->MyManaWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 0, 1), Spell.Name);

			
			if (WillCrit)CritText = "Critical Hit! ";

			if (Victim->bIsDead)
			{
				DeathText = TargetString + " has died!";
				StatusText = "";
				Victim->ActiveAbnormalities.Empty();
				Victim->ActiveBuffs.Empty();
			}

			if(Spell.Target != "All Enemies" && Spell.Target != "All Allies")
				SetBottomTooltip(true, CritText + TargetString + " " + Spell.BottomTooltipText + " " + EleText + " " + DeathText + " " + StatusText);
			else
				SetBottomTooltip(true, TargetString + " " + Spell.BottomTooltipText);
			
		}
		else
		{
			SetBottomTooltip(true, "...But it missed!");
			GlobalPool->PlaySound2D(SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_Missed));
		}
		


		MyActionHUD->MonsterHPs_Current[Victim->MonsterID] = Victim->CurrentHealth;
		MyActionHUD->MonsterMPs_Current[Victim->MonsterID] = Victim->CurrentMana;
	}
}

float ACombatController::GetMultiplierBasedOnElements(EElementalPower Element_Spell, EElementalPower Element_Victim)
{
	if (Element_Spell == EElementalPower::Fire && Element_Victim == EElementalPower::Earth)
	{
		return 2;
	}
	if (Element_Spell == EElementalPower::Earth && Element_Victim == EElementalPower::Storm)
	{
		return 2;
	}
	if (Element_Spell == EElementalPower::Storm && Element_Victim == EElementalPower::Water)
	{
		return 2;
	}
	if (Element_Spell == EElementalPower::Water && Element_Victim == EElementalPower::Fire)
	{
		return 2;
	}


	if (Element_Spell == EElementalPower::Earth && Element_Victim == EElementalPower::Fire)
	{
		return 0.5f;
	}
	if (Element_Spell == EElementalPower::Storm && Element_Victim == EElementalPower::Earth)
	{
		return 0.5f;
	}
	if (Element_Spell == EElementalPower::Water && Element_Victim == EElementalPower::Storm)
	{
		return 0.5f;
	}
	if (Element_Spell == EElementalPower::Fire && Element_Victim == EElementalPower::Water)
	{
		return 0.5f;
	}

	if (Element_Spell == EElementalPower::Arcane)
	{
		return 1.5f;
	}
	

	if (Element_Spell ==  Element_Victim  && Element_Spell != EElementalPower::Normal)
	{
		return 0.f;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////
//								UI Helper Function										  //
////////////////////////////////////////////////////////////////////////////////////////////

void ACombatController::SetBottomTooltip(bool bShouldReset, FString ActualText)
{
	MyActionHUD->bIsCustomTooltip = bShouldReset;
	MyActionHUD->BottomTooltip = ActualText;

}

////////////////////////////////////////////////////////////////////////////////////////////
//								Inventory Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////

void ACombatController::InitializeInventoryStock()
{
	GroupAInventory = GroupInventories;
	GroupBInventory = GroupInventories;

	MyActionHUD->GroupAInventory = GroupAInventory;
	MyActionHUD->GroupBInventory = GroupBInventory;
}

////////////////////////////////////////////////////////////////////////////////////////////
//								AI Related Function										  //
////////////////////////////////////////////////////////////////////////////////////////////

void ACombatController::AI_ExecuteAction()
{
	APlayerController* PC = Cast<APlayerController>(this);
	Cast<AMonsterAIController>(TurnOrder[IndexOfCurrentPlayingMonster]->Controller)->DecideNextAction(PC);
}