#include "AIMonster/Component/AI_MonsterStatComponent.h"

UAI_MonsterStatComponent::UAI_MonsterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAI_MonsterStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	
	UE_LOG(LogTemp, Log, TEXT("[StatComp] %s: BeginPlay - HP initialized to %.1f"), 
		*GetOwner()->GetName(), CurrentHP);
}

void UAI_MonsterStatComponent::TakeDamage(float Damage)
{
	if (Damage <= 0.f) return;
	
	float OldHP = CurrentHP;
	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);
	
	UE_LOG(LogTemp, Warning, TEXT("[StatComp] %s: TakeDamage %.1f → %.1f (Damage: %.1f)"),
		*GetOwner()->GetName(), OldHP, CurrentHP, Damage);
	
	if (OldHP > 0.f && CurrentHP <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[StatComp] %s: Broadcasting OnDeath (%.1f → %.1f)"),
			*GetOwner()->GetName(), OldHP, CurrentHP);
		
		OnDeath.Broadcast();
	}
	else if (CurrentHP <= 0.f)
	{
		// 이미 죽어있는 상태에서 또 데미지를 받음
		UE_LOG(LogTemp, Warning, TEXT("[StatComp] %s: Already dead, NOT broadcasting (HP: %.1f)"),
			*GetOwner()->GetName(), CurrentHP);
	}
}

void UAI_MonsterStatComponent::SetHP(float NewHP)
{
	float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(NewHP, 0.f, MaxHP);

	OnHPChanged.Broadcast(CurrentHP);

	UE_LOG(LogTemp, Log, TEXT("[StatComp] %s: SetHP %.1f → %.1f"),
		*GetOwner()->GetName(), OldHP, CurrentHP);
}
