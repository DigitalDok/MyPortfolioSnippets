// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CombatMeters.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERCOMBAT_API UCombatMeters : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat Meter Custom")
		void CreateNumberOverHead(bool bIsCrit, FLinearColor Col, const FString& Number);
	
};
