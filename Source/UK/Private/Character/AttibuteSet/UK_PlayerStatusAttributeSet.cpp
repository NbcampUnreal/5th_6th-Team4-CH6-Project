// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/UK_CharacterBase.h"
#include "Tags/UK_GameplayTags.h"
#include "Systems/Data/UK_InGameSave.h"

UUK_PlayerStatusAttributeSet::UUK_PlayerStatusAttributeSet()
{
}

void UUK_PlayerStatusAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 데미지 전처리
	if (Attribute == GetDamageAttribute())
	{
		/*받는 피해량 = (몬스터 공격력 X 스킬 계수) X ( 고정상수 C/ 고정상수C + 방어력)*/
		float LocalDamage = NewValue;
		float LocalDefense = 1.f / (1.f + GetDefence());
		LocalDamage *= LocalDefense;
		NewValue = LocalDamage;
		if (GetOwningAbilitySystemComponent()->HasMatchingGameplayTag(UK_GameplayTags::Action::invincibility))
		{
			NewValue = 0.f;
		}
	}
	// 체력 전처리
	else if (Attribute == GetHealthAttribute())
	{
		const float OldValue = NewValue;
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());

		if (OldValue != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("[AttributeSet] PreAttributeChange - Health clamped: %.1f → %.1f"), OldValue,
			       NewValue);
		}

		if (NewValue <= 0.f)
		{
		}
		UE_LOG(LogTemp, Log, TEXT("   Health: %.1f → %.1f (Max: %.1f)"),
		       OldValue, NewValue, GetMaxHealth());
	}
	// 최대채력 전처리
	else if (Attribute == GetMaxHealthAttribute())
	{
		const float OldMaxHealth = GetMaxHealth();

		if (OldMaxHealth != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("   Health adjusted: %.1f → %.1f"), OldMaxHealth, GetMaxHealth());
		}
	}
	// 최대 MP 전 처리
	else if (Attribute == GetMaxMpAttribute())
	{
		const float OldMaxHealth = GetMaxMp();


		if (OldMaxHealth != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("   Max Mp: %.1f → %.1f"), OldMaxHealth, NewValue);
		}
	}
	// MP 전처리
	else if (Attribute == GetCurrentMpAttribute())
	{
		const float OldValue = GetCurrentMp();

		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMp());


		UE_LOG(LogTemp, Log, TEXT("   Mp: %.1f → %.1f (Max: %.1f)"),
		       OldValue, NewValue, GetMaxMp());
	}
	// 최대 스테미너 전처리
	else if (Attribute == GetMaxStaminaAttribute())
	{
		const float OldMaxStamina = GetMaxStamina();

		if (OldMaxStamina != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("   Max Stamina: %.1f → %.1f"), OldMaxStamina, NewValue);
		}
	}
	// 스테미너 전처리
	else if (Attribute == GetCurrentStaminaAttribute())
	{
		const float OldValue = GetCurrentStamina();

		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());


		// UE_LOG(LogTemp, Log, TEXT("   Stamina: %.1f → %.1f (Max: %.1f)"),
		//        OldValue, NewValue, GetMaxStamina());
	}
	// 최대 경험치 전처리
	else if (Attribute == GetMaxEXPAttribute())
	{
		const float OldMaxExp = GetMaxEXP();

		if (OldMaxExp != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("   Max EXP: %.1f → %.1f"), OldMaxExp, NewValue);
		}
	}
	// 경험치 전처리
	else if (Attribute == GetEXPAttribute())
	{
		const float OldValue = GetEXP();
		if (GetMaxLevel() == GetLevel())
		{
			NewValue = 0.f;
		}
		
		UE_LOG(LogTemp, Log, TEXT("   EXP: %.1f → %.1f (Max: %.1f)"),
		       OldValue, NewValue, GetMaxEXP());
	}
	// 최대 레벨 전처리
	else if (Attribute == GetMaxLevelAttribute())
	{
		const float OldValue = GetMaxLevel();

		if (OldValue != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("   Max Level: %.1f → %.1f"), OldValue, NewValue);
		}
	}
	// 레벨 전 처리 
	else if (Attribute == GetLevelAttribute())
	{
		const float OldValue = GetLevel();

		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxLevel());

		UE_LOG(LogTemp, Log, TEXT("   Level: %.1f → %.1f (Max: %.1f)"),
		       OldValue, NewValue, GetMaxLevel());
	}
	// 방어력 전 처리 
	else if (Attribute == GetDefenceAttribute())
	{
	}
	else if (Attribute == GetAttackPowerAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("   AttackPower: %.1f → %.1f"),
		       GetAttackPower(), NewValue);
	}
	else if (Attribute == GetCurrentPowerAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("   CurrentPower: %.1f → %.1f"),
		       GetCurrentPower(), NewValue);
	}
}
void UUK_PlayerStatusAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetCurrentStaminaAttribute())
    {
    	float NewValue = GetCurrentStamina();
        SetCurrentStamina(FMath::Clamp(NewValue, 0.f, GetMaxStamina()));
    }
    else if (Data.EvaluatedData.Attribute == GetCurrentMpAttribute())
    {
    	float NewValue = GetCurrentMp();
        SetCurrentMp(FMath::Clamp(NewValue, 0.f, GetMaxMp()));
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
    	float NewValue = GetHealth();
        SetHealth(FMath::Clamp(NewValue, 0.f, GetMaxHealth()));
    }
	
}
void UUK_PlayerStatusAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
                                                       float NewValue)
{
	// 데미지 후처리
	if (Attribute == GetDamageAttribute())
	{
		if (NewValue != 0.f)
		{
			DamageChanged.Broadcast(OldValue, NewValue);
			float OldHealth = GetHealth();
			SetHealth(FMath::Clamp(GetHealth() - GetDamage(), 0.0f, GetMaxHealth()));
			UE_LOG(LogTemp, Display, TEXT("%f-> %f (MAX : %f)"), OldHealth, GetHealth(), GetMaxHealth())
			SetDamage(0.f);
		}
	}
	// 체력 후처리
	else if (Attribute == GetHealthAttribute())
	{
		if (GetHealth() <= 0.f)
		{
			HandleOutOfHealth();
		}
		//SetCurrentMp(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		HealthChanged.Broadcast(OldValue, NewValue);
	}
	// 최대채력 후처리
	else if (Attribute == GetMaxHealthAttribute())
	{
		SetHealth(GetMaxHealth());
		MaxHealthChanged.Broadcast(OldValue, NewValue);
	}
	// 최대 MP 후처리
	else if (Attribute == GetMaxMpAttribute())
	{
		SetCurrentMp(GetMaxMp());
		MaxMpChanged.Broadcast(OldValue, NewValue);
	}
	// MP 후처리
	else if (Attribute == GetCurrentMpAttribute())
	{
		//SetCurrentMp(FMath::Clamp(GetCurrentMp(), 0.f, GetMaxMp()));
		CurrentMpChanged.Broadcast(OldValue, NewValue);
	}
	// 최대 스테미너 후처리
	else if (Attribute == GetMaxStaminaAttribute())
	{
		SetCurrentStamina(GetMaxStamina());
		MaxMpChanged.Broadcast(OldValue, NewValue);
	}
	// 스테미너 후처리
	else if (Attribute == GetCurrentStaminaAttribute())
	{
		if (GetCurrentStamina() <= 0.f)
		{
			HandleOutOStamina();
		}
		//SetCurrentStamina(FMath::Clamp(GetCurrentStamina(), 0.f, GetMaxStamina()));
		CurrentStaminaChanged.Broadcast(OldValue, NewValue);
	}
	// 최대 경험치 후처리
	else if (Attribute == GetMaxEXPAttribute())
	{
		MaxEXPChanged.Broadcast(OldValue, NewValue);
	}
	// 경험치 후처리
	else if (Attribute == GetEXPAttribute())
	{
		if (GetMaxEXP() != 0 && GetMaxEXP() <= GetEXP())
		{
			float RequiredEXP = GetMaxEXP();
			SetLevel(FMath::Min(GetLevel() + 1, GetMaxLevel()));
			SetEXP(GetEXP() - RequiredEXP);
			EXPChanged.Broadcast(OldValue, NewValue);
		}
	}
	// 최대 레벨 후처리
	else if (Attribute == GetMaxLevelAttribute())
	{
		MaxLevelChanged.Broadcast(OldValue, NewValue);
	}
	// 레벨 후 처리 
	else if (Attribute == GetLevelAttribute())
	{
		if (GetMaxLevel() != NewValue)
		{
			if (NewValue == 1)
				return;
			SetMaxEXP((NewValue*2) * 10 + 100);
			SetAttackPower(GetAttackPower() + 3);
			SetMaxHealth(GetMaxHealth() + 10);
			SetMaxMp(GetMaxMp() + 10);
			LevelUp();
			LevelChanged.Broadcast(OldValue, NewValue);
		}
	}
	// 방어력 후처리
	else if (Attribute == GetLevelAttribute())
	{
		DefenceChanged.Broadcast(OldValue, NewValue);
	}
	else if ( Attribute == GetCurrentPowerAttribute() )
	{
		CurrentPowerChanged.Broadcast(OldValue, NewValue);
	}
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UUK_PlayerStatusAttributeSet::HandleOutOfHealth()
{
	UE_LOG(LogTemp, Log, TEXT("OnDead"));

	if (AActor* Avatar = GetOwningAbilitySystemComponent()->GetAvatarActor())
	{
		if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Avatar))
		{
			UE_LOG(LogTemp, Log, TEXT("OnDead1"));
			Player->Dead();
		}
	}
}

void UUK_PlayerStatusAttributeSet::HandleOutOStamina()
{
	if (AActor* Avatar = GetOwningAbilitySystemComponent()->GetAvatarActor())
	{
		if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Avatar))
		{
			UE_LOG(LogTemp, Log, TEXT("OutOfStamina"));
			Player->OutOfStamina();
		}
	}
}
void UUK_PlayerStatusAttributeSet::LevelUp()
{
	if (AActor* Avatar = GetOwningAbilitySystemComponent()->GetAvatarActor())
	{
		if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(Avatar))
		{
			Player->HandleLevelUp();
		}
	}
}

void UUK_PlayerStatusAttributeSet::ExportStats(FCharacterStatSaveData& OutData)
{
	OutData.Health = GetHealth();
	OutData.MaxHealth = GetMaxHealth();
	OutData.AttackPower = GetAttackPower();
	OutData.CurrentPower = GetCurrentPower();
	OutData.CurrentMp = GetCurrentMp();
	OutData.MaxMp = GetMaxMp();
	OutData.MaxStamina = GetMaxStamina();
	OutData.CurrentStamina = GetCurrentStamina();
	OutData.Level = GetLevel();
	OutData.MaxLevel = GetMaxLevel();
	OutData.Exp = GetEXP();
	OutData.MaxEXP = GetMaxEXP();
	OutData.Defence = GetDefence();
}

void UUK_PlayerStatusAttributeSet::ImportStats(const struct FCharacterStatSaveData& InData)
{
	InitAttackPower(InData.AttackPower);
	InitLevel(InData.Level);
	InitMaxLevel(InData.MaxLevel);
	InitEXP(InData.Exp);
	InitMaxEXP(InData.MaxEXP);
	InitDefence(InData.Defence);
    
	InitMaxHealth(InData.MaxHealth);
	InitMaxMp(InData.MaxMp);
	InitMaxStamina(InData.MaxStamina);

	SetHealth(InData.Health);
	SetCurrentMp(InData.CurrentMp);
	SetCurrentStamina(InData.CurrentStamina);

	HealthChanged.Broadcast(0.f, InData.Health);
	CurrentMpChanged.Broadcast(0.f, InData.CurrentMp);
	EXPChanged.Broadcast(0.f, InData.Exp);
	MaxEXPChanged.Broadcast(0.f, InData.MaxEXP); 
}

