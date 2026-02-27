#include "AIMonster/Component/AI_MonsterStatComponent.h"

#pragma region Initialization
UAI_MonsterStatComponent::UAI_MonsterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAI_MonsterStatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}
#pragma endregion

#pragma region Damage & HP
void UAI_MonsterStatComponent::TakeDamage(float Damage)
{
	if (Damage <= 0.f) return;

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);

	if (OldHP > 0.f && CurrentHP <= 0.f)
	{
		OnDeath.Broadcast();
	}
}

void UAI_MonsterStatComponent::SetHP(float NewHP)
{
	CurrentHP = FMath::Clamp(NewHP, 0.f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP);
}
#pragma endregion