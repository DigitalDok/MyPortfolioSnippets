// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayUI.h"
#include "DmgOverlayUI.h"
#include "EnemyHealthBarUI.h"
#include "GlobalController.h"

#include "MageMastermind.generated.h"



UENUM()
enum EAvailableSpells
{
	None,
	Frost_IceTrail,
	Frost_IceTrap,
	Frost_FrostArmor,
	Forst_GlacierShattering
};

/**
 * 
 */
UCLASS()
class MAGEWARS_API AMageMastermind : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	
	virtual void BeginPlay() override;

	AGlobalController* GameModeMaster;

	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> PlayerUserInterfaceHolder;

	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> DmgUIPlaceholder;

	UPROPERTY(EditAnywhere, Category = "User Interface Stuff")
		TSubclassOf<class UUserWidget> HPBarPlaceholder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float Cooldown_Spell_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float Cooldown_Spell_2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float Cooldown_Spell_3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage Attack")
		float Cooldown_Spell_4;

	UPROPERTY(EditAnywhere, Category = "Spellbook")
		TEnumAsByte<EAvailableSpells> Spell_1;
	UPROPERTY(EditAnywhere, Category = "Spellbook")
		TEnumAsByte<EAvailableSpells> Spell_2;
	UPROPERTY(EditAnywhere, Category = "Spellbook")
		TEnumAsByte<EAvailableSpells> Spell_3;
	UPROPERTY(EditAnywhere, Category = "Spellbook")
		TEnumAsByte<EAvailableSpells> Spell_4;

	void DetermineManaCosts();

	void DetermineManaCost(UTexture2D* &Icon, float &SpellPlaceholderMana, TEnumAsByte<EAvailableSpells> TheSpell, float &Cooldown);

	UGameplayUI* GameplayUI;
	UDmgOverlayUI* DamageOverHead;
	UEnemyHealthBarUI* EnemyHPBarUIWidget;

	void CreateDamageTextAboveActor(AActor* Victim, float Damage, float Lifetime);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_CreateDamageTextAboveActor(AActor* Victim, float Damage, float Lifetime);
	void Server_CreateDamageTextAboveActor_Implementation(AActor* Victim, float Damage, float Lifetime);
	bool Server_CreateDamageTextAboveActor_Validate(AActor* Victim, float Damage, float Lifetime);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CreateDamageTextAboveActor(AActor* Victim, float Damage, float Lifetime);
	void Multicast_CreateDamageTextAboveActor_Implementation (AActor* Victim, float Damage, float Lifetime);

	void UpdateUI();

	void GenerateUI();


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
