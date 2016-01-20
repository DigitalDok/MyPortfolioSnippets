// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ItemPanel_Button.h"
#include "ItemPanel_Targets.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERCOMBAT_API UItemPanel_Targets : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		FString MyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI C++")
		UItemPanel_Button* AssociatedButton;
};
