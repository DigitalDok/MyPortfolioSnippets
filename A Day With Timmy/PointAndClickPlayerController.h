
#pragma once
#include "GameFramework/PlayerController.h"
#include "DialogueSystem.h"
#include "InteractiveObject.h"
#include "CharacterAnimInstance.h"
#include "LetterboxDialogues.h"
#include "InventoryUI.h"
#include "PointAndClickPlayerController.generated.h"


UCLASS()
class APointAndClickPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	#pragma region Core Functions

		APointAndClickPlayerController();
		virtual void PlayerTick(float DeltaTime) override;
		virtual void SetupInputComponent() override;
		virtual void BeginPlay() override;

	#pragma endregion
	
	#pragma region Visual Helper - Animation Instance - Cursor 

		FVector LastClickedLocation;

		UPROPERTY()
			UStaticMeshComponent* VisualHelper = nullptr;

		UPROPERTY()
			AActor* CursorMan = nullptr;

		UPROPERTY()
			UCharacterAnimInstance* MyAnimInstance = nullptr;

	#pragma endregion

	#pragma region Interaction System

		FVector InteractiveObjectLocation = FVector(0);
	
		bool bTravellingToInteractiveObject;
	
		float InteractionRange;

		UFUNCTION()
			AInteractiveObject* GetInteractiveHoveredActor();

		UPROPERTY()
			AInteractiveObject* LastInteractedActor = nullptr;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Helper")
			FString CurrentObjectName;

		bool bLastInteractionWas_Interact;
		bool bLastInteractionWas_Inspect;

	#pragma endregion

	#pragma region Navigation


		/** Navigate player to the current mouse cursor location. */
		void MoveToMouseCursor();
		/** Navigate player to the current touch location. */
		void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
		/** Navigate player to the given world location. */
		void SetNewMoveDestination(const FVector DestLocation);
		void SetNewMoveDestination(const AActor* TargetActor);

		/** Input handlers for SetDestination action. */
		void OnSetDestinationPressed();
		void OnSetDestinationReleased();

		/** True if the controlled character should navigate to the mouse cursor. */
		uint32 bMoveToMouseCursor : 1;

	#pragma endregion

	#pragma region Input

		void RMB();

		UFUNCTION(BlueprintCallable, Category="UI Helper")
		void LMB();

	#pragma endregion

	#pragma region User Interface PRoperties & Functions


		UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
			TSubclassOf<class UUserWidget> UI_Holder;
		UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
			TSubclassOf<class UUserWidget> UI_Holder2;

		UPROPERTY()
			ULetterboxDialogues* LetterboxUI = nullptr;

		UPROPERTY()
			UInventoryUI* MyInventoryUI = nullptr;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
			UDialogueSystem* DialogueSystem = nullptr;

		UFUNCTION()
			ULetterboxDialogues* GetLetterboxUI();

		UPROPERTY()
		UTextBlock* MyCursorTextBlockControl = nullptr;
	
		UPROPERTY()
		UTextBlock* MySubtitleTextBlockControl = nullptr;

		UPROPERTY()
			UUserWidget* TooltipUI = nullptr;

	#pragma endregion

	#pragma region Character Info

		UPROPERTY(EditAnywhere, Category = "Character Information")
		FString MyCharacterName;

		UPROPERTY(EditAnywhere, Category = "Character Information")
		UTexture2D* MyCharacterPortrait = nullptr;

		UPROPERTY(EditAnywhere, Category = "Character Information")
			TArray<FAdventureCharacterStruct> WorldCharacters;
	
	#pragma endregion

	#pragma region Audio System

		UPROPERTY()
		UAudioComponent* GlobalAudioComponent = nullptr;

		UPROPERTY(EditAnywhere, Category = "Global Audio Effects")
			USoundWave* InventoryItemGained;

		UPROPERTY(EditAnywhere, Category = "Global Audio Effects")
			USoundWave* UIClick;

		UPROPERTY(EditAnywhere, Category = "Global Audio Effects")
			USoundWave* CombinationSuccess;

	#pragma endregion

	#pragma region Inventory Management

		UPROPERTY(EditAnywhere, Category = "Inventory Information")
			TArray<FInventoryItem> ItemPool;

	#pragma endregion

	#pragma region Typewriter Effects


		UFUNCTION(BlueprintCallable,Category="Typewriter")
			void EnqueueTextForTypewriter(FString Text, USoundWave* Sfx);
	
		void EnqueueTextForTypewriter(FString Text, USoundWave* Sfx, EAdventureCharacterIDs Character);

		void UpdateLetterboxUIVisibility(ESlateVisibility NewVisibility);

		bool bIsInDialogueMode;
	
	#pragma endregion

	#pragma region Gameplay & Helpers

		void Special_Item_Checker(FString Item);

		void MainInteraction();

		void CharacterAndAnimationPointers();

		void CursorFunctionality();

		FRotator FindLookAtRotation(FVector Start, FVector Target);

	#pragma endregion

};


