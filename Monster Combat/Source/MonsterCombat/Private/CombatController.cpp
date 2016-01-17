
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
	EndTurn();
}

void ACombatController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (MyActionHUD)
	{
		MyActionHUD->CurrentMonsterID = -1;
		if (bIsActorClickEnabled)
		{
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

	if (bIsWalkingTowardsDestination)
	{
		FVector CurPos = TurnOrder[IndexOfCurrentPlayingMonster]->GetActorLocation();
		if(FVector::Dist(CurPos,ReceiverPos)< 200)
			GLog->Log("HEY");
		//if (TurnOrder[IndexOfCurrentPlayingMonster]->GetMovementComponent()->Velocity == FVector(0))
		//{
		//	
		//	if(CurPos.X == ReceiverPos.X && CurPos.Y == ReceiverPos.Y)
		//	//EPathFollowingStatus::Type status = Cast<AAIController>(TurnOrder[IndexOfCurrentPlayingMonster]->GetController())->GetMoveStatus();
		//	//if(status == EPathFollowingStatus::Type::Idle)
		//	
		//}
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
	if (MyActionHUD->CurrentMonsterID != -1)
	{
		if (MyActionHUD->DepthLevel == 1)
		{
			MyActionHUD->DepthLevel = 3;
			bIsActorClickEnabled = false;
			ChangeCamera(ECameras::NoneCam);
			MyActionHUD->CurrentMenu = EActiveMenus::None;
			ReceiverPos = GetEnemyMeleeReceiverPosByID(MyActionHUD->CurrentMonsterID);
			UNavigationSystem::SimpleMoveToLocation(GetEnemyControllerByID(TurnOrder[IndexOfCurrentPlayingMonster]->MonsterID), ReceiverPos);
			bIsWalkingTowardsDestination = true;
		}
		else if (MyActionHUD->DepthLevel == 2)
		{

		}
	}
}

void ACombatController::RMB()
{
	if(MyActionHUD->DepthLevel>0)
	MyActionHUD->Cancel();
}


void ACombatController::ReferenceCameras()
{
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
	}
}

void ACombatController::ChangeCamera(ECameras CamType)
{
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
		break;
	case NoneCam:
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);
		break;
	default:
		break;
	}
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
		IndexOfCurrentPlayingMonster = 0;
	else
	{
		IndexOfCurrentPlayingMonster++;
	}

	if (TurnOrder[IndexOfCurrentPlayingMonster]->Party == EPartyType::PlayerControlled)
	{
		TurnOrder[IndexOfCurrentPlayingMonster]->MonsterCameraRoot->SetRelativeRotation(TurnOrder[IndexOfCurrentPlayingMonster]->InitialCameraRot);
		SetViewTarget(TurnOrder[IndexOfCurrentPlayingMonster]);

		MyActionHUD->CurrentMenu = EActiveMenus::MainMenu;
		MyActionHUD->SetVisibility(ESlateVisibility::Visible);
		MyActionHUD->DepthLevel = 0;
	}
	else
	{

	}
}

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