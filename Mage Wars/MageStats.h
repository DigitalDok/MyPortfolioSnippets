// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayUI.h"
#include "MageStats.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGEWARS_API UMageStats : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMageStats();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	
	UPROPERTY(EditAnywhere, Category = "Mage Core Stat")
		AActor* MyMage;

	UPROPERTY(EditAnywhere, Category = "Mage Core Stat")
		float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Mage Core Stat")
		float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "Mage Core Stat")
		float MaxMana;

	UPROPERTY(EditAnywhere, Category = "Mage Core Stat")
		float CurrentMana;

	UPROPERTY(EditAnywhere, Category = "Mage Regen")
		float ManaRegenRate;

	UPROPERTY(EditAnywhere, Category = "Mage Regen")
		float HealthRegenRate;

	UPROPERTY(EditAnywhere, Category = "Mage Regen")
		float RegenTick;

	UPROPERTY(EditAnywhere, Category = "Mage Regen")
		float RegenTimer;

	UPROPERTY(EditAnywhere, Category = "Mage Speed")
		float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Mage Speed")
		float AttackSpeed;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float SpellPower;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float SpellPower_Extra_Percentage_Min;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float SpellPower_Extra_Percentage_Max;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float SpellCrit;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float CritMultiplier;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float MagicPenetration;

	UPROPERTY(EditAnywhere, Category = "Mage Combat")
		float MagicResistance;


	void UpdateBuffEffect(EBuffs BuffEffect, bool WillRemove);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_UpdateBuffEffect(EBuffs BuffEffect, bool WillRemove);
	void Server_UpdateBuffEffect_Implementation(EBuffs BuffEffect, bool WillRemove);
	bool Server_UpdateBuffEffect_Validate(EBuffs BuffEffect, bool WillRemove);

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_UpdateBuffEffect(EBuffs BuffEffect, bool WillRemove);
	void Multicast_UpdateBuffEffect_Implementation(EBuffs BuffEffect, bool WillRemove);

	TArray<TEnumAsByte<EBuffs>> AppliedBuffs;

	// *********************************** MAIN SPELLS ************************************* // 


	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Lifetime;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Speed;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_ManaCost;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Force;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Damage;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Debuff_Duration;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		float Frost_IceTrail_Cooldown;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		UMaterial* Frost_IceTrail_LinePlaneMaterial;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		UTexture2D* Frost_IceTrail_Icon;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trail")
		TSubclassOf<AActor> Frost_IceTrail_Projectile;



	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_Lifetime;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_Speed;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_MaxRange;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_RegionRadius;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_ManaCost;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		float Frost_IceTrap_Cooldown;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		UMaterial* Frost_IceTrap_CirclePlaneMaterial;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		UTexture2D* Frost_IceTrap_Icon;

	UPROPERTY(EditAnywhere, Category = "[Frost] Ice Trap")
		TSubclassOf<AActor> Frost_IceTrap_Projectile;




	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		float Frost_FrostArmor_Lifetime;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		float Frost_FrostArmor_Speed;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		float Frost_FrostArmor_ManaCost;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		float Frost_FrostArmor_Cooldown;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		UMaterial* Frost_FrostArmor_CirclePlaneMaterial;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		UTexture2D* Frost_FrostArmor_Icon;

	UPROPERTY(EditAnywhere, Category = "[Frost] Frost Armor")
		TSubclassOf<AActor> Frost_FrostArmor_Projectile;




	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_Lifetime;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_Speed;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_ManaCost;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_Cooldown;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_MaxRange;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		float Frost_GlacierShattering_CircleRadius;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		UMaterial* Frost_GlacierShattering_CirclePlaneMaterial;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		UTexture2D* Frost_GlacierShattering_Icon;

	UPROPERTY(EditAnywhere, Category = "[Frost] Glacier Shattering")
		TSubclassOf<AActor> Frost_GlacierShattering_Projectile;
	
};
