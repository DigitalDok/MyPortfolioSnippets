// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "AI_Bot.h"
#include "MasterBot.generated.h"

/**
 * 
 */
UCLASS()
class EQS_DEMO_API AMasterBot : public AGameMode
{
	GENERATED_BODY()
	
public:

	// Sets default values for this character's properties
	AMasterBot();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> AllBots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> RedTeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> BlueTeam;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> BlueTeam_Berserkers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> BlueTeam_Guards;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> BlueTeam_Warriors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> BlueTeam_Tacticians;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> RedTeam_Berserkers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> RedTeam_Guards;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> RedTeam_Warriors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		TArray<AAI_Bot*> RedTeam_Tacticians;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		int RedTeamKills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		int BlueTeamKills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		float RedTeamMorale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		float BlueTeamMorale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI System")
		bool MoraleSystemOn;

	void ChangeMorale(bool bTeamRed, float Val);
	void LastStand(bool bTeamRed);


	void GetAllTeams();
	
	
};
