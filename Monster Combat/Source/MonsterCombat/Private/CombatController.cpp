
#include "MonsterCombat.h"
#include "CombatController.h"
#include "AIController.h"
#include "CombatMeters.h"

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
}

void ACombatController::InitializerGrande()
{

	IndexOfCurrentPlayingMonster = -1;
	ReferenceCameras();
	DetermineParties();
	DetermineTurnOrder();
	InitializeInventoryStock();

	FBuff BuffToApply;
	BuffToApply.DamagePerTurn = 5;
	BuffToApply.TurnsLifetime = 3;

	BuffToApply.AttackTempChange = 0;
	BuffToApply.DefenseTempChange = 0;
	BuffToApply.MagicDefenseTempChange = 0;
	BuffToApply.MagicAttackTempChange = 0;

	BuffToApply.CurrentAbnormality = EStatusAbnormality::Stun;

	BuffToApply.BuffID = 0;
	BuffToApply.DamageOnEnd = 0;
	TurnOrder[0]->ApplyBuff(BuffToApply);

	FBuff BuffToApply2;
	BuffToApply2.DamagePerTurn = 5;
	BuffToApply2.TurnsLifetime = 3;

	BuffToApply2.AttackTempChange = 0;
	BuffToApply2.DefenseTempChange = 0;
	BuffToApply2.MagicDefenseTempChange = 0;
	BuffToApply2.MagicAttackTempChange = 0;

	BuffToApply2.CurrentAbnormality = EStatusAbnormality::Paralyze;

	BuffToApply2.BuffID = 0;
	BuffToApply2.DamageOnEnd = 0;
	TurnOrder[0]->ApplyBuff(BuffToApply2);

	// Wiring up the delegate that will take care of destroying an audio comp when a sound has finished playing.
	AudioFinishDelegate.BindUFunction(this, "DestroyFinishedAudio");

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

	if (!MyActionHUD)return; // This indicates that the game has not started yet, and keeps Controller out of looping while in Editor.

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
	{
		CameraOverhead->SetActorRotation(FindLookAtRotation(CameraOverhead->GetActorLocation(), TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation()));
	}


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
		}
	}
	if (CurrentAttackPhase == EActionPhasesAttack::MoveTowardsInitialPosition)
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
			for (size_t i = 0; i < 6; i++)
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

			MyActionHUD->bIsFadingOut = true;
			// USE ITEM

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
		else if (MyActionHUD->CurrentTarget == "All Allies" || MyActionHUD->CurrentTarget == "All Enemy")
		{
			for (size_t i = 0; i < 6; i++)
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
	else if (CurrentAbilityPhase == EActionPhasesAbility::TargetActorCameraEffect)
	{
		HelperPhaseTimer += DeltaTime;
		if (HelperPhaseTimer > GlobalWaitingTimer)
		{
			HelperPhaseTimer = 0;
			CurrentAbilityPhase = EActionPhasesAbility::SpellFinalizing;


			if (MyActionHUD->CurrentTarget == "All Allies")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(0)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(1)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(2)), MyActionHUD->LatestAbility);
			}
			else if (MyActionHUD->CurrentTarget == "All Enemies")
			{
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(3)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(4)), MyActionHUD->LatestAbility);
				ApplyMagicalEffect(TurnOrder[IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(GetEnemyActorByID(5)), MyActionHUD->LatestAbility);
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
		if (HelperPhaseTimer > GlobalWaitingTimer)
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
		if (HelperPhaseTimer > GlobalWaitingTimer)
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

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);
				MyActionHUD->bIsFadingOut = false;
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
				MyActionHUD->DepthLevel = 3;
				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
				
				MyActionHUD->CurrentMenu = EActiveMenus::None;

				MyActionHUD->bIsCustomTooltip = false;
				MyActionHUD->BottomTooltip = "";
				MyActionHUD->bIsFadingOut = false;
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
				MyActionHUD->DepthLevel = 3;
				MyActionHUD->CurrentMenu = EActiveMenus::None;
				ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);

				ChangeCamera(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID);

				CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
				MyActionHUD->bIsFadingOut = false;
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

				CurrentItemPhase = EActionPhasesItem::GlobalItemUsageWait;

				MyActionHUD->bIsFadingOut = false;
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
		MyActionHUD->Cancel();
	}
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
	if(IndexOfCurrentPlayingMonster < 5)
		IndexOfCurrentPlayingMonster++;
	else
		IndexOfCurrentPlayingMonster = 0;
	
	
	if (!TurnOrder[IndexOfCurrentPlayingMonster]->bIsDead)
	{
		TurnOrder[IndexOfCurrentPlayingMonster]->MonsterCameraRoot->SetRelativeRotation(TurnOrder[IndexOfCurrentPlayingMonster]->InitialCameraRot);
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);
		TurnOrder[IndexOfCurrentPlayingMonster]->bIsDefending = false;

		for (size_t i = 0; i < TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs.Num(); i++)
		{
			TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].TurnsLifetime--;

			TurnOrder[IndexOfCurrentPlayingMonster]->UpdateHealth(false, -TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].DamagePerTurn);

			if (TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i].TurnsLifetime == 0)
			{
				TurnOrder[IndexOfCurrentPlayingMonster]->RemoveBuff(TurnOrder[IndexOfCurrentPlayingMonster]->ActiveBuffs[i]);
			}
		}
	}

	if (TurnOrder[IndexOfCurrentPlayingMonster]->bIsDead)
	{
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
			EndTurn();
		}
	}

	MyActionHUD->UpdateTurnOrder();
}

