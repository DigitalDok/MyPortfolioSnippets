// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCombat.h"
#include "MonsterAnimInstance.h"
#include "CombatController.h"



void UMonsterAnimInstance::ApplyDamageToTargetActor()
{
	ACombatController* Controller = Cast<ACombatController>(GetWorld()->GetFirstPlayerController());
	Controller->ApplyDamage(Controller->TurnOrder[Controller->IndexOfCurrentPlayingMonster], Cast<ACustomMonsterActor>(TargetActor));
}
