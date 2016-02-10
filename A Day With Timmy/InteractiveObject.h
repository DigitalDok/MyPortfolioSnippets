// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "DialogueSystem.h"
#include "InteractiveObject.generated.h"

UCLASS()
class POINTANDCLICK_API AInteractiveObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveObject();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
		float InteractionRange;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
		float InspectionRange;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
		FVector CharacterPositionNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Stuff")
		FString ObjectName;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
	int32 ItemNameNeededToProceed_TargetBranchID;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
	FString ItemNameNeededToProceed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction System [Inspect]")
		TArray<FDialogBranch> InspectBranches;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction System [Interact]")
		TArray<FDialogBranch> InteractBranches;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction System [Inspect]")
		int32 InspectBranchID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction System [Interact]")
		int32 InteractBranchID;

	UPROPERTY(EditAnywhere, Category = "Interaction System")
		bool RequiresSpecialItemToBegin;

	// ****************************************************************************************
	// Dialogue Tree System - Enabled when interacting with a humanoid
	// ****************************************************************************************
	
	UPROPERTY()
	UDialogueSystem* MasterDialogueSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Dialogue Tree")
		bool bIsHuman;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		TArray<FDialogBranch> DialogueBanches;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		TArray<AActor*> Cameras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		UTexture2D* Portrait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		FString MyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		int32 DialogueTreeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		int32 InitialTreeIndex;



	void ShowDialogueOptions(int32 DialoguePhaseIndexVar);

	void InitiateDialogue();

	void EndDialogue();

	int32 DialoguePhaseIndex = -1;

	void ProceedToNextDialogueInteraction(int32 ButtonClicked);

	void ProceedToSelectedBranch(int32 BranchID);

	void GameplaySpecialAction(int32 BranchID);

};
