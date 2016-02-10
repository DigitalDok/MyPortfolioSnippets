
/////////////////////////////////////////////////////////////////////////
// Monster AI (Turn Based RPG) - By Kostas Dokos					   //
/////////////////////////////////////////////////////////////////////////

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
	int32 RandomInt = 0;
	FString Action = ""; //Local Variable that will define the action that the AI will take.
	MyMonster = Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]; // A Reference to the currently playing AI.
	bHasActionBeenDecidedYet = false;

	// If I Am a Healer, then maybe a heal spell is in order before we attempt to look for items.
	Perform_HP_Spell(Con);
	if (bHasActionBeenDecidedYet)return;

	ResurrectMyTeamMatePriority(Con);
	if (bHasActionBeenDecidedYet)return;

	if (MyMonster->CurrentHealth < MyMonster->MaxHealth)
	{
		//How many HP exactly? If I'm over 50% gone, then It has high priority.
		float LostPercentage = (MyMonster->MaxHealth - MyMonster->CurrentHealth) / MyMonster->MaxHealth;
		if (LostPercentage >= 0.5f)
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

		CastSelfBuff(Con);
		if (bHasActionBeenDecidedYet)return;

		DecideForSpellToCast(Con);
		if (bHasActionBeenDecidedYet)return;
		
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Mage)
	{
		// This is an archetype of monsters that mainly use offensive abilities in battle.

		CastSelfBuff(Con);
		if (bHasActionBeenDecidedYet)return;

		DecideForSpellToCast(Con);
		if (bHasActionBeenDecidedYet)return;
		
	}
	else if (MyMonster->AI_Archetype == EAIArchetypes::Warrior)
	{
		// Warriors Favor Melee Attacks above all, since they are strong with them.
		// Targeting Enemies is decided based on which one has the lowest quantitative HP or the lowest armor, or random.
		// If our Melee Hero has low HP, then it's purely to chance that he will either guard, or attack (Depending if there is an enemy with low HP as well)
		// This is known as an attack of opportunity. It's also considered if the enemy is defending or not.

		PhysicalAttackPriority(Con);
		if (bHasActionBeenDecidedYet)return;
	}


	// Fallback Cases for Mage or healer.
	if (MyMonster->AI_Archetype == EAIArchetypes::Mage || MyMonster->AI_Archetype == EAIArchetypes::Healer || MyMonster->AI_Archetype == EAIArchetypes::Warrior)
	{
		PhysicalAttackPriority(Con);
		if (bHasActionBeenDecidedYet)return;
	}

	Con->EndTurn();
}



