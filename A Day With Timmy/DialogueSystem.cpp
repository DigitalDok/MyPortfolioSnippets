
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialogue System by Kostas Dokos                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PointAndClick.h"
#include "DialogueSystem.h"
#include "PointAndClickPlayerController.h"
#include "InteractiveObject.h"

APointAndClickPlayerController* TheController;
AInteractiveObject* LastAccessedInteractiveObject;

UDialogueSystem::UDialogueSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	AudioFinishDelegate.BindUFunction(this, "Finished");
}

void UDialogueSystem::BeginPlay()
{
	Super::BeginPlay();

	AudioFinishDelegate.BindUFunction(this, "Finished");
	TheController = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
	LastAccessedInteractiveObject = nullptr;
}

// The main Logic of our Dialogue System.
void UDialogueSystem::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// If Dialogue has ended, give it some extra time.
	if (DialogueDeathTimer > 0)
	{
		DialogueDeathTimer -= DeltaTime;
		if (DialogueDeathTimer <= 0)
		{
			LastAccessedInteractiveObject->EndDialogue();
		}
	}

	// Cleaning up references, should the dialogue end.
	if (FullTextTimer > 0)
	{
		FullTextTimer -= DeltaTime;
		if (FullTextTimer <= 0)
		{
			
			if (TheController->bIsInDialogueMode)
			{
				MyLetterboxUIControl->CurrentText = "";
			}
			else
			{
				SubtitleTextBlock->SetText(FText::FromString(""));
			}
			OutputString = "";
		}
	}

	// If our Dialogue Queue is not empty...
	else if (!DialogueQueue.IsEmpty())
	{
		// ... and we are not busy, it means it's time to work!
		if (!bIsBusy)
		{
			// Grab the first item of the queue, start typing and play the voiceover!

			FString X;
			DialogueQueue.Peek(X);
			InputString = X;
			bIsBusy = true;

			if (TheController->bIsInDialogueMode)
			{
				MyLetterboxUIControl->CurrentText = "";
			}
			else
			{
				SubtitleTextBlock->SetText(FText::FromString(""));
			}
			
			OutputString = "";


			bAudioHasCompletedPlayback = false;
			USoundWave* Y;
			SfxQueue.Peek(Y);
			ReferencedAudioComp->Sound = Y;
			ReferencedAudioComp->Play();

			if (TheController->bIsInDialogueMode)
			{
				FAdventureCharacterStruct Z;
				CharsQueue.Peek(Z);
				MyLetterboxUIControl->TalkerIcon = Z.CharacterPortrait;
				MyLetterboxUIControl->CharacterText = Z.CharacterTextColor;
				MyLetterboxUIControl->TalkerName = Z.CharacterName;

				if (Z.CharacterName == "Timmy") TheController->MyAnimInstance->Talk();

				if (Cast<AInteractiveObject>(LastInteractedActor))
				{
					LastAccessedInteractiveObject = Cast<AInteractiveObject>(LastInteractedActor);
					TheController->SetViewTarget(LastAccessedInteractiveObject->Cameras[FMath::RandRange(0, LastAccessedInteractiveObject->Cameras.Num()-1)]);
				}
			}
			else
			{
				SubtitleTextBlock->SetColorAndOpacity(TheController->WorldCharacters[0].CharacterTextColor);
			}
		}
	}

	// Being busy means that the typewriter is working, outputting chars on screen, one at a time.
	if (bIsBusy)
	{
		TypingTimer += DeltaTime;
		
		// On Intervals...
		if (TypingTimer > TypingInterval)
		{
			TypingTimer = 0;

			// If there is still some text to write on screen...
			if (InputString.Len() > 0)
			{
				//... Add it to the output string, removing it from the input string, one char at a time.
				// Make sure to also display it in the UI.

				OutputString.Append(InputString.Mid(0, 1));
				if (TheController->bIsInDialogueMode)
				{
					MyLetterboxUIControl->CurrentText = OutputString;
				}
				else
				{
					SubtitleTextBlock->SetText(FText::FromString(OutputString));
				}

				InputString.RemoveAt(0, 1);
			}

			// However, if there is no more text to write AND the voiceover has stopped playing...
			else if (InputString == "" && bAudioHasCompletedPlayback)
			{
				// Cleaning up, removing references, wrapping up and ending dialogue to dequeue stuff from the Text/Voiceover buffer.

				bAudioHasCompletedPlayback = false;
				FullTextTimer = FullTextLifetime;

				FString KilledText;
				DialogueQueue.Dequeue(KilledText);

				USoundWave* KilledSfx;
				SfxQueue.Dequeue(KilledSfx);

				bIsBusy = false;

				CheckStuffForDialogueMode();
				
			}
		}
	}
}

void UDialogueSystem::InitializeTypewriter(UTextBlock *& GlobalTextBlock, UAudioComponent* &AudioComp, ULetterboxDialogues* &Letterbox)
{
	SubtitleTextBlock = GlobalTextBlock;
	ReferencedAudioComp = AudioComp;
	MyLetterboxUIControl = Letterbox;
	
	ReferencedAudioComp->OnAudioFinished.Add(AudioFinishDelegate);
}

void UDialogueSystem::SkipToNextDialoguePart()
{
	TypingTimer = 0;
	InputString = "";
	FullTextTimer = 0.1f;

	ReferencedAudioComp->Stop();

	FString KilledText;
	DialogueQueue.Dequeue(KilledText);
	USoundWave* KilledSfx;
	SfxQueue.Dequeue(KilledSfx);

	if (TheController->bIsInDialogueMode)
	{
		FAdventureCharacterStruct DeadChar;
		CharsQueue.Dequeue(DeadChar);
	}

	bIsBusy = false;
	CheckStuffForDialogueMode();
}

void UDialogueSystem::Finished()
{
	bAudioHasCompletedPlayback = true;
}

void UDialogueSystem::CheckStuffForDialogueMode()
{
	if (TheController->bIsInDialogueMode)
	{
		FAdventureCharacterStruct DeadChar;
		CharsQueue.Dequeue(DeadChar);
	}

	if (DialogueQueue.IsEmpty())
	{
		if (Cast<AInteractiveObject>(LastInteractedActor))
		{
			LastAccessedInteractiveObject = Cast<AInteractiveObject>(LastInteractedActor);

			if (TheController->bIsInDialogueMode)
			{
				LastAccessedInteractiveObject->GameplaySpecialAction(LastAccessedInteractiveObject->DialoguePhaseIndex);
				if (bWillEndDialogue)
				{
					bWillEndDialogue = false;
					DialogueDeathTimer = 1;
				}
				else
				{
					LastAccessedInteractiveObject->ShowDialogueOptions(LastAccessedInteractiveObject->DialoguePhaseIndex);
				}
			}
			else
			{
				if (TheController->bLastInteractionWas_Inspect)
				{
					LastAccessedInteractiveObject->GameplaySpecialAction(LastAccessedInteractiveObject->InspectBranchID);
				}
				else if (TheController->bLastInteractionWas_Interact)
				{
					LastAccessedInteractiveObject->GameplaySpecialAction(LastAccessedInteractiveObject->InteractBranchID);
				}
			}

			
		}
	}
}


