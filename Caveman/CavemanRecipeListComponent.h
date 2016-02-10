// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CavemanRecipeListComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAVEMANTESTFIELD_API UCavemanRecipeListComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// ******************************************************************************
	// The Caveman knows Item Crafting Recipes and Cooking Recipes.
	//
	// The FString Array Holds the Actual Recipes (Data Bound) and the bool Array
	// holds whether or now we know them.
	// ******************************************************************************


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe List")
		TArray<FString> Items_Recipes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe List")
		TArray<bool> Recipe_Knowledge;

	// ---------------------------------------------------------------------------- //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe List")
		TArray<FString> Cook_Items_Recipes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe List")
		TArray<bool> Cook_Recipe_Knowledge;
	
};