void AMonsterAIController::PhysicalAttackPriority(ACombatController* &Con)
{
	int32 RandomInt = 0;

	// First Things First. If the warrior has an ability that is a Self-Buff, he uses it.
	if (MyMonster->AI_Archetype == EAIArchetypes::Warrior)
	{
		CastSelfBuff(Con);
		if (bHasActionBeenDecidedYet)return;
	}

	// If not, then we proceed to check our HP.
	if ((MyMonster->MaxHealth - MyMonster->CurrentHealth / MyMonster->MaxHealth) >= 0.75f)
	{
		// If our HP is low, we check all our enemies that are not currently defending, and are also low on hp to perform an attack of opportunity.
		TArray<int32> IndexesOfPossibleVictims;
		for (size_t i = 0; i < 3; i++)
		{
			if (!Con->PartyA[i]->bIsDead)
			{
				if (Con->PartyA[i]->bIsDefending)
				{
					if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 0.9f)
					{
						//Even if they defend, if they have less than 10%, we may have a chance.
						IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
					}
				}
				else
				{
					if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 0.8f)
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
					for (size_t j = 0; j < Con->PartyA.Num(); j++)
					{
						if (Con->PartyA[j]->MonsterID == IndexesOfPossibleVictims[i])
						{
							if (Con->PartyA[j]->CurrentHealth < LowLimHP)
							{
								LowLimHP = Con->PartyA[j]->CurrentHealth;
								IndexOfFinalTarget = IndexesOfPossibleVictims[i];
							}
						}
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
					for (size_t j = 0; j < Con->PartyA.Num(); j++)
					{
						if (Con->PartyA[j]->MonsterID == IndexesOfPossibleVictims[i])
						{
							if (Con->PartyA[j]->GetDefense() < LowLimDEF)
							{
								LowLimDEF = Con->PartyA[j]->GetDefense();
								IndexOfFinalTarget = IndexesOfPossibleVictims[i];
							}
						}
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
	else
	{
		// If our HP is NOT low, we attack based on the prioritization made above.
		RandomInt = FMath::FRandRange(1, 100);
		if (RandomInt < 80 || MyMonster->AI_Archetype != EAIArchetypes::Warrior)
		{
			TArray<int32> IndexesOfPossibleVictims;
			for (size_t i = 0; i < 3; i++)
			{
				if (!Con->PartyA[i]->bIsDead)
				{
					if (Con->PartyA[i]->bIsDefending)
					{
						if ((Con->PartyA[i]->MaxHealth - Con->PartyA[i]->CurrentHealth / Con->PartyA[i]->MaxHealth) >= 0.8f)
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
						for (size_t j = 0; j < Con->PartyA.Num(); j++)
						{
							if (Con->PartyA[j]->MonsterID == IndexesOfPossibleVictims[i])
							{
								if (Con->PartyA[j]->CurrentHealth < LowLimHP)
								{
									LowLimHP = Con->PartyA[j]->CurrentHealth;
									IndexOfFinalTarget = IndexesOfPossibleVictims[i];
								}
							}
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
						for (size_t j = 0; j < Con->PartyA.Num(); j++)
						{
							if (Con->PartyA[j]->MonsterID == IndexesOfPossibleVictims[i])
							{
								if (Con->PartyA[j]->GetDefense() < LowLimDEF)
								{
									LowLimDEF = Con->PartyA[j]->GetDefense();
									IndexOfFinalTarget = IndexesOfPossibleVictims[i];
								}
							}
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
		else
		{
			DecideForSpellToCast(Con);
			if (bHasActionBeenDecidedYet)return;
		}
	}
}

void AMonsterAIController::CastSelfBuff(ACombatController* &Con)
{
	for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
	{
		if (MyMonster->Spells[i].bIsSelfBuff)
		{
			bool IsSpellEnabled = false;
			for (size_t j = 0; j < MyMonster->ActiveBuffs.Num(); j++)
			{
				if (MyMonster->ActiveBuffs[j].OriginatingSpellName == MyMonster->Spells[i].Name)
				{
					IsSpellEnabled = true;
				}
			}

			if (!IsSpellEnabled)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana && MyMonster->Spells[i].Target == "Self")
				{
					Con->CurrentPhase = EActionPhasesType::Ability;
					Con->MyActionHUD->LatestAbility = MyMonster->Spells[i];
					Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
					Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
					Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
					Con->MyActionHUD->bIsFadingOut = false;
					Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));

					Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;
					bHasActionBeenDecidedYet = true;
				}
			}
		}
	}
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
					if (LostPercentage >= 0.4f)
					{
						MonstersEligibleForMPItem.Add(Con->PartyB[i]->MonsterID);
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
				if (Con->GroupBInventory.Items[i].Target == "Single")
				{
					if (Con->GroupBInventory.Items[i].Quantity>0)
					{
						for (size_t j = 0; j < Con->PartyB.Num(); j++)
						{
							if (Con->PartyB[j]->MonsterID == MonstersEligibleForMPItem[0])
							{
								if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[j]->MaxMana)
								{
									Con->CurrentPhase = EActionPhasesType::Item;
									Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
									Con->ChangeCamera(MyMonster->MonsterID);
									Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[0];
									Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[0]);
									Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
									Con->MyActionHUD->bIsFadingOut = false;
									Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
									Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
									bHasActionBeenDecidedYet = true;

									return;
								}
							}
						}
					}
				}
			}
		}
		else if (QuantitiesLostForEligibleMonsters[0] < QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Target == "Single")
				{
					if (Con->GroupBInventory.Items[i].Quantity>0)
					{
						for (size_t j = 0; j < Con->PartyB.Num(); j++)
						{
							if (Con->PartyB[j]->MonsterID == MonstersEligibleForMPItem[1])
							{
								if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[1] <= Con->PartyB[j]->MaxMana)
								{
									Con->CurrentPhase = EActionPhasesType::Item;
									Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
									Con->ChangeCamera(MyMonster->MonsterID);
									Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[1];
									Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[1]);
									Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
									Con->MyActionHUD->bIsFadingOut = false;
									Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
									Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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
	else if (MonstersEligibleForMPItem.Num() == 1)
	{
		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Target == "Single")
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					for (size_t j = 0; j < Con->PartyB.Num(); j++)
					{
						if (Con->PartyB[j]->MonsterID == MonstersEligibleForMPItem[0])
						{
							if (Con->GroupBInventory.Items[i].MP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[j]->MaxMana)
							{
								Con->CurrentPhase = EActionPhasesType::Item;
								Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
								Con->ChangeCamera(MyMonster->MonsterID);
								Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForMPItem[0];
								Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForMPItem[0]);
								Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
								Con->MyActionHUD->bIsFadingOut = false;
								Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
								Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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
					if (LostPercentage >= 0.4f)
					{
						MonstersEligibleForHPItem.Add(Con->PartyB[i]->MonsterID);
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
				if(Con->GroupBInventory.Items[i].Target == "Single")
				{
					if (Con->GroupBInventory.Items[i].Quantity>0)
					{
						for (size_t j = 0; j < Con->PartyB.Num(); j++)
						{
							if (Con->PartyB[j]->MonsterID == MonstersEligibleForHPItem[0])
							{
								if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[j]->MaxHealth)
								{
									Con->CurrentPhase = EActionPhasesType::Item;
									Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
									Con->ChangeCamera(MyMonster->MonsterID);
									Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[0];
									Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[0]);
									Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
									Con->MyActionHUD->bIsFadingOut = false;
									Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
									Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
									bHasActionBeenDecidedYet = true;

									return;
								}
							}
						}
					}
				}
			}
		}
		else if (QuantitiesLostForEligibleMonsters[0] < QuantitiesLostForEligibleMonsters[1])
		{
			for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
			{
				if (Con->GroupBInventory.Items[i].Target == "Single")
				{
					if (Con->GroupBInventory.Items[i].Quantity>0)
					{
						for (size_t j = 0; j < Con->PartyB.Num(); j++)
						{
							if (Con->PartyB[j]->MonsterID == MonstersEligibleForHPItem[1])
							{
								if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[1] <= Con->PartyB[j]->MaxHealth)
								{
									Con->CurrentPhase = EActionPhasesType::Item;
									Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
									Con->ChangeCamera(MyMonster->MonsterID);
									Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[1];
									Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[1]);
									Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
									Con->MyActionHUD->bIsFadingOut = false;
									Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
									Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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
	else if (MonstersEligibleForHPItem.Num() == 1)
	{
		for (size_t i = 0; i < Con->GroupBInventory.Items.Num(); i++)
		{
			if (Con->GroupBInventory.Items[i].Target == "Single")
			{
				if (Con->GroupBInventory.Items[i].Quantity>0)
				{
					for (size_t j = 0; j < Con->PartyB.Num(); j++)
					{
						if (Con->PartyB[j]->MonsterID == MonstersEligibleForHPItem[0])
						{
							if (Con->GroupBInventory.Items[i].HP_Restored_Quantity + QuantitiesLostForEligibleMonsters[0] <= Con->PartyB[j]->MaxHealth)
							{
								Con->CurrentPhase = EActionPhasesType::Item;
								Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
								Con->ChangeCamera(MyMonster->MonsterID);
								Con->MyActionHUD->CurrentMonsterID = MonstersEligibleForHPItem[0];
								Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MonstersEligibleForHPItem[0]);
								Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
								Con->MyActionHUD->bIsFadingOut = false;
								Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
								Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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

			if (LostPercentageParty >= 0.6f) MembersBelow60.Add(Con->PartyB[i]->MonsterID);
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
									Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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

			if (LostPercentageParty >= 0.3f) MembersBelow30.Add(Con->PartyB[i]->MonsterID);
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
						Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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
		if (Con->GroupBInventory.Items[i].HP_Restored_Percentage >= (MyMonster->MaxHealth - MyMonster->CurrentHealth) / MyMonster->MaxHealth ||
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
				Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
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
		Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
		Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;
		bHasActionBeenDecidedYet = true;
	}
}

void AMonsterAIController::SelfMPPriority(ACombatController* &Con)
{
	if (MyMonster->CurrentMana < MyMonster->MaxMana)
	{
		float LostPercentage = (MyMonster->MaxMana - MyMonster->CurrentMana) / MyMonster->MaxMana;
		if (LostPercentage >= 0.4f)
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
						Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
					else if (Con->GroupBInventory.Items[i].MP_Restored_Percentage <= LostPercentage)
					{
						Con->CurrentPhase = EActionPhasesType::Item;
						Con->MyActionHUD->LatestItem = Con->GroupBInventory.Items[i];
						Con->ChangeCamera(MyMonster->MonsterID);
						Con->MyActionHUD->CurrentMonsterID = MyMonster->MonsterID;
						Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(MyMonster->MonsterID);
						Con->CurrentItemPhase = EActionPhasesItem::CameraOnActor_Item;
						Con->MyActionHUD->bIsFadingOut = false;
						Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
						Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestItem.Name;

						bHasActionBeenDecidedYet = true;

						return;
					}
				}
			}
		}
	}
}

void AMonsterAIController::DecideForSpellToCast(ACombatController* &Con)
{

	#pragma region Populate Targets

	TArray<int32> IndexesOfPossibleVictims;
	for (size_t i = 0; i < 3; i++)
	{
		if (!Con->PartyA[i]->bIsDead)
		{
			IndexesOfPossibleVictims.Add(Con->PartyA[i]->MonsterID);
		}
	}

#pragma endregion

	TArray<int32> AbnormalitySpells;
	for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
	{
		if (MyMonster->Spells[i].StatusAbnormality != EStatusAbnormality::NoAbnormality)
		{
			if (!MyMonster->Spells[i].bIsDefensiveSpell)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
				{
					AbnormalitySpells.Add(i);
				}
			}
		}
	}

	if (AbnormalitySpells.Num() > 0)
	{
		int32 RandomInt = 60;
		if (RandomInt > FMath::FRandRange(1, 100))
		{
			//Abnormality
			for (size_t i = 0; i < AbnormalitySpells.Num(); i++)
			{
				for (size_t j = 0; j < IndexesOfPossibleVictims.Num(); j++)
				{
					for (size_t z = 0; z < 3; z++)
					{
						if (Con->PartyA[z]->MonsterID == IndexesOfPossibleVictims[j])
						{
							for (size_t w = 0; w < Con->PartyA[z]->ActiveBuffs.Num(); w++)
							{
								if (Con->PartyA[z]->ActiveBuffs[w].OriginatingSpellName != MyMonster->Spells[AbnormalitySpells[i]].Name)
								{
									if (Con->GetMultiplierBasedOnElements(MyMonster->Spells[AbnormalitySpells[i]].ElementalPower, Con->PartyA[z]->MonsterAffinityType) >= 1.0f)
									{
										//FOUND MATCH! CAST!

										Con->CurrentPhase = EActionPhasesType::Ability;
										Con->MyActionHUD->LatestAbility = MyMonster->Spells[AbnormalitySpells[i]];
										Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
										Con->MyActionHUD->CurrentMonsterID = IndexesOfPossibleVictims[j];
										Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
										Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
										Con->MyActionHUD->bIsFadingOut = false;
										Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
										Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

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

	// Normal!
	int32 PredeterminedLoops = 0;
	while (PredeterminedLoops < 10)
	{
		int32 RandomInt = FMath::FRandRange(1, 100);
		int32 FinalTargetID = -1;
		if (RandomInt <= 40)
		{
			// Lowest HP
			int32 LowestHP = Con->PartyA[IndexesOfPossibleVictims[0]]->CurrentHealth;
			FinalTargetID = IndexesOfPossibleVictims[0];
			for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
			{
				if (LowestHP < Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth)
				{
					LowestHP = Con->PartyA[IndexesOfPossibleVictims[i]]->CurrentHealth;
					FinalTargetID = Con->PartyA[IndexesOfPossibleVictims[i]]->MonsterID;
				}
			}
		}
		else if (RandomInt <= 80)
		{
			//Lowest Magic Res
			int32 LowestMagicRes = Con->PartyA[IndexesOfPossibleVictims[0]]->GetMagicDefense();
			FinalTargetID = IndexesOfPossibleVictims[0];
			for (size_t i = 0; i < IndexesOfPossibleVictims.Num(); i++)
			{
				if (LowestMagicRes < Con->PartyA[IndexesOfPossibleVictims[i]]->GetMagicDefense())
				{
					LowestMagicRes = Con->PartyA[IndexesOfPossibleVictims[i]]->GetMagicDefense();
					FinalTargetID = Con->PartyA[IndexesOfPossibleVictims[i]]->MonsterID;
				}
			}
		}
		else
		{
			// Random
			FinalTargetID = IndexesOfPossibleVictims[FMath::RandRange(0, IndexesOfPossibleVictims.Num() - 1)];

		}

		TArray<int32> SpellIndexes;
		for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
		{
			if (!MyMonster->Spells[i].bIsDefensiveSpell)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
				{
					if (Con->GetMultiplierBasedOnElements(MyMonster->Spells[i].ElementalPower, Con->PartyA[FinalTargetID]->MonsterAffinityType) >= 2)
					{
						if (!Cast<ACustomMonsterActor>(Con->GetEnemyActorByID(FinalTargetID))->ActiveAbnormalities.Contains(MyMonster->Spells[i].StatusAbnormality))
						{
							SpellIndexes.Add(i);
						}
					}
				}
			}
		}

		if (SpellIndexes.Num() > 0)
		{
			Con->CurrentPhase = EActionPhasesType::Ability;
			Con->MyActionHUD->LatestAbility = MyMonster->Spells[SpellIndexes[FMath::RandRange(0, SpellIndexes.Num() - 1)]];
			Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
			Con->MyActionHUD->CurrentMonsterID = FinalTargetID;
			Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
			Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			Con->MyActionHUD->bIsFadingOut = false;
			Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
			Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

			bHasActionBeenDecidedYet = true;
			return;
		}
		SpellIndexes.Empty();

		for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
		{
			if (!MyMonster->Spells[i].bIsDefensiveSpell)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
				{
					if (Con->GetMultiplierBasedOnElements(MyMonster->Spells[i].ElementalPower, Con->PartyA[FinalTargetID]->MonsterAffinityType) >= 1.5f)
					{
						if (!Cast<ACustomMonsterActor>(Con->GetEnemyActorByID(FinalTargetID))->ActiveAbnormalities.Contains(MyMonster->Spells[i].StatusAbnormality))
						{
							SpellIndexes.Add(i);
						}
					}
				}
			}
		}
		if (SpellIndexes.Num() > 0)
		{
			Con->CurrentPhase = EActionPhasesType::Ability;
			Con->MyActionHUD->LatestAbility = MyMonster->Spells[SpellIndexes[FMath::RandRange(0, SpellIndexes.Num() - 1)]];
			Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
			Con->MyActionHUD->CurrentMonsterID = FinalTargetID;
			Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
			Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			Con->MyActionHUD->bIsFadingOut = false;
			Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
			Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

			bHasActionBeenDecidedYet = true;
			return;
		}
		SpellIndexes.Empty();
		for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
		{
			if (!MyMonster->Spells[i].bIsDefensiveSpell)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
				{
					if (Con->GetMultiplierBasedOnElements(MyMonster->Spells[i].ElementalPower, Con->PartyA[FinalTargetID]->MonsterAffinityType) >= 1.f)
					{
						if (!Cast<ACustomMonsterActor>(Con->GetEnemyActorByID(FinalTargetID))->ActiveAbnormalities.Contains(MyMonster->Spells[i].StatusAbnormality))
						{
							SpellIndexes.Add(i);
						}
					}
				}
			}
		}
		if (SpellIndexes.Num() > 0)
		{
			Con->CurrentPhase = EActionPhasesType::Ability;
			Con->MyActionHUD->LatestAbility = MyMonster->Spells[SpellIndexes[FMath::RandRange(0, SpellIndexes.Num() - 1)]];
			Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
			Con->MyActionHUD->CurrentMonsterID = FinalTargetID;
			Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
			Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			Con->MyActionHUD->bIsFadingOut = false;
			Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
			Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

			bHasActionBeenDecidedYet = true;
			return;
		}
		SpellIndexes.Empty();
		for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
		{
			if (!MyMonster->Spells[i].bIsDefensiveSpell)
			{
				if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
				{
					if (Con->GetMultiplierBasedOnElements(MyMonster->Spells[i].ElementalPower, Con->PartyA[FinalTargetID]->MonsterAffinityType) >= 0.5f)
					{
						if (!Cast<ACustomMonsterActor>(Con->GetEnemyActorByID(FinalTargetID))->ActiveAbnormalities.Contains(MyMonster->Spells[i].StatusAbnormality))
						{
							SpellIndexes.Add(i);
						}
					}
				}
			}
		}
		if (SpellIndexes.Num() > 0)
		{
			Con->CurrentPhase = EActionPhasesType::Ability;
			Con->MyActionHUD->LatestAbility = MyMonster->Spells[SpellIndexes[FMath::RandRange(0, SpellIndexes.Num() - 1)]];
			Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
			Con->MyActionHUD->CurrentMonsterID = FinalTargetID;
			Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
			Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;
			Con->MyActionHUD->bIsFadingOut = false;
			Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
			Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

			bHasActionBeenDecidedYet = true;
			return;
		}
		SpellIndexes.Empty();
		PredeterminedLoops++;
	}

}

void AMonsterAIController::PerformDefend(ACombatController* &Con)
{
	Con->HelperPhaseTimer = 0;
	Con->CurrentPhase = EActionPhasesType::Defend;
	Con->CurrentDefendPhase = EActionPhasesDefend::DefendingParticle;
	Con->MyActionHUD->bIsFadingOut = false;
	Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
	Con->SetBottomTooltip(true, Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterName + " Defends! (+50% Defense until next turn!)");
	Con->MyActionHUD->CentralTooltip = "Defend";
	bHasActionBeenDecidedYet = true;
}

void AMonsterAIController::PerformAttack(ACombatController* &Con, int32 TargetID)
{
	if(Cast<ACustomMonsterActor>(Con->GetEnemyActorByID(TargetID))->bIsDead)GLog->Log("OMG! 3");
	
	Con->HelperPhaseTimer = 0;
	Con->CurrentPhase = EActionPhasesType::Attack;
	Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);
	Con->ReceiverPos = Con->GetEnemyMeleeReceiverPosByID(TargetID);
	Con->MyActionHUD->bIsFadingOut = false;
	Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
	Con->MyActionHUD->CurrentMonsterID = TargetID;
	Con->MyActionHUD->CentralTooltip = "Attack";
	Con->CurrentAttackPhase = EActionPhasesAttack::CameraOnActor;
	bHasActionBeenDecidedYet = true;
}

void AMonsterAIController::Perform_HP_Spell(ACombatController* &Con)
{

	if (MyMonster->AI_Archetype != EAIArchetypes::Healer)return;
	

	TArray<int32> MonstersEligibleForHPItem;
	TArray<int32> PercetagesLostForEligibleMonsters;
	TArray<int32> QuantitiesLostForEligibleMonsters;

	// Get Possible Members
	for (size_t i = 0; i < Con->PartyB.Num(); i++)
	{
		if (!Con->PartyB[i]->bIsDead)
		{
			if (Con->PartyB[i]->CurrentHealth < Con->PartyB[i]->MaxHealth)
			{
				float LostPercentage = (Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth) / Con->PartyB[i]->MaxHealth;
				if (LostPercentage >= 0.4f)
				{
					MonstersEligibleForHPItem.Add(Con->PartyB[i]->MonsterID);
					PercetagesLostForEligibleMonsters.Add(LostPercentage);
					QuantitiesLostForEligibleMonsters.Add(Con->PartyB[i]->MaxHealth - Con->PartyB[i]->CurrentHealth);
				}
			}
		}
		
	}

	//Prioritize
	int32 IndexOfMemberThatNeedsHP = -1;
	if (MonstersEligibleForHPItem.Num() == 3)
	{
		//100% AOE Spell.
		IndexOfMemberThatNeedsHP = 100;
	}
	else if (MonstersEligibleForHPItem.Num() == 2)
	{
		
		if (QuantitiesLostForEligibleMonsters[0] > QuantitiesLostForEligibleMonsters[1] * 1.5f)
		{
			IndexOfMemberThatNeedsHP = MonstersEligibleForHPItem[0];
		}
		else if (QuantitiesLostForEligibleMonsters[1] > QuantitiesLostForEligibleMonsters[0] * 1.5f)
		{
			IndexOfMemberThatNeedsHP = MonstersEligibleForHPItem[1];
		}
		else
		{
			IndexOfMemberThatNeedsHP = 100; // 100 is something that we are going to use to let C++ that we need AOE spell!
		}
	}
	else if(MonstersEligibleForHPItem.Num() == 1)
	{
		// 100% Single Heal Spell.
		IndexOfMemberThatNeedsHP = MonstersEligibleForHPItem[0];
	}
	
	


	if (IndexOfMemberThatNeedsHP != -1)
	{
			for (size_t i = 0; i < MyMonster->Spells.Num(); i++)
			{
				if (MyMonster->Spells[i].bIsDefensiveSpell)
				{
					if (MyMonster->Spells[i].ManaCost <= MyMonster->CurrentMana)
					{
						if (!MyMonster->Spells[i].bIsDamagingForHealth)
						{
							if (MyMonster->Spells[i].BaseSpellPowerHealth>0)
							{
								if (IndexOfMemberThatNeedsHP == 100)
								{
									if (MyMonster->Spells[i].Target == "All Allies")
									{
										Con->CurrentPhase = EActionPhasesType::Ability;
										Con->MyActionHUD->LatestAbility = MyMonster->Spells[i];
										Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
										Con->ChangeCamera(ECameras::Camera_Party_B);

										Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;

										Con->MyActionHUD->CurrentMonsterID = -1;
										Con->MyActionHUD->bIsCustomTooltip = false;
										Con->MyActionHUD->BottomTooltip = "";
										Con->MyActionHUD->bIsFadingOut = false;
										Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
										Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

										Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);

										bHasActionBeenDecidedYet = true;
									}
								}
								else
								{
									if (MyMonster->Spells[i].Target == "Self" && IndexOfMemberThatNeedsHP == Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID)
									{
										Con->CurrentPhase = EActionPhasesType::Ability;
										Con->MyActionHUD->LatestAbility = MyMonster->Spells[i];
										Con->MyActionHUD->CurrentMonsterID = IndexOfMemberThatNeedsHP;
										Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
										Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);

										Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;



										Con->MyActionHUD->bIsFadingOut = false;
										Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
										Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

										bHasActionBeenDecidedYet = true;
									}
									if (MyMonster->Spells[i].Target == "Single Ally")
									{
										Con->CurrentPhase = EActionPhasesType::Ability;
										Con->MyActionHUD->LatestAbility = MyMonster->Spells[i];
										Con->MyActionHUD->CurrentMonsterID = IndexOfMemberThatNeedsHP;
										Con->MyActionHUD->CurrentTarget = Con->MyActionHUD->LatestAbility.Target;
										Con->ChangeCamera(Con->TurnOrder[Con->IndexOfCurrentPlayingMonster]->MonsterID);

										Con->CurrentAbilityPhase = EActionPhasesAbility::CameraOnActorAbility;


										Con->GlobalPool->PlaySound2D(Con->SFX_Dictionary.FindRef(ESoundEffectLibrary::SFX_ActionStart));
										Con->MyActionHUD->bIsFadingOut = false;
										Con->MyActionHUD->CentralTooltip = Con->MyActionHUD->LatestAbility.Name;

										bHasActionBeenDecidedYet = true;
									}
								}
							}
						}
					}
				}
			}
		
	}
}