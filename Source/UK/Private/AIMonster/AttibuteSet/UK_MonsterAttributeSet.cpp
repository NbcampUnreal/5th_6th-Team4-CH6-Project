#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameplayEffectExtension.h"

UUK_MonsterAttributeSet::UUK_MonsterAttributeSet()
{
}

void UUK_MonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health는 0 ~ MaxHealth 사이로 제한
	if (Attribute == GetHealthAttribute())
	{
		const float OldValue = NewValue;
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UUK_MonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(GetOwningActor());
	const FString MonsterName = Monster ? Monster->GetName() : TEXT("Unknown");

	// Damage
	// ※ 방어력 차감은 UK_MonsterDamageExecutionCalculation 에서 처리됨
	//   여기서 받는 값은 이미 방어력이 적용된 최종 데미지
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float RawDamage = GetDamage();
		
		SetDamage(0.0f);
		
		const float MonsterDefense = FMath::Max(GetDefense(), 0.f);
		const float FinalDamage = FMath::Max(RawDamage / (1.f + MonsterDefense), 0.f);

		UE_LOG(LogTemp, Warning, TEXT("[Monster] RawDmg=%.1f | DEF=%.1f | FinalDmg=%.1f | HP=%.1f"),
			RawDamage, MonsterDefense, FinalDamage, GetHealth());
		
		if (FinalDamage > 0.0f)
		{
			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Max(0.0f, GetHealth() - FinalDamage);
			SetHealth(NewHealth);
			if (Monster)
			{
				AController* InstigatorController = nullptr;
				if (Data.EffectSpec.GetContext().GetInstigator())
				{
					APawn* InstigatorPawn = Cast<APawn>(Data.EffectSpec.GetContext().GetInstigator());
					if (InstigatorPawn)
					{
						InstigatorController = InstigatorPawn->GetController();
					}
				}
				FVector HitLocation = FVector::ZeroVector;
				if (const FHitResult* HitResult = Data.EffectSpec.GetContext().GetHitResult())
				{
					HitLocation = HitResult->ImpactPoint;
				}
				Monster->PlayHitEffect(HitLocation); 
				Monster->NotifyAttacked(InstigatorController);
				Monster->SpawnFloatingDamage(FinalDamage);
			}
			
			if (NewHealth <= 0.0f)
			{
				HandleOutOfHealth();
			}
		}
	}
	// Health가 직접 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float OldHealth = GetHealth();
		
		// MaxHealth를 초과하지 않도록
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		if (GetHealth() <= 0.0f)
		{
			HandleOutOfHealth();
		}
	}
	// MaxHealth가 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		const float OldHealth = GetHealth();
		
		// 현재 체력이 새 최대값을 초과하지 않도록
		SetHealth(FMath::Min(GetHealth(), GetMaxHealth()));
	}
	// 방어력이 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetDefenseAttribute())
	{
		const float NewDef = FMath::Max(0.f, GetDefense());
		SetDefense(NewDef);
	}
	
	//보스 체력비례 페이즈 체크용
	if (AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(GetOwningActor()))
	{
		Boss->UpdatePhase();
	}
}

void UUK_MonsterAttributeSet::HandleOutOfHealth()
{
	// Owner가 몬스터인지 확인
	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(GetOwningActor()))
	{
		Monster->Die();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to cast OwningActor to AAIMonsterBase!"));
	}
}