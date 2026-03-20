#include "UI/Inventory/UK_InvSystem.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"

void UUK_InvSystem::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningPawn);
	if (ASC)
	{
		const UUK_PlayerStatusAttributeSet* AS = ASC->GetSet<UUK_PlayerStatusAttributeSet>();
		if (AS)
		{
			UUK_PlayerStatusAttributeSet* MutableAS = const_cast<UUK_PlayerStatusAttributeSet*>(AS);

			//델리게이트 바인딩
			MutableAS->HealthChanged.AddDynamic(this, &UUK_InvSystem::UpdateHealth);
			MutableAS->MaxHealthChanged.AddDynamic(this, &UUK_InvSystem::UpdateMaxHealth);
			MutableAS->AttackPowerChanged.AddDynamic(this, &UUK_InvSystem::UpdateAttackPower);
			MutableAS->CurrentMpChanged.AddDynamic(this, &UUK_InvSystem::UpdateCurrentMp);
			MutableAS->MaxMpChanged.AddDynamic(this, &UUK_InvSystem::UpdateMaxMp);
			MutableAS->DefenceChanged.AddDynamic(this, &UUK_InvSystem::UpdateDefence);
			MutableAS->CriticalChanceChanged.AddDynamic(this, &UUK_InvSystem::UpdateCriticalChance);
			MutableAS->CriticalDamageChanged.AddDynamic(this, &UUK_InvSystem::UpdateCriticalDamage);
			MutableAS->MaxStaminaChanged.AddDynamic(this, &UUK_InvSystem::UpdateMaxStamina);
			MutableAS->CurrentPowerChanged.AddDynamic(this, &UUK_InvSystem::UpdateAttackWeapon);

			//초기 값 설정
			ProcessStatUpdate(StateHealth, Arrow_Health, 0.f, AS->GetHealth());
			ProcessStatUpdate(StateMaxHealth, Arrow_MaxHealth, 0.f, AS->GetMaxHealth());
			ProcessStatUpdate(StateAttackPower, Arrow_Attack, 0.f, AS->GetAttackPower());
			ProcessStatUpdate(StateCurrentMp, Arrow_CurrentMp, 0.f, AS->GetCurrentMp());
			ProcessStatUpdate(StateMaxMp, Arrow_MaxMp, 0.f, AS->GetMaxMp());
			ProcessStatUpdate(StateDefence, Arrow_Defence, 0.f, AS->GetDefence());
			ProcessStatUpdate(StateCriticalChance, Arrow_CritChance, 0.f, AS->GetCriticalChance());
			ProcessStatUpdate(StateCriticalDamage, Arrow_CritDamage, 0.f, AS->GetCriticalDamage());
			ProcessStatUpdate(StateMaxStamina, Arrow_Stamina, 0.f, AS->GetMaxStamina());

			float WeaponPower = AS->GetCurrentPower();
			ProcessStatUpdate(StateAttackWeapon, Arrow_AttackWeapon, 0.f, WeaponPower);
		}
	}
}

void UUK_InvSystem::UpdateHealth(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateHealth, Arrow_Health, OldValue, NewValue); 
}
void UUK_InvSystem::UpdateMaxHealth(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateMaxHealth, Arrow_MaxHealth, OldValue, NewValue);
}
void UUK_InvSystem::UpdateAttackPower(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateAttackPower, Arrow_Attack, OldValue, NewValue);
}
void UUK_InvSystem::UpdateCurrentMp(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateCurrentMp, Arrow_CurrentMp, OldValue, NewValue); 
}
void UUK_InvSystem::UpdateMaxMp(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateMaxMp, Arrow_MaxMp, OldValue, NewValue);
}
void UUK_InvSystem::UpdateDefence(float OldValue, float NewValue)
{
	ProcessStatUpdate(StateDefence, Arrow_Defence, OldValue, NewValue);
}
void UUK_InvSystem::UpdateCriticalChance(float OldValue, float NewValue) 
{
	ProcessStatUpdate(StateCriticalChance, Arrow_CritChance, OldValue, NewValue); 
}
void UUK_InvSystem::UpdateCriticalDamage(float OldValue, float NewValue)
{
	ProcessStatUpdate(StateCriticalDamage, Arrow_CritDamage, OldValue, NewValue);
}
void UUK_InvSystem::UpdateMaxStamina(float OldValue, float NewValue)
{
	ProcessStatUpdate(StateMaxStamina, Arrow_Stamina, OldValue, NewValue);
}

void UUK_InvSystem::UpdateAttackWeapon(float OldValue, float NewValue)
{
	ProcessStatUpdate(StateAttackWeapon, Arrow_AttackWeapon, OldValue, NewValue);
}

void UUK_InvSystem::ProcessStatUpdate(UTextBlock* TargetText, UImage* ArrowImage, float OldValue, float NewValue)
{
	if (!TargetText) return;

	FSlateFontInfo FontInfo = TargetText->GetFont();

	if (OldValue <= 0.f)
	{
		FontInfo.Size = NormalFontSize;
		TargetText->SetFont(FontInfo);
		TargetText->SetColorAndOpacity(FSlateColor(NormalColor));

		FString NormalStr = FString::Printf(TEXT("[ %d ]"), FMath::FloorToInt(NewValue));
		TargetText->SetText(FText::FromString(NormalStr));

		if (ArrowImage) ArrowImage->SetVisibility(ESlateVisibility::Hidden);
		return; 
	}

	//실시간 수치 변화 감지
	bool bIsIncreased = (NewValue > OldValue);
	bool bIsDecreased = (NewValue < OldValue);

	//상승 회복, 레벨업 등
	if (bIsIncreased)
	{
		FontInfo.Size = ChangedFontSize;
		TargetText->SetFont(FontInfo);
		TargetText->SetColorAndOpacity(FSlateColor(IncreaseColor));

		FString UpdateStr = FString::Printf(TEXT("[ %d -> %d ]"), FMath::FloorToInt(OldValue), FMath::FloorToInt(NewValue));
		TargetText->SetText(FText::FromString(UpdateStr));

		if (ArrowImage)
		{
			ArrowImage->SetVisibility(ESlateVisibility::Visible);
			ArrowImage->SetRenderTransformAngle(0.f);
			ArrowImage->SetColorAndOpacity(IncreaseColor);
		}
	}
	//하락 피격, 마나 소비 등
	else if (bIsDecreased)
	{
		FontInfo.Size = ChangedFontSize;
		TargetText->SetFont(FontInfo);
		TargetText->SetColorAndOpacity(FSlateColor(DecreaseColor));

		FString UpdateStr = FString::Printf(TEXT("[ %d -> %d ]"), FMath::FloorToInt(OldValue), FMath::FloorToInt(NewValue));
		TargetText->SetText(FText::FromString(UpdateStr));

		if (ArrowImage)
		{
			ArrowImage->SetVisibility(ESlateVisibility::Visible);
			ArrowImage->SetRenderTransformAngle(180.f);
			ArrowImage->SetColorAndOpacity(DecreaseColor);
		}
	}
}