void ACombatController::CheckForWinConditions()
{
	if (PartyA[0]->bIsDead && PartyA[1]->bIsDead && PartyA[2]->bIsDead)
	{
		// AI WINS
	}
	else if (PartyB[0]->bIsDead && PartyB[1]->bIsDead && PartyB[2]->bIsDead)
	{
		// YOU WIN
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
		if (Victim->bIsDead)return;

		bool WillCrit=false;

		if (Damager->HitChance >= FMath::RandRange(0, 100))
		{
			int32 DamageBonus = (Damager->GetAttack() / 5) * (FMath::FRandRange(Damager->AttackBonus_Min, Damager->AttackBonus_Max) / 100);
			int32 Damage = (Damager->GetAttack() / 5) + DamageBonus;
			
			Damage -= (Victim->GetDefense() / 500) * Damage;
			Damage -= (Victim->bIsDefending) ? ((Victim->GetDefense()*2) / 500) * Damage / 2 : 0;

			if (Damager->AttackCritChance >= FMath::RandRange(0, 100))
			{
				Damage *= Damager->AttackCritMultiplier;
				WillCrit = true;
			}

			Victim->UpdateHealth(WillCrit ,-Damage);
			Victim->AnimInstance->GetHurt();
		}
		else
		{
			Cast<UCombatMeters>(Victim->MyWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(false, FLinearColor(1, 1, 1, 1), "Missed!");
		}
		
	}
}

void ACombatController::ApplyItem(FItem ItemToApply, ACustomMonsterActor* Target)
{
	if (Target)
	{
		if (ItemToApply.Resurrection)
		{
			Target->bIsDead = false;
		}

		if (Target->bIsDead)return;

		if (!Target->bIsDead)
		{
			if (ItemToApply.HP_Restored_Quantity > 0)
			{
				Target->UpdateHealth(false, ItemToApply.HP_Restored_Quantity);
			}
			if (ItemToApply.MP_Restored_Quantity > 0)
			{
				Target->UpdateMana(false, ItemToApply.MP_Restored_Quantity);
			}
			if (ItemToApply.HP_Restored_Percentage > 0)
			{
				Target->UpdateHealth(false, (Target->MaxHealth) * (ItemToApply.HP_Restored_Percentage / 100));
			}
			if (ItemToApply.MP_Restored_Percentage > 0)
			{
				Target->UpdateMana(false, (Target->MaxMana) * (ItemToApply.MP_Restored_Percentage / 100));
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

		}
	}
}

void ACombatController::ApplyMagicalEffect(ACustomMonsterActor* Damager, ACustomMonsterActor* Victim, FAbility Spell)
{
	if (Victim)
	{
		if (Victim->bIsDead)return;

		Damager->CurrentMana -= Spell.ManaCost;
		MyActionHUD->MonsterMPs_Current[TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID] = Damager->CurrentMana;

		bool WillCrit = false;
		bool WillHit = false;

		if (Spell.Target == "Single Ally" || Spell.Target == "All Allies" || Spell.Target == "Self")WillHit = true;

		int32 MagicAmount;
		if (Spell.BaseSpellPowerHealth != 0)
		{
		if (Spell.bIsDamagingForHealth)
		{
			if (Spell.SpellHitChance > FMath::RandRange(0, 100) || WillHit)
			{
				WillHit = true;
				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
				MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;

				MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;
				MagicAmount -= (Victim->bIsDefending) ? ((Victim->GetMagicDefense() * 2) / 500) * MagicAmount / 2 : 0;

				MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);

				if (Spell.SpellCritChance > FMath::RandRange(0, 100) && MagicAmount > 0)
				{
					MagicAmount *= Spell.SpellCritMultiplier;
					WillCrit = true;
				}

				Victim->UpdateHealth(WillCrit, -MagicAmount);
				Victim->AnimInstance->GetHurt();

				if (MagicAmount <= 0)
					Cast<UCombatMeters>(Victim->MyWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Immune!");
			}
			else
			{
				Cast<UCombatMeters>(Victim->MyWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Missed!");
			}
		}
		else
		{
			WillHit = true;
			int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
			MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
			Victim->UpdateHealth(WillCrit, MagicAmount);
		}
	}

		if (Spell.BaseSpellPowerMana != 0)
		{
			if (Spell.bIsDamagingForMana)
			{
				if (Spell.SpellHitChance > FMath::RandRange(0, 100) || WillHit)
				{
					WillHit = true;
					int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) * (FMath::FRandRange(Spell.BaseSpellPowerBonusMana_Min, Spell.BaseSpellPowerBonusMana_Max) / 100);
					MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) + BonusMagicAmount;

					MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;

					MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);

					Victim->UpdateMana(WillCrit, -MagicAmount);
					Victim->AnimInstance->GetHurt();
					if (MagicAmount <= 0)
						Cast<UCombatMeters>(Victim->MyWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Immune!");
				}
				else
				{
					Cast<UCombatMeters>(Victim->MyWidgetComponent->GetUserWidgetObject())->CreateNumberOverHead(WillCrit, FLinearColor(1, 1, 1, 1), "Missed!");
				}
			}
			else
			{
				WillHit = true;
				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerMana) * (FMath::FRandRange(Spell.BaseSpellPowerBonusMana_Min, Spell.BaseSpellPowerBonusMana_Max) / 100);
				MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
				Victim->UpdateMana(WillCrit, MagicAmount);
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

				int32 BonusMagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) * (FMath::FRandRange(Spell.BaseSpellPowerBonusHealth_Min, Spell.BaseSpellPowerBonusHealth_Max) / 100);
				
				MagicAmount = ((Damager->GetMagicAttack() / 5) + Spell.BaseSpellPowerHealth) + BonusMagicAmount;
				MagicAmount -= (Victim->GetMagicDefense() / 500)*MagicAmount;
				MagicAmount *= GetMultiplierBasedOnElements(Damager->MonsterAffinityType, Victim->MonsterAffinityType);
				MagicAmount /= Spell.TurnsOfEffect;

				BuffToApply.DamagePerTurn = MagicAmount;

				MagicAmount = Spell.SpellDamageAfterXTurns + (Damager->GetMagicAttack() / 5);
				BuffToApply.DamageOnEnd = MagicAmount - (Victim->GetMagicDefense() / 500)*MagicAmount;

				
				BuffToApply.BuffID = Victim->ActiveBuffs.Num();

				Victim->ApplyBuff(BuffToApply);
			}
		}
		


		MyActionHUD->MonsterHPs_Current[Victim->MonsterID] = Victim->CurrentHealth;
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
		return 0;
	}
	if (Element_Spell == EElementalPower::Storm && Element_Victim == EElementalPower::Earth)
	{
		return 0;
	}
	if (Element_Spell == EElementalPower::Water && Element_Victim == EElementalPower::Storm)
	{
		return 0;
	}
	if (Element_Spell == EElementalPower::Fire && Element_Victim == EElementalPower::Water)
	{
		return 0;
	}

	if (Element_Spell == EElementalPower::Arcane)
	{
		return 1.5f;
	}
	

	if (Element_Spell ==  Element_Victim )
	{
		return 0.5f;
	}

	return 1;
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