#include "UI/InGame/UK_FloatingDamage.h"
#include "Components/TextBlock.h"

void UUK_FloatingDamage::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageWidget] NativeConstruct"));

	RefreshDamageText();
}

void UUK_FloatingDamage::SetDamageAmount(float InDamage)
{
	DamageAmount = InDamage;

	UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageWidget] SetDamageAmount = %.1f"), DamageAmount);

	RefreshDamageText();

	if ( Damage )
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageWidget] SetDamageAmount PlayAnimation"));
		PlayAnimationForward(Damage, 1.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageWidget] Damage animation is null in SetDamageAmount"));
	}
}

void UUK_FloatingDamage::RefreshDamageText()
{
	if ( FloatingDamage_Text )
	{
		const int32 DisplayDamage = FMath::RoundToInt(DamageAmount);
		FloatingDamage_Text->SetText(FText::AsNumber(DisplayDamage));

		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageWidget] RefreshDamageText = %d"), DisplayDamage);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[FloatingDamageWidget] FloatingDamage_Text is null"));
	}
}