// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCombat.h"
#include "CombatController.h"
#include "MonsterAIController.h"
#include "CustomMonsterActor.h"


	ACustomMonsterActor* MyMonster;

void AMonsterAIController::BeginPlay()
{

}


void AMonsterAIController::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult)
{
	Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->bHasFinishedPath = true;
}

void AMonsterAIController::DecideNextAction(APlayerController* &ControllerFromMaster)
{
	ACombatController* Con = Cast<ACombatController>(ControllerFromMaster);
	int32 RandomInt=0;
	FString Action = ""; //Local Variable that will define the action that the AI will take.
	MyMonster = Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]; // A Reference to the currently playing AI.
	bHasActionBeenDecidedYet = false;

	// First things first, the AI needs to think about the vital stuff, that will immediately affect winning or losing:
	// (A): Is Someone Dead? (B): Is Someone Low on HP? (C): Does my party fare well in terms of HP/MP?

	// Do I Have Missing HP?
	if (MyMonster->CurrentHealth < MyMonster->MaxHealth)
	{
		//How many HP exactly? If I'm over 50% gone, then It has high priority.
		float LostPercentage = (MyMonster->MaxHealth - MyMonster->CurrentHealth) / MyMonster->MaxHealth;
		if (LostPercentage >= 50)
		{
			KeepMyHPUpPriority(Con);
			if (bHasActionBeenDecidedYet)return;
		}
	}

	RandomInt = FMath::FRandRange(1, 100);
	if (MyMonster->AI_Archetype == EAIArchetypes::Healer)
	{
		PartyHPPriorityHigh(Con);
		if (bHasActionBeenDecidedYet)return;

		PartyHPPriorityLow(Con);
		if (bHasActionBeenDecidedYet)return;

		AnyoneHPPriority(Con);
		if (bHasActionBeenDecidedYet)return;

		SelfMPPriority(Con);
		if (bHasActionBeenDecidedYet)return;

		AnyoneMPPriority(Con);
		if (bHasActionBeenDecidedYet)return;
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Mage)
	{
		SelfMPPriority(Con);
		if (bHasActionBeenDecidedYet)return;

		if (RandomInt < 50)
		{
			PartyHPPriorityHigh(Con);
			if (bHasActionBeenDecidedYet)return;

			PartyHPPriorityLow(Con);
			if (bHasActionBeenDecidedYet)return;

			AnyoneHPPriority(Con);
			if (bHasActionBeenDecidedYet)return;	

			AnyoneMPPriority(Con);
			if (bHasActionBeenDecidedYet)return;
		}
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Warrior)
	{
		if (RandomInt < 30)
		{
			PartyHPPriorityHigh(Con);
			if (bHasActionBeenDecidedYet)return;

			PartyHPPriorityLow(Con);
			if (bHasActionBeenDecidedYet)return;

			AnyoneHPPriority(Con);
			if (bHasActionBeenDecidedYet)return;

			AnyoneMPPriority(Con);
			if (bHasActionBeenDecidedYet)return;
		}
	}

	

	// If we reach this place, then items are fully used up.

	// Before we delve deeper into attacking, we are checking out the defensive spells available.
	// Each monster is based on an AI Archetype, which favors either Physical Attacks, Spell Combat or Support (Items / Defensive Spells).
	// The archetype itself however is merely an indication as to what each monster decides to do.

	if (MyMonster->AI_Archetype == EAIArchetypes::Healer)
	{
		// This is an archetype of monsters that mainly use support abilities in battle.
		// First thing to check is whether or not they are OOM or capable to cast a spell.
		int32 SpellIDToCast = -1;
		SpellIDToCast = DecideForSpellToCast(Con);
		// Either -1 is returned (No Mana Left or No Need for Spell) or an index from 0 to N (Which Spell is selected).
		if (SpellIDToCast == -1)
		{
			// We don't have the mana required for the spell required. We definetly need mana. OR our spell may not be of any use now.
			// If we haven't received any mana by now (Which we should by the cases above) fallback to a minor action.
			// this happens outside this if .. else case.
		}
		else
		{
			// We do have the mana, and we do have a favored spell. So we cast that spell.
			// TODO: Cast Spell
		}
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Mage)
	{
		// This is an archetype of monsters that mainly use offensive abilities in battle.
		// First thing to check is whether or not they are OOM or capable to cast a spell.
		int32 SpellIDToCast = -1;
		SpellIDToCast = DecideForSpellToCast(Con);
		// Either -1 is returned (No Mana Left or No Need for Spell) or an index from 0 to N (Which Spell is selected).
		if (SpellIDToCast == -1)
		{
			// We don't have the mana required for the spell required. We definetly need mana. OR our spell may not be of any use now.
			// If we haven't received any mana by now (Which we should by the cases above) fallback to a minor action.
			// this happens outside this if .. else case.
		}
		else
		{
			// We do have the mana, and we do have a favored spell. So we cast that spell.
			// TODO: Cast Spell
		}
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Warrior)
	{
		// Warriors Favor Melee Attacks above all, since they are strong with them.
		// Targeting Enemies is decided based on which one has the lowest quantitative HP or the lowest armor, or random.
		// If our Melee Hero has low HP, then it's purely to chance that he will either guard, or attack (Depending if there is an enemy with low HP as well)
		// This is known as an attack of opportunity. It's also considered if the enemy is defending or not.

		// First Things First. If the warrior has an ability that is a Self-Buff, he uses it.

		// If not, then we proceed to check our HP.
		if ((MyMonster->MaxHealth - MyMonster->CurrentHealth / MyMonster->MaxHealth) >= 75)
		{
			// If our HP is low, we check all our enemies that are not currently defending, and are also low on hp to perform an attack of opportunity.
			TArray<int32> IndexesOfPossibleVictims;
			for (size_t i = 0; i < 3; i++)
			{
				if (!Con->PartyA[i]->bIsDead)
				{
					if (Con->PartyA[i]->bIsDefending)
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 90)
						{
							//Even if they defend, if they have less than 10%, we may have a chance.
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
					else
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 80)
						{
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
				}
			}

			//Prioritize the enemies based on:
			if (IndexesOfPossibleVictims.Num() > 0)
			{
				RandomInt = FMath::FRandRange(1, 100);
				if (RandomInt <= 40) // Lowest Quantitative Health.
				{
					int32 LowLimHP = Con->PartyA[IndexesOfPossibleVictims[0]]->CurrentHealth;
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth < LowLimHP)
						{
							LowLimHP = Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth;
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}

					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else if (RandomInt <= 75) // Lowest Defense
				{
					int32 LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[0]]->GetDefense();
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense() < LowLimDEF)
						{
							LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense();
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}
					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else // Random.
				{
					PerformAttack(Con, IndexesOfPossibleVictims[FMath::RandRange(0, IndexesOfPossibleVictims.Num() - 1)]);
					if (bHasActionBeenDecidedYet)return;
				}
			}
			else
			{
				// No monsters selected. Defend instead.

				// TODO: DEFEND.
				PerformDefend(Con);
				if (bHasActionBeenDecidedYet)return;
			}
		}
		else
		{
			// If our HP is NOT low, we attack based on the prioritization made above.
			TArray<int32> IndexesOfPossibleVictims;
			for (size_t i = 0; i < 3; i++)
			{
				if (!Con->PartyA[i]->bIsDead)
				{
					if (Con->PartyA[i]->bIsDefending)
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 80)
						{
							//Even if they defend, if they have less than 10%, we may have a chance.
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
					else
					{
						IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
					}
				}
			}

			//Prioritize the enemies based on:
			if (IndexesOfPossibleVictims.Num() > 0)
			{
				RandomInt = FMath::FRandRange(1, 100);
				if (RandomInt <= 40) // Lowest Quantitative Health.
				{
					int32 LowLimHP = Con->PartyA[IndexesOfPossibleVictims[0]]->CurrentHealth;
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth < LowLimHP)
						{
							LowLimHP = Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth;
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}

					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else if (RandomInt <= 75) // Lowest Defense
				{
					int32 LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[0]]->GetDefense();
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense() < LowLimDEF)
						{
							LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense();
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}
					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else // Random.
				{
					PerformAttack(Con, IndexesOfPossibleVictims[FMath::RandRange(0, IndexesOfPossibleVictims.Num() - 1)]);
					if (bHasActionBeenDecidedYet)return;
				}
			}
			else
			{
				// No monsters selected. Defend instead.
				PerformDefend(Con);
				if (bHasActionBeenDecidedYet)return;
			}
		}
	}


	// Fallback Cases for Mage or healer.
	if (MyMonster->AI_Archetype == EAIArchetypes::Mage || MyMonster->AI_Archetype == EAIArchetypes::Healer)
	{
		if ((MyMonster->MaxHealth - MyMonster->CurrentHealth / MyMonster->MaxHealth) >= 75)
		{
			// If our HP is low, we check all our enemies that are not currently defending, and are also low on hp to perform an attack of opportunity.
			TArray<int32> IndexesOfPossibleVictims;
			for (size_t i = 0; i < 3; i++)
			{
				if (!Con->PartyA[i]->bIsDead)
				{
					if (Con->PartyA[i]->bIsDefending)
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 90)
						{
							//Even if they defend, if they have less than 10%, we may have a chance.
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
					else
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 80)
						{
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
				}
			}

			//Prioritize the enemies based on:
			if (IndexesOfPossibleVictims.Num() > 0)
			{
				RandomInt = FMath::FRandRange(1, 100);
				if (RandomInt <= 40) // Lowest Quantitative Health.
				{
					int32 LowLimHP = Con->PartyA[IndexesOfPossibleVictims[0]]->CurrentHealth;
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth < LowLimHP)
						{
							LowLimHP = Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth;
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}

					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else if (RandomInt <= 75) // Lowest Defense
				{
					int32 LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[0]]->GetDefense();
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense() < LowLimDEF)
						{
							LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense();
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}
					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else // Random.
				{
					PerformAttack(Con, IndexesOfPossibleVictims[FMath::RandRange(0, IndexesOfPossibleVictims.Num() - 1)]);
					if (bHasActionBeenDecidedYet)return;
				}
			}
			else
			{
				// No monsters selected. Defend instead.

				// TODO: DEFEND.
				PerformDefend(Con);
				if (bHasActionBeenDecidedYet)return;
			}
		}
		else
		{
			// If our HP is NOT low, we attack based on the prioritization made above.
			TArray<int32> IndexesOfPossibleVictims;
			for (size_t i = 0; i < 3; i++)
			{
				if (!Con->PartyA[i]->bIsDead)
				{
					if (Con->PartyA[i]->bIsDefending)
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 80)
						{
							//Even if they defend, if they have less than 10%, we may have a chance.
							IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
						}
					}
					else
					{
						IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
					}
				}
			}

			//Prioritize the enemies based on:
			if (IndexesOfPossibleVictims.Num() > 0)
			{
				RandomInt = FMath::FRandRange(1, 100);
				if (RandomInt <= 40) // Lowest Quantitative Health.
				{
					int32 LowLimHP = Con->PartyA[IndexesOfPossibleVictims[0]]->CurrentHealth;
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth < LowLimHP)
						{
							LowLimHP = Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth;
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}

					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else if (RandomInt <= 75) // Lowest Defense
				{
					int32 LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[0]]->GetDefense();
					int32 IndexOfFinalTarget = 0;
					for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
					{
						if (Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense() < LowLimDEF)
						{
							LowLimDEF = Con->PartyA[IndexesOfPossibleVictims[i]]->GetDefense();
							IndexOfFinalTarget = IndexesOfPossibleVictims[i];
						}
					}
					PerformAttack(Con, IndexOfFinalTarget);
					if (bHasActionBeenDecidedYet)return;
				}
				else // Random.
				{
					PerformAttack(Con, IndexesOfPossibleVictims[FMath::RandRange(0, IndexesOfPossibleVictims.Num() - 1)]);
					if (bHasActionBeenDecidedYet)return;
				}
			}
			else
			{
				// No monsters selected. Defend instead.
				PerformDefend(Con);
				if (bHasActionBeenDecidedYet)return;
			}
		}
	}




	ResurrectMyTeamMatePriority(Con);
	if (bHasActionBeenDecidedYet)return;

	Con->EndTurn();
}

