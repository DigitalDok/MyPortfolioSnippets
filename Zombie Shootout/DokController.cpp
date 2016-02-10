// Fill out your copyright notice in the Description page of Project Settings.

#include "RoguelikeSurvival.h"
#include "DokController.h"
#include "RoguelikeChar.h"


void ADokController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerUserInterfaceHolder)
	{
		GameplayUI = Cast<URoguelikeMainUI>(CreateWidget<UUserWidget>(this, PlayerUserInterfaceHolder));
		GameplayUI->AddToViewport();
	}

	bShowMouseCursor = false;
	BuildLevelFromText();
}

#pragma region Level Building Procedures

void ADokController::BuildLevelFromText()
{
	FString Tsirla;
	//this is a nice comment
	FString GameDir = FPaths::GameDir();
	FString CompletePath = GameDir + "Content/Map/Dungeon"+FString::FromInt(FMath::RandRange(1,3))+".txt";
	FFileHelper::LoadFileToString(Tsirla, *CompletePath);

	GLog->Log(CompletePath);

	int32 IndexOfWidth = Tsirla.Find(";", ESearchCase::IgnoreCase, ESearchDir::FromStart, 0);
	int32 IndexOfHeight = Tsirla.Find(";", ESearchCase::IgnoreCase, ESearchDir::FromStart, IndexOfWidth+1);

	DungeonWidth = FCString::Atoi(*Tsirla.Mid(0, IndexOfWidth));
	DungeonHeight = FCString::Atoi(*Tsirla.Mid(IndexOfWidth + 1, IndexOfHeight - 1));

	My2DLevel.AddUninitialized(DungeonWidth,DungeonHeight);

	int32 TotalDungeonItr = 0;
	FString RoomFlag="";
	for (int32 j = 0; j < DungeonHeight; j++)
	{
		for (int32 i = 0; i < DungeonWidth; i++)
		{

			RoomFlag = Tsirla.Mid(IndexOfHeight + 1 + TotalDungeonItr, 1);
			while (RoomFlag == "-" || RoomFlag == " ")
			{
				TotalDungeonItr++;
				RoomFlag = Tsirla.Mid(IndexOfHeight + 1 + TotalDungeonItr, 1);
			}

			TotalDungeonItr++;
			My2DLevel.Rows[i].Columns[j] = RoomFlag;
		}
	}


	for (int32 j = 0; j < DungeonHeight; j++)
	{
		for (int32 i = 0; i < DungeonWidth; i++)
		{
			
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.Instigator = Instigator;
			AActor* SpawnedObject;

			ETileTypes MyType = DetermineTileType(i, j);
			
			
			if (MyType != ETileTypes::None && MyType != ETileTypes::Empty)
			{
				SpawnedObject = GetWorld()->SpawnActor<AActor>(Tile_Floor, FVector(j * 100 * TileScaling, i * 100 * TileScaling, 0), FRotator(0), spawnParams);
				SpawnedObject->SetActorScale3D(FVector(SpawnedObject->GetActorScale3D().X * TileScaling, SpawnedObject->GetActorScale3D().Y * TileScaling, SpawnedObject->GetActorScale3D().Z * TileScaling));
				
				if (MyType==ETileTypes::Spike_Down || MyType==ETileTypes::Spike_Left || MyType==ETileTypes::Spike_Right || MyType==ETileTypes::Spike_Up)
				{
					SpawnedObject->Tags.Add(FName("Spike"));
				}
				else SpawnedObject->Tags.Add(FName("PowerUpTile"));



			}
			
			SpawnedObject = GetWorld()->SpawnActor<AActor>(Tile_Roof, FVector(j * 100 * TileScaling, i * 100 * TileScaling, 99 * TileScaling), FRotator(0), spawnParams);
			SpawnedObject->SetActorScale3D(FVector(SpawnedObject->GetActorScale3D().X * TileScaling, SpawnedObject->GetActorScale3D().Y * TileScaling, SpawnedObject->GetActorScale3D().Z * TileScaling));

			switch (MyType)
			{
			case Single_Floor:

				break;
			case Empty:

				break;
			case Wall_Up:
				SpawnWall(EWallType::Up, i, j);
				break;
			case Wall_Down:
				SpawnWall(EWallType::Down, i, j);
				break;
			case Wall_Left:
				SpawnWall(EWallType::Left, i, j);
				break;
			case Wall_Right:
				SpawnWall(EWallType::Right, i, j);
				break;
			case Corner_UpRight:
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			case Corner_UpLeft:
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Left, i, j);
				break;
			case Corner_DownRight:
				SpawnWall(EWallType::Down, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			case Corner_DownLeft:
				SpawnWall(EWallType::Down, i, j);
				SpawnWall(EWallType::Left, i, j);
				break;
			case Corridor_Vertical:
				SpawnWall(EWallType::Left, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			case Corridor_Horizontal:
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Down, i, j);
				break;
			case Spike_Left:
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Down, i, j);
				SpawnWall(EWallType::Left, i, j);
				break;
			case Spike_Right:
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Down, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			case Spike_Up:
				SpawnWall(EWallType::Left, i, j);
				SpawnWall(EWallType::Up, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			case Spike_Down:
				SpawnWall(EWallType::Left, i, j);
				SpawnWall(EWallType::Down, i, j);
				SpawnWall(EWallType::Right, i, j);
				break;
			default:
				break;
			}

		}
	}
}

bool ADokController::IsThisTileOccupied(int32 X, int32 Y)
{
	if (My2DLevel.Rows[X].Columns[Y] == "#")
	{
		return true;
	}

	return false;
}

bool ADokController::DoesThisTileExist(int32 X, int32 Y)
{
	if (X == -1 || Y == -1 || X == DungeonWidth || Y == DungeonHeight)
	{
		return false;
	}
	else
	{
		return true;
	}
}

ETileTypes ADokController::DetermineTileType(int32 X, int32 Y)
{
	

	if (IsThisTileOccupied(X, Y))
	{
		bool UpValid;
		bool DownValid;
		bool RightValid;
		bool LeftValid;

		if (DoesThisTileExist(X, Y - 1))
		{
			if (IsThisTileOccupied(X, Y - 1))
			{
				UpValid = true;
			}
			else
			{
				UpValid = false;
			}
		}
		else
		{
			UpValid = false;
		}

		if (DoesThisTileExist(X, Y + 1))
		{
			if (IsThisTileOccupied(X, Y + 1))
			{
				DownValid = true;
			}
			else
			{
				DownValid = false;
			}
		}
		else
		{
			DownValid = false;
		}

		if (DoesThisTileExist(X-1, Y))
		{
			if (IsThisTileOccupied(X-1, Y ))
			{
				LeftValid = true;
			}
			else
			{
				LeftValid = false;
			}
		}
		else
		{
			LeftValid = false;
		}

		if (DoesThisTileExist(X + 1, Y))
		{
			if (IsThisTileOccupied(X + 1, Y))
			{
				RightValid = true;
			}
			else
			{
				RightValid = false;
			}
		}
		else
		{
			RightValid = false;
		}

		if (UpValid && DownValid && LeftValid && RightValid)
		{
			return ETileTypes::Single_Floor;
		}

		if (UpValid && DownValid && LeftValid && !RightValid)
		{
			return ETileTypes::Wall_Right;
		}
		if (UpValid && DownValid && !LeftValid && RightValid)
		{
			return ETileTypes::Wall_Left;
		}
		if (UpValid && !DownValid && LeftValid && RightValid)
		{
			return ETileTypes::Wall_Down;
		}
		if (!UpValid && DownValid && LeftValid && RightValid)
		{
			return ETileTypes::Wall_Up;
		}

		if (!UpValid && !DownValid && !LeftValid && RightValid)
		{
			return ETileTypes::Spike_Left;
		}
		if (!UpValid && !DownValid && LeftValid && !RightValid)
		{
			return ETileTypes::Spike_Right;
		}
		if (!UpValid && DownValid && !LeftValid && !RightValid)
		{
			return ETileTypes::Spike_Up;
		}
		if (UpValid && !DownValid && !LeftValid && !RightValid)
		{
			return ETileTypes::Spike_Down;
		}

		if (UpValid && DownValid && !LeftValid && !RightValid)
		{
			return ETileTypes::Corridor_Vertical;
		}
		if (!UpValid && !DownValid && LeftValid && RightValid)
		{
			return ETileTypes::Corridor_Horizontal;
		}

		if (!UpValid && DownValid && !LeftValid && RightValid)
		{
			return ETileTypes::Corner_UpLeft;
		}
		if (!UpValid && DownValid && LeftValid && !RightValid)
		{
			return ETileTypes::Corner_UpRight;
		}
		if (UpValid && !DownValid && LeftValid && !RightValid)
		{
			return ETileTypes::Corner_DownRight;
		}
		if (UpValid && !DownValid && !LeftValid && RightValid)
		{
			return ETileTypes::Corner_DownLeft;
		}


	}
	else
	{
		return ETileTypes::Empty;
	}
	return ETileTypes::None;
}

void ADokController::SpawnWall(EWallType Wall, int32 i, int32 j)
{
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = Instigator;
	AActor* SpawnedWall = nullptr;

	switch (Wall)
	{
		case Up:
			SpawnedWall = GetWorld()->SpawnActor<AActor>(Tile_Wall, FVector((j * 100 - 50) * TileScaling, (i * 100) * TileScaling, 50 * TileScaling), FRotator(90, 0, 0), spawnParams);
			SpawnedWall->AddActorWorldRotation(FRotator(0, 0, 90));
			break;
		case Left:
			SpawnedWall = GetWorld()->SpawnActor<AActor>(Tile_Wall, FVector((j * 100) * TileScaling, (i * 100 - 50) * TileScaling, 50 * TileScaling), FRotator(90, 90, 0), spawnParams);
			SpawnedWall->AddActorWorldRotation(FRotator(90, 0, 0));
			break;
		case Right:
			SpawnedWall = GetWorld()->SpawnActor<AActor>(Tile_Wall, FVector((j * 100) * TileScaling, (i * 100 + 50) * TileScaling, 50 * TileScaling), FRotator(90, 90, 0), spawnParams);
			SpawnedWall->AddActorWorldRotation(FRotator(90, 0, 0));
			break;
		case Down:
			SpawnedWall = GetWorld()->SpawnActor<AActor>(Tile_Wall, FVector((j * 100 + 50) * TileScaling,( i * 100) * TileScaling, 50 * TileScaling), FRotator(90, 0, 0), spawnParams);
			SpawnedWall->AddActorWorldRotation(FRotator(0, 0, 90));
			break;
	}
	SpawnedWall->SetActorScale3D(FVector(SpawnedWall->GetActorScale3D().X * TileScaling, SpawnedWall->GetActorScale3D().Y * TileScaling, SpawnedWall->GetActorScale3D().Z * TileScaling));
}

#pragma endregion

// ----------- Update the User Interface elements by transferring all character data to the UserWidget -------- //

void ADokController::UpdateUI()
{
	ARoguelikeChar* LeChar = Cast<ARoguelikeChar>(GetCharacter());
	if (LeChar)
	{
		GameplayUI->Health = LeChar->CurrentHealth;

		GameplayUI->BulletsLeft_A = LeChar->BulletsLeft_A;
		GameplayUI->BulletsLeft_B = LeChar->BulletsLeft_B;
		GameplayUI->BulletsLeft_C = LeChar->BulletsLeft_C;

		GameplayUI->BulletsLeft_A_Total = LeChar->BulletsLeft_A_Total;
		GameplayUI->BulletsLeft_B_Total = LeChar->BulletsLeft_B_Total;
		GameplayUI->BulletsLeft_C_Total = LeChar->BulletsLeft_C_Total;

		GameplayUI->CurrentlyEquippedBulletType = LeChar->CurrentBulletEquipped;

		GameplayUI->bIsReloading = LeChar->PlayerAnimationInstance->bIsReloading;

		GameplayUI->bIsSprinting = LeChar->bIsSprinting;

		GameplayUI->Kills = LeChar->Kills;
		GameplayUI->Wave = LeChar->Wave;

		GameplayUI->bIsWaveCountingDown = LeChar->bIsWaveCountingDown;
		GameplayUI->WaveCountdown = LeChar->WaveCountdown;

		GameplayUI->PowerupName = LeChar->PowerupName;

		GameplayUI->Stamina = LeChar->CurrentStamina;
	}
	
}


