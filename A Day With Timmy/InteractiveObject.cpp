// Fill out your copyright notice in the Description page of Project Settings.

#include "PointAndClick.h"
#include "InteractiveObject.h"
#include "PointAndClickGameMode.h"
#include "EngineUtils.h"
#include "PointAndClickPlayerController.h"


// Sets default values
AInteractiveObject::AInteractiveObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}


void AInteractiveObject::BeginPlay()
{
	Super::BeginPlay();
	
	APointAndClickPlayerController* TheController = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());

	MasterDialogueSystem = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->DialogueSystem;

	TArray<UStaticMeshComponent*> Comps;
	GetComponents<UStaticMeshComponent>(Comps);

	for (size_t i = 0; i < Comps.Num(); i++)
	{
		if (Comps[i]->ComponentHasTag("CharacterNode"))
		{
			CharacterPositionNode = Comps[i]->GetComponentLocation();
			Comps[i]->SetVisibility(false);
		}
	}
}

void AInteractiveObject::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AInteractiveObject::ShowDialogueOptions(int32 DialoguePhaseIndexVar)
{
	if (DialogueBanches[DialoguePhaseIndexVar].QuitChoiceID == -2)
	{
		MasterDialogueSystem->bWillEndDialogue = true;
	}
	else
	{
		if (DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts[0] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionAText = DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts[0];
		if (DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts.Num() >= 2)
			MasterDialogueSystem->MyLetterboxUIControl->OptionBText = DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts[1];
		if (DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts.Num() >= 3)
			MasterDialogueSystem->MyLetterboxUIControl->OptionCText = DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts[2];
		if (DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts.Num() >= 4)
			MasterDialogueSystem->MyLetterboxUIControl->OptionDText = DialogueBanches[DialoguePhaseIndexVar].ChoiceTexts[3];
	}
}

void AInteractiveObject::InitiateDialogue()
{
	Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->UpdateLetterboxUIVisibility(ESlateVisibility::Visible);
	


	// AJ
	if (DialogueTreeID == 0)
	{
		if (Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->CurrentItemEquipped == "Big Ol' Wrench" || Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->DoIHaveThisItem("Big Ol' Wrench") && Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("Has_Found_Powerbox"))
		{
			DialoguePhaseIndex = 5;
		}
		else if (Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->CurrentItemEquipped == "Big Ol' Wrench" || Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->DoIHaveThisItem("Big Ol' Wrench"))
		{
			DialoguePhaseIndex = 4;
		}
		else if (Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("Has_Found_Powerbox"))
		{
			DialoguePhaseIndex = 3;
		}
		else
		{
			DialoguePhaseIndex = InitialTreeIndex;
		}
	}
	else if (DialogueTreeID == 1)
	{
		if (Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->DoIHaveThisItem("Lost Package") && InitialTreeIndex == 1)
		{
			DialoguePhaseIndex = 2;
		}
		else
		{
			DialoguePhaseIndex = InitialTreeIndex;
		}
	}
	else if (DialogueTreeID == 2)
	{
		if (Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("Has_Finished_Roberto_Quest"))
		{
			DialoguePhaseIndex = 4;
		}
		else if (Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("HasWishedCorrectly"))
		{
			DialoguePhaseIndex = 6;

			Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("Has_Finished_Roberto_Quest");
		}
		else if (Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyInventoryUI->DoIHaveThisItem("Golden Coin"))
		{
			DialoguePhaseIndex = 4;
		}
		else
		{
			DialoguePhaseIndex = InitialTreeIndex;
		}
	}
	else if (DialogueTreeID == 3)
	{
		if (Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("ClueFound"))
		{
			DialoguePhaseIndex = 4;
		}
		else
		{
			DialoguePhaseIndex = InitialTreeIndex;
		}
	}
	else
	{
		DialoguePhaseIndex = InitialTreeIndex;
	}

	if (DialogueBanches[DialoguePhaseIndex].ThingsToSay.Num() == 0)
	{
		MasterDialogueSystem->MyLetterboxUIControl->TalkerName = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyCharacterName;
		MasterDialogueSystem->MyLetterboxUIControl->TalkerIcon = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyCharacterPortrait;

		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[0] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionAText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[0];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[1] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionBText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[1];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[2] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionCText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[2];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[3] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionDText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[3];
	}
	else
	{
		
		for (size_t i = 0; i < DialogueBanches[DialoguePhaseIndex].ThingsToSay.Num(); i++)
		{
			Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->EnqueueTextForTypewriter(
				DialogueBanches[DialoguePhaseIndex].ThingsToSay[i], 
				DialogueBanches[DialoguePhaseIndex].SoundsToPlay[i],
				DialogueBanches[DialoguePhaseIndex].Speaker[i]
				);
		}
		
	}
	
	if (DialogueBanches[DialoguePhaseIndex].QuitChoiceID == -2)
	{
		// Todo: Perform Exiting Branch.
		MasterDialogueSystem->bWillEndDialogue = true;
	}
}

void AInteractiveObject::EndDialogue()
{
	MasterDialogueSystem->LastInteractedActor = nullptr;
	
	Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->UpdateLetterboxUIVisibility(ESlateVisibility::Collapsed);
	Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->bIsInDialogueMode = false;
}

void AInteractiveObject::ProceedToNextDialogueInteraction(int32 ButtonClicked)
{
	if (DialogueBanches[DialoguePhaseIndex].QuitChoiceID == ButtonClicked || DialogueBanches[DialoguePhaseIndex].QuitChoiceID == -2)
	{
		// Todo: Perform Exiting Branch.
		MasterDialogueSystem->bWillEndDialogue = true;
	}
	
	DialoguePhaseIndex = DialogueBanches[DialoguePhaseIndex].ChoiceIDs[ButtonClicked];
	MasterDialogueSystem->MyLetterboxUIControl->OptionAText = "";
	MasterDialogueSystem->MyLetterboxUIControl->OptionBText = "";
	MasterDialogueSystem->MyLetterboxUIControl->OptionCText = "";
	MasterDialogueSystem->MyLetterboxUIControl->OptionDText = "";
	ProceedToSelectedBranch(DialoguePhaseIndex);
}

void AInteractiveObject::ProceedToSelectedBranch(int32 BranchID)
{
	if (DialogueBanches[DialoguePhaseIndex].QuitChoiceID == -2)
	{
		MasterDialogueSystem->bWillEndDialogue = true;
	}

	if (DialogueBanches[DialoguePhaseIndex].ThingsToSay.Num() == 0)
	{
		MasterDialogueSystem->MyLetterboxUIControl->TalkerName = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyCharacterName;
		MasterDialogueSystem->MyLetterboxUIControl->TalkerIcon = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->MyCharacterPortrait;

		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[0] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionAText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[0];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[1] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionBText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[1];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[2] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionCText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[2];
		if (DialogueBanches[DialoguePhaseIndex].ChoiceTexts[3] != "")
			MasterDialogueSystem->MyLetterboxUIControl->OptionDText = DialogueBanches[DialoguePhaseIndex].ChoiceTexts[3];
	}
	else
	{
		for (size_t i = 0; i < DialogueBanches[DialoguePhaseIndex].ThingsToSay.Num(); i++)
		{
			Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController())->EnqueueTextForTypewriter(
				DialogueBanches[DialoguePhaseIndex].ThingsToSay[i],
				DialogueBanches[DialoguePhaseIndex].SoundsToPlay[i],
				DialogueBanches[DialoguePhaseIndex].Speaker[i]);
		}
	}
}

void AInteractiveObject::GameplaySpecialAction(int32 BranchID)
{
	APointAndClickPlayerController* TheController = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
	if (TheController->bIsInDialogueMode)
	{
		// AJ's dialogue tree
		if (DialogueTreeID == 0)
		{
			if (BranchID == 1)
			{
				InitialTreeIndex = 2;
			}
			if (BranchID == 3)
			{
				/*TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();

				InitialTreeIndex = 4;

				APointAndClickPlayerController* TheController = Cast<APointAndClickPlayerController>(GetWorld()->GetFirstPlayerController());
				TheController->MyInventoryUI->AddItemByName("Severed Timmy Head");
				TheController->MyInventoryUI->AddItemByName("Knight's Hand");
				TheController->MyInventoryUI->AddItemByName("Cumshots");
				TheController->MyInventoryUI->AddItemByName("Rock Texture");*/
			}
			if (BranchID == 6)
			{
				//TheController->MyInventoryUI->RemoveItemByName("AdvancedHand");
			}
		}
		
		// Claire
		else if (DialogueTreeID == 1)
		{
			if (BranchID == 0)
			{
				InitialTreeIndex = 1;
			}
			if (BranchID == 2)
			{
				InitialTreeIndex = 3;

				TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();
				TheController->MyInventoryUI->RemoveItemByName("Lost Package");
				TheController->MyInventoryUI->AddItemByName("Lockbox");
			}
		}

		// Roberto
		else if (DialogueTreeID == 2)
		{
			if (BranchID == 1)
			{
				InitialTreeIndex = 7;
			}
			if (BranchID == 2)
			{
				TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();
				TheController->MyInventoryUI->AddItemByName("Golden Coin");

				InitialTreeIndex = 3;
			}
			else if (BranchID == 5)
			{
				TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();
				TheController->MyInventoryUI->AddItemByName("Golden Coin");
			}
			else if (BranchID == 6)
			{
				InitialTreeIndex = 4;
				Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("Has_Finished_Roberto_Quest");

				TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();
				TheController->MyInventoryUI->AddItemByName("Golden Key");
			}
		}

		// Well
		else if (DialogueTreeID == 3)
		{
			if (BranchID == 5)
			{
				TheController->MyInventoryUI->RemoveItemByName("Golden Coin");
				Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("HasWishedCorrectly");
			}
			if (BranchID == 2)
			{
				TheController->MyInventoryUI->RemoveItemByName("Golden Coin");
			}
			if (BranchID == 3)
			{
				TheController->MyInventoryUI->RemoveItemByName("Golden Coin");
			}
		}
		else if (DialogueTreeID == 4)
		{
			if (BranchID == 0)
			{
				Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("Has_Found_Powerbox");
				InitialTreeIndex = 1;
			}
			else if (BranchID == 2)
			{
				InitialTreeIndex = 3;
				TheController->MyInventoryUI->RemoveItemByName("Big Ol' Wrench");
				Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("TechDemoDone");

				// Stop Music;
				for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
				{
					if (ActorItr->ActorHasTag("FUNKY_MUSIC"))
					{
						ActorItr->FindComponentByClass<class UAudioComponent>()->Stop();
					}
				}

				TheController->MyInventoryUI->GameWon = true;
				TheController->MyInventoryUI->GameWon = true;
			}
		}
	}
	else
	{
		if (TheController->bLastInteractionWas_Interact)
		{
			// Sign Clue
			if (BranchID == 0 && MyName == "Mailbox")
			{
				TheController->GlobalAudioComponent->Sound = TheController->InventoryItemGained;
				TheController->GlobalAudioComponent->Play();
				TheController->MyInventoryUI->AddItemByName("Lost Package");

				InteractBranchID++;
			}

			TheController->bLastInteractionWas_Interact = false;
		}
		else if(TheController->bLastInteractionWas_Inspect)
		{
			// Sign Clue
			if (BranchID == 2 && MyName == "Sign")
			{
				if(!Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Contains("ClueFound"))
					Cast<APointAndClickGameMode>(GetWorld()->GetAuthGameMode())->EnabledFlags.Add("ClueFound");
			}

			if (BranchID == 0)InspectBranchID = 1;
			if (BranchID == 1)InspectBranchID = 2;
			if (BranchID == 2)InspectBranchID = 0;

			TheController->bLastInteractionWas_Inspect = false;
		}
	}
}


