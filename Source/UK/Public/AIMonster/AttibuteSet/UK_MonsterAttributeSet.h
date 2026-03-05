#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "UK_MonsterAttributeSet.generated.h"

/**
 * 몬스터 Attribute Set
 */
UCLASS()
class UK_API UUK_MonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UUK_MonsterAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	ATTRIBUTE_ACCESSORS_BASIC(UUK_MonsterAttributeSet, Health);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_MonsterAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_MonsterAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UUK_MonsterAttributeSet, Damage);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData AttackPower;

	// 받은 데미지
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Damage;

private:
	void HandleOutOfHealth();
};