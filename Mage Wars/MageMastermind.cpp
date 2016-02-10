// Fill out your copyright notice in the Description page of Project Settings.

#include "MageWars.h"
#include "MageMastermind.h"
#include "PlayerMageCharacter.h"


void AMageMastermind::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;
}

void AMageMastermind::DetermineManaCosts()
{
	if (Role < ROLE_Authority)
	{
		APlayerMageCharacter* LeChar = Cast<APlayerMageCharacter>(GetCharacter());

		if (LeChar)
		{

			DetermineManaCost(GameplayUI->SpellIcon_1 , LeChar->Spell_1_ManaCost, Spell_1, Cooldown_Spell_1);
			DetermineManaCost(GameplayUI->SpellIcon_2 , LeChar->Spell_2_ManaCost, Spell_2, Cooldown_Spell_2);
			DetermineManaCost(GameplayUI->SpellIcon_3 , LeChar->Spell_3_ManaCost, Spell_3, Cooldown_Spell_3);
			DetermineManaCost(GameplayUI->SpellIcon_4 , LeChar->Spell_4_ManaCost, Spell_4, Cooldown_Spell_4);
		}
	}
}

void AMageMastermind::DetermineManaCost(UTexture2D* &Icon, float &CharacterPlaceholder, TEnumAsByte<EAvailableSpells> TheSpell, float &Cooldown)
{
	APlayerMageCharacter* LeChar = Cast<APlayerMageCharacter>(GetCharacter());

	switch (TheSpell)
	{
	case EAvailableSpells::None:
		break;
	case EAvailableSpells::Frost_IceTrail:
		CharacterPlaceholder = Frost_IceTrail_ManaCost;
		Icon = Frost_IceTrail_Icon;
		Cooldown = Frost_IceTrail_Cooldown;
		break;
	case EAvailableSpells::Frost_IceTrap:
		CharacterPlaceholder = Frost_IceTrap_ManaCost;
		Icon = Frost_IceTrap_Icon;
		Cooldown = Frost_IceTrap_Cooldown;
		break;
		case EAvailableSpells::Frost_FrostArmor:
		CharacterPlaceholder = Frost_FrostArmor_ManaCost;
		Icon = Frost_FrostArmor_Icon;
		Cooldown = Frost_FrostArmor_Cooldown;
		break;
		case EAvailableSpells::Forst_GlacierShattering:
		CharacterPlaceholder = Frost_GlacierShattering_ManaCost;
		Icon = Frost_GlacierShattering_Icon;
		Cooldown = Frost_GlacierShattering_Cooldown;
		break;
	}
}

void AMageMastermind::CreateDamageTextAboveActor(AActor* Victim, float Damage, float Lifetime)
{
	if(Role < ROLE_Authority)
	if (DmgUIPlaceholder)
	{
		DamageOverHead = Cast<UDmgOverlayUI>(CreateWidget<UUserWidget>(this, DmgUIPlaceholder));
		DamageOverHead->AddToViewport();

		DamageOverHead->Victim = Victim;
		DamageOverHead->DamageText = Damage;
		DamageOverHead->LifeTime = Lifetime;

		DamageOverHead->SetThingsUp();
	}
}


void AMageMastermind::Server_CreateDamageTextAboveActor_Implementation(AActor* Victim, float Damage, float Lifetime)
{
	Multicast_CreateDamageTextAboveActor(Victim, Damage, Lifetime);
}
bool AMageMastermind::Server_CreateDamageTextAboveActor_Validate(AActor* Victim, float Damage, float Lifetime)
{
	return true;
}
void AMageMastermind::Multicast_CreateDamageTextAboveActor_Implementation(AActor* Victim, float Damage, float Lifetime)
{
	CreateDamageTextAboveActor(Victim, Damage, Lifetime);
}

void AMageMastermind::GenerateUI()
{
	if (Role < ROLE_Authority)
	{
		
		if (PlayerUserInterfaceHolder)
		{
			GameplayUI = Cast<UGameplayUI>(CreateWidget<UUserWidget>(this, PlayerUserInterfaceHolder));
			GameplayUI->AddToViewport();

			DetermineManaCosts();
		}
		if (HPBarPlaceholder)
		{
			EnemyHPBarUIWidget = Cast<UEnemyHealthBarUI>(CreateWidget<UUserWidget>(this, HPBarPlaceholder));
			APlayerMageCharacter* LeChar = Cast<APlayerMageCharacter>(GetCharacter());
			
			for (TActorIterator<APlayerMageCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				if (*ActorItr != LeChar)
				{
					EnemyHPBarUIWidget->Victim = Cast<AActor>(*ActorItr);
					break;
				}
			}
		}

		
	}
}

void AMageMastermind::UpdateUI()
{
	if (Role < ROLE_Authority)
	{
		APlayerMageCharacter* LeChar = Cast<APlayerMageCharacter>(GetCharacter());
		if (LeChar)
		{
			GameplayUI->HealthPercentage = (LeChar->CurrentHealth / LeChar->MaxHealth) * 100;
			GameplayUI->ManaPercentage = (LeChar->CurrentMana / LeChar->MaxMana) * 100;

			GameplayUI->ActiveCD_1 = LeChar->ActiveCD_1;
			GameplayUI->ActiveCD_2 = LeChar->ActiveCD_2;
			GameplayUI->ActiveCD_3 = LeChar->ActiveCD_3;
			GameplayUI->ActiveCD_4 = LeChar->ActiveCD_4;
		}
		if (HPBarPlaceholder)
		{

		}
	}
}