void AMonsterAIController::AnyoneMPPriority(ACombatController* &Con)
{
	TArray<int32> MonstersEligibleForMPItem;
	TArray<int32> QuantitiesLostForEligibleMonsters;

	for (size_t i = 0; i < Con->PartyB.Num(); i++)
	{
		if (Con->PartyB[i]->MonsterID != MyMonster->MonsterID)
		{
			if (!Con->PartyB[i]->bIsDead)
			{
				if (Con->PartyB[i]->CurrentMana < Con->PartyB[i]->MaxMana)
				{
					float LostPercentage = (Con->PartyB[i]->MaxMana - Con->PartyB[i]->CurrentMana) / Con->PartyB[i]->MaxMana;
					if (LostPercentage >= 40)
					{
						MonstersEligibleForMPItem.Add(i);
						QuantitiesLostForEligibleMonsters.Add(Con->PartyB[i]->MaxMana - Con->PartyB[i]->CurrentMana);
					}
				}
			}
		}
	}

	//First of all, make sure that there is indeed a conflict.
	if (MonstersEligibleForMPItem.Num() == 2)
	{
		//first, compare the 2 lost quantities.
		if (QuantitiesLostForEligibleMonsters[0] >= QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[MonstersEligibleForMPItem[0]]->MaxMana)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[0];
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[0]);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
		else if (QuantitiesLostForEligibleMonsters[0] < QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[1] <= Con->PartyB[MonstersEligibleForMPItem[1]]->MaxMana)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[1];
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[1]);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
	}
	else if (MonstersEligibleForMPItem.Num() == 1)
	{
		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Quantity>0)
			{
				if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[MonstersEligibleForMPItem[0]]->MaxMana)
				{
					Con->CurrentPhase = EActionPhasesType::Item;
					Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
					Con->ChangeCamera(MyMonster->MonsterID);
					Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[0];
					Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[0]);
					Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
					Con->MyActionHUD->bIsFadingOut = false;
					Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

					bHasActionBeenDecidedYet = true;

					return;
				}
			}
		}
	}
}

void AMonsterAIController::AnyoneHPPriority(ACombatController* &Con)
{
	TArray<int32> MonstersEligibleForHPItem;
	TArray<int32> PercetagesLostForEligibleMonsters;
	TArray<int32> QuantitiesLostForEligibleMonsters;

	for (size_t i = 0; i < Con->PartyB.Num(); i++)
	{
		if (Con->PartyB[i]->MonsterID != MyMonster->MonsterID)
		{
			if (!Con->PartyB[i]->bIsDead)
			{
				if (Con->PartyB[i]->CurrentHealth < Con->PartyB[i]->MaxHealth)
				{
					float LostPercentage = (Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth) / Con->PartyB[i]->MaxHealth;
					if (LostPercentage >= 40)
					{
						MonstersEligibleForHPItem.Add(i);
						PercetagesLostForEligibleMonsters.Add(LostPercentage);
						QuantitiesLostForEligibleMonsters.Add(Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth);
					}
				}
			}
		}
	}

	//First of all, make sure that there is indeed a conflict.
	if (MonstersEligibleForHPItem.Num() == 2)
	{
		//first, compare the 2 lost quantities.
		if (QuantitiesLostForEligibleMonsters[0] >= QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[MonstersEligibleForHPItem[0]]->MaxHealth)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[0];
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[0]);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
		else if (QuantitiesLostForEligibleMonsters[0] < QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[1] <= Con->PartyB[MonstersEligibleForHPItem[1]]->MaxHealth)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[1];
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[1]);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
	}
	else if (MonstersEligibleForHPItem.Num() == 1)
	{
		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Quantity>0)
			{
				if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[MonstersEligibleForHPItem[0]]->MaxHealth)
				{
					Con->CurrentPhase = EActionPhasesType::Item;
					Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
					Con->ChangeCamera(MyMonster->MonsterID);
					Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[0];
					Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[0]);
					Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
					Con->MyActionHUD->bIsFadingOut = false;
					Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

					bHasActionBeenDecidedYet = true;

					return;
				}
			}
		}
	}
}

