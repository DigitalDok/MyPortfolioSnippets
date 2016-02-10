
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialogue System by Kostas Dokos                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Components/ActorComponent.h"
#include "LetterboxDialogues.h"
#include "DialogueSystem.generated.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This enum is used to distinctify the characters among them, with text colour, name and portrait.//
/////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM()
enum EAdventureCharacterIDs
{
	Timmy,
	Jimbo,
	Claire,
	Roberto
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// The information that each character holds, used in our dialogue system (Portrait | Color | Name)//
/////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FAdventureCharacterStruct
{
	GENERATED_USTRUCT_BODY()

	FAdventureCharacterStruct()
	{

	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* CharacterPortrait = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSlateColor CharacterTextColor;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The main backbone of our Dialogue System.																   //
// 																											   //
// Each Character holds a number of "Dialogue Branches" on him/her.											   //
// Each branch represents a single queue of dialogues among the character and our character.				   //
// 																											   //
// After all dialogue has been exhausted, there is always the choice of possible actions, which appear.		   //
// Depending on the selection of the player, the dialogue proceeds to the selected branch ID.				   //
// 																											   //
// If QuitChoice ID is equal to -1 nothing happens, however if it is equal to a specific branch ID,			   //
// then if we select that specific branch to proceed, it's guarranteed that the dialogue will end.			   //
//																											   //
// Alternatively, if the QuitChoiceID is equal to -2 the dialogue ends no matter what at the end of the queue. //
//																											   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FDialogBranch
{
	GENERATED_USTRUCT_BODY()


	FDialogBranch()
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 BranchID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 QuitChoiceID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ThingsToSay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<USoundWave*> SoundsToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TEnumAsByte<EAdventureCharacterIDs>> Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> ChoiceIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ChoiceTexts;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POINTANDCLICK_API UDialogueSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDialogueSystem();
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	// Special Property, used only for the Player Character, for common expressions such as "This doesn't work" or "I can't do that"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
		TArray<FDialogBranch> DialogueBanches;

	// ****************************************************************************************************
	
	#pragma region Dialogue System Core Structure

	UPROPERTY()
	UTextBlock* SubtitleTextBlock = nullptr;
	
	UPROPERTY()
	UAudioComponent* ReferencedAudioComp = nullptr;
	
	TQueue<FString> DialogueQueue;
	
	TQueue<USoundWave*> SfxQueue;
	
	TQueue<FAdventureCharacterStruct> CharsQueue;

	#pragma endregion

	// ****************************************************************************************************

	#pragma region Text/VoiceOver Logic & Typewriter Effects

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	bool bIsBusy;

	#pragma region Voiceover

	bool bAudioHasCompletedPlayback;

	TScriptDelegate<FWeakObjectPtr> AudioFinishDelegate;

	UFUNCTION()
	void Finished();

	#pragma endregion

	FString InputString;
	FString OutputString;

	UPROPERTY(EditAnywhere, Category = "Typewriting Effects")
		float TypingTimer;

	UPROPERTY(EditAnywhere, Category = "Typewriting Effects")
		float TypingInterval;

	UPROPERTY(EditAnywhere, Category = "Typewriting Effects")
		float FullTextTimer;

	UPROPERTY(EditAnywhere, Category = "Typewriting Effects")
		float FullTextLifetime;

	#pragma endregion
	
	// ****************************************************************************************************

	#pragma region Branching Control & Dialogue Functionality

	UPROPERTY()
	ULetterboxDialogues* MyLetterboxUIControl = nullptr;
	
	UPROPERTY()
	AActor* LastInteractedActor = nullptr;

	bool bWillEndDialogue;
	float DialogueDeathTimer = 0;

	void InitializeTypewriter(UTextBlock* &GlobalTextBlock, UAudioComponent* &AudioComp, ULetterboxDialogues* &Letterbox);
	void CheckStuffForDialogueMode();
	void SkipToNextDialoguePart();

	#pragma endregion

	// ****************************************************************************************************
};
