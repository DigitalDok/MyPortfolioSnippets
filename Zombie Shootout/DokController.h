// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "RoguelikeMainUI.h"
#include "DokController.generated.h"


#pragma region Structs


// ********************************************************************************************
// A whole Struct of DungeonGridRows is used to represent a single row in a 2D Array (Dungeon).
// ********************************************************************************************

USTRUCT()
struct FDungeonGridRow
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		TArray<FString> Columns;

	void AddNewColumn()
	{
		Columns.Add("");
	}
	
	FDungeonGridRow()
	{

	}
};


// ********************************************************************************************
// Many FDungeonGridRows will form the final Dungeon, represented as a FDungeonGrid Struct.
// ********************************************************************************************

USTRUCT()
struct FDungeonGrid
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		TArray<FDungeonGridRow> Rows;

	void AddNewRow()
	{
		Rows.Add(FDungeonGridRow());
	}

	void AddUninitialized(const int32 RowCount, const int32 ColCount)
	{
		//Add Rows
		for (int32 v = 0; v < RowCount; v++)
		{
			AddNewRow();
		}

		//Add Columns
		for (int32 v = 0; v < RowCount; v++)
		{
			for (int32 b = 0; b < ColCount; b++)
			{
				Rows[v].AddNewColumn();
			}
		}
	}

	//default properties
	FDungeonGrid()
	{

	}
};

#pragma endregion

#pragma region Enums

// *************************************************************************
// All of the common tile types which will be used when generating our level
// *************************************************************************

UENUM()
enum ETileTypes
{
	Single_Floor,
	Empty,
	Wall_Up,
	Wall_Down,
	Wall_Left,
	Wall_Right,
	Corner_UpRight,
	Corner_UpLeft,
	Corner_DownRight,
	Corner_DownLeft,
	Corridor_Vertical,
	Corridor_Horizontal,
	Spike_Left,
	Spike_Right,
	Spike_Up,
	Spike_Down,
	None
};

// *********************************************************************************************************
// All of the wall types used to define which part in a 4-Directional Tile will be covered with which piece.
// *********************************************************************************************************

UENUM()
enum EWallType
{
	Up,
	Left,
	Right,
	Down
};

#pragma endregion

UCLASS()
class ROGUELIKESURVIVAL_API ADokController : public APlayerController
{
	GENERATED_BODY()

private:
	FVector PlayerSpawnPosition;

public:
	FORCEINLINE FVector GetPlayerSpawnPosition() { return PlayerSpawnPosition; }
#pragma region Fields
		
#pragma region User Interface

	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> PlayerUserInterfaceHolder;

	URoguelikeMainUI* GameplayUI;

#pragma endregion

#pragma region Level Generation

	FDungeonGrid My2DLevel;

	UPROPERTY(EditAnywhere, Category = "Spawning Tiles")
		TSubclassOf<AActor> Tile_Floor;
	
	UPROPERTY(EditAnywhere, Category = "Spawning Tiles")
		TSubclassOf<AActor> Tile_Wall;

	UPROPERTY(EditAnywhere, Category = "Spawning Tiles")
		TSubclassOf<AActor> Tile_Roof;

	int32 DungeonWidth;
	int32 DungeonHeight;

	UPROPERTY(EditAnywhere, Category = "Spawning Tiles")
		float TileScaling;

#pragma endregion

#pragma endregion

#pragma region Functions

#pragma region Core
	// -- Our own definition of the BeginPlay Function -- //
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Procedural Level Generation

	bool IsThisTileOccupied(int32 X, int32 Y);

	bool DoesThisTileExist(int32 X, int32 Y);

	ETileTypes DetermineTileType(int32 X, int32 Y);

	void SpawnWall(EWallType Wall, int32 X, int32 Y);

	void BuildLevelFromText();
#pragma endregion

#pragma region User Interface

	void UpdateUI();

#pragma endregion

#pragma endregion

	

	

	
};