void AMonsterAIController::PartyHPPriorityLow(ACombatController* &Con)
{
	// Winning Conditions are Important so I'll have to check about my teammates health.
	TArray<int32> MembersBelow60;
	for (size_t i = 0; i < Con->PartyB.Num(); i++)
	{
		if (!Con->PartyB[i]->bIsDead)
		{
			float LostPercentageParty = 0;
			LostPercentageParty = (Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth) / Con->PartyB[i]->MaxHealth;

			if (LostPercentageParty >= 60) MembersBelow60.Add(i);
		}
	}

	// 2 of them are below 60%! Is the third one damaged as well?
	if (MembersBelow60.Num() == 2)
	{
		for (size_t i = 0; i < Con->PartyB.Num(); i++)
		{
			if (!MembersBelow60.Contains(Con->PartyB[i]->MonsterID))
			{
				if (Con->PartyB[i]->CurrentHealth < Con->PartyB[i]->MaxHealth)
				{
					for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
					{
						if (Con->GroupBInventory.Items[i].Target == "All")
						{
							if (Con->GroupBInventory.Items[i].HP_Restored_Quantity > 0 || Con->GroupBInventory.Items[i].HP_Restored_Percentage >= 40)
							{
								if (Con->GroupBInventory.Items[i].Quantity>0)
								{
									Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Item;
									Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
									Con->MyActionHUD->CurrentTarget == "All";
									Con->MyActionHUD->CurrentMonsterID = -1;
									Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
									Con->CurrentItemPhase = EActionPhasesItem::GlobalItemUsageWait;
									Con->MyActionHUD->bIsFadingOut = false;
									Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
									Con->ChangeCamera(ECameras::Camera_Party_B);
									bHasActionBeenDecidedYet = true;

									return;
								}
							}
						}
					}
				}
			}
		}
	}
}

void AMonsterAIController::PartyHPPriorityHigh(ACombatController* &Con)
{
	// Winning Conditions are Important so I'll have to check about my teammates health.
	TArray<int32> MembersBelow30;
	for (size_t i = 0; i < Con->PartyB.Num(); i++)
	{
		if (!Con->PartyB[i]->bIsDead)
		{
			float LostPercentageParty = 0;
			LostPercentageParty = (Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth) / Con->PartyB[i]->MaxHealth;

			if (LostPercentageParty >= 30) MembersBelow30.Add(i);
		}
	}

	// 2 of them are below 25%! Ignore the third one!
	if (MembersBelow30.Num() == 2)
	{

		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Target == "All")
			{
				if (Con->GroupBInventory.Items[i].HP_Restored_Quantity > 0 || Con->GroupBInventory.Items[i].HP_Restored_Percentage > 0)
				{
					if (Con->GroupBInventory.Items[i].Quantity>0)
					{
						Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->MyActionHUD->CurrentTarget == "All";
						Con->MyActionHUD->CurrentMonsterID = -1;
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
						Con->CurrentItemPhase = EActionPhasesItem::GlobalItemUsageWait;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
						Con->ChangeCamera(ECameras::Camera_Party_B);
						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
	}
}

void AMonsterAIController::KeepMyHPUpPriority(ACombatController* &Con)
{
	for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
	{
		if (Con->GroupBInventory.Items[i].HP_Restored_Percentage >= (MyMonster->MaxHealth - MyMonster->CurrentHealth)/ MyMonster->MaxHealth ||
			Con->GroupBInventory.Items[i].HP_Restored_Quantity + MyMonster->CurrentHealth < MyMonster->MaxHealth)
		{
			if (Con->GroupBInventory.Items[i].Quantity > 0)
			{
				Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Item;
				Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
				Con->ChangeCamera(MyMonster->MonsterID);
				Con->MyActionHUD->CurrentMonsterID = MyMonster->MonsterID;
				Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
				Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
				Con->MyActionHUD->bIsFadingOut = false;
				Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

				bHasActionBeenDecidedYet = true;

				return;
			}
		}
	}
}

void AMonsterAIController::ResurrectMyTeamMatePriority(ACombatController* &Con)
{
	int32 IndexOfDeadMember = -1;
	int32 IndexOfRezItem = -1;

	//Question 1: Is Someone dead?
	for (size_t i = 0; i < 3; i++)
	{
		if (Con->PartyB[i]->bIsDead)
			IndexOfDeadMember = Con->PartyB[i]->MonsterID;
	}

	if (IndexOfDeadMember != -1)
	{
		//Question 2: Do I Have a Rez Item?
		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Resurrection)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					IndexOfRezItem = i;
				}
			}
		}
	}

	if (IndexOfRezItem != -1)
	{
		Cast<ACombatController>(GetWorld()->GetFirstPlayerController())->CurrentPhase = EActionPhasesType::Item;
		Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[IndexOfRezItem];
		Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
		Con->MyActionHUD->CurrentMonsterID = IndexOfDeadMember;
		Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(IndexOfDeadMember);
		Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
		Con->MyActionHUD->bIsFadingOut = false;
		Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
		bHasActionBeenDecidedYet = true;
	}
}

void AMonsterAIController::SelfMPPriority(ACombatController* &Con)
{
	if (MyMonster->CurrentMana < MyMonster->MaxMana)
	{
		float LostPercentage = (MyMonster->MaxMana - MyMonster->CurrentMana) / MyMonster->MaxMana;
		if (LostPercentage >= 40)
		{
			int32 QuantityLost = MyMonster->MaxMana - MyMonster->CurrentMana;
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					if (Con->GroupBInventory.Items[i].MP_Restored_Quantity <= QuantityLost)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MyMonster->MonsterID;
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
					else if(Con->GroupBInventory.Items[i].MP_Restored_Percentage <= LostPercentage)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MyMonster->MonsterID;
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
	}
}

int32 AMonsterAIController::DecideForSpellToCast(ACombatController* &Con)
{
	return -1;
}

void AMonsterAIController::PerformDefend(ACombatController* &Con)
{
	Con->CurrentPhase = EActionPhasesType::Defend;
	Con->CurrentDefendPhase = EActionPhasesDefend::DefendingParticle;
	Con->MyActionHUD->bIsFadingOut = false;
	Con->SetBottomTooltip(true, Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterName + " Defends! (+50% Defense until next turn!)");
	Con->MyActionHUD->CentralTooltip = "Defend";
	bHasActionBeenDecidedYet = true;
}

void AMonsterAIController::PerformAttack(ACombatController* &Con, int32 TargetID)
{
	Con->CurrentPhase = EActionPhasesType::Attack;
	Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
	Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(TargetID);
	Con->MyActionHUD->bIsFadingOut = false;
	Con->MyActionHUD->CurrentMonsterID = TargetID;
	Con->MyActionHUD->CentralTooltip = "Attack";
	Con->CurrentAttackPhase = EActionPhasesAttack::CameraOnActor;
	bHasActionBeenDecidedYet = true;
}