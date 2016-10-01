// Fill out your copyright notice in the Description page of Project Settings.

#include "EQS_Demo.h"
#include "MasterBot.h"
#include "EngineUtils.h"

// Sets default values for this character's properties
AMasterBot::AMasterBot()
{

}

// Called when the game starts or when spawned
void AMasterBot::BeginPlay()
{
	BlueTeamMorale = 1.f;
	RedTeamMorale = 1.f;

	GetAllTeams();
}

void AMasterBot::GetAllTeams()
{
	TArray<AAI_Bot*> Bots;
	for (TActorIterator<AAI_Bot> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if ((*ActorItr)->bIsRedTeam)
		{
			RedTeam.Add(*ActorItr);

			switch ((*ActorItr)->MyArchetype)
			{
			case EClassArchetypes::Berserker:
				RedTeam_Berserkers.Add(*ActorItr);
				break;

			case EClassArchetypes::Warrior:
				RedTeam_Warriors.Add(*ActorItr);
				break;

			case EClassArchetypes::Tactician:
				RedTeam_Tacticians.Add(*ActorItr);
				break;

			case EClassArchetypes::Guard:
				RedTeam_Guards.Add(*ActorItr);
				break;

			}


		}
		else
		{
			BlueTeam.Add(*ActorItr);

			switch ((*ActorItr)->MyArchetype)
			{
			case EClassArchetypes::Berserker:
				BlueTeam_Berserkers.Add(*ActorItr);
				break;

			case EClassArchetypes::Warrior:
				BlueTeam_Warriors.Add(*ActorItr);
				break;

			case EClassArchetypes::Tactician:
				BlueTeam_Tacticians.Add(*ActorItr);
				break;

			case EClassArchetypes::Guard:
				BlueTeam_Guards.Add(*ActorItr);
				break;
			}
		}

		AllBots.Add(*ActorItr);
	}
}

void AMasterBot::LastStand(bool bTeamRed)
{
	if (bTeamRed)
	{
		int Deadppl = 0;
		for (size_t i = 0; i < RedTeam.Num(); i++)
		{
			if (RedTeam[i]->CurrentHealth <= 0)Deadppl++;
		}

		if (Deadppl == RedTeam.Num() - 1)
		{
			for (size_t i = 0; i < RedTeam.Num(); i++)
			{
				if (RedTeam[i]->CurrentHealth >= 0)
				{
					RedTeam[i]->MaxHealth *= 2;
					RedTeam[i]->CurrentHealth = RedTeam[i]->MaxHealth;
					RedTeam[i]->Master->BB->SetValueAsBool(TEXT("Feared"), false);
					RedTeam[i]->Fearless = 101;
				}
			}
		}
	}
	else
	{
		int Deadppl = 0;
		for (size_t i = 0; i < BlueTeam.Num(); i++)
		{
			if (BlueTeam[i]->CurrentHealth <= 0)Deadppl++;
		}

		if (Deadppl == BlueTeam.Num() - 1)
		{
			for (size_t i = 0; i < BlueTeam.Num(); i++)
			{
				if (BlueTeam[i]->CurrentHealth >= 0)
				{
					BlueTeam[i]->MaxHealth *= 2;
					BlueTeam[i]->CurrentHealth = BlueTeam[i]->MaxHealth;
					BlueTeam[i]->Master->BB->SetValueAsBool(TEXT("Feared"), false);
					BlueTeam[i]->Fearless = 101;
				}
			}
		}
	}
}

void AMasterBot::ChangeMorale(bool bTeamRed, float Val)
{
	if (!MoraleSystemOn)return;

	if (bTeamRed)
	{
		RedTeamMorale += Val;
		BlueTeamMorale -= Val;
	}
	else
	{

		BlueTeamMorale += Val;
		RedTeamMorale -= Val;
	}

	if (BlueTeamMorale <= 0.1f)
		BlueTeamMorale = 0.1f;

	if (RedTeamMorale > 2)
	{
		RedTeamMorale = 2;
		BlueTeamMorale = 0.1f;
	}

	if (BlueTeamMorale > 2)
	{
		BlueTeamMorale = 2;
		RedTeamMorale = 0.1f;
	}

	if (RedTeamMorale <= 0.1f)
		RedTeamMorale = 0.1f;

	for (size_t i = 0; i < BlueTeam.Num(); i++)
	{
		BlueTeam[i]->GetCharacterMovement()->MaxWalkSpeed = (350 + BlueTeam[i]->Mobility) * BlueTeamMorale;
	}
	for (size_t i = 0; i < RedTeam.Num(); i++)
	{
		RedTeam[i]->GetCharacterMovement()->MaxWalkSpeed = (350 + RedTeam[i]->Mobility) * RedTeamMorale;
	}
}