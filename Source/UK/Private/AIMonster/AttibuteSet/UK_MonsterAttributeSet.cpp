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
		
		if (OldValue != NewValue)
		{
			UE_LOG(LogTemp, Log, TEXT("[AttributeSet] PreAttributeChange - Health clamped: %.1f → %.1f"), 
				OldValue, NewValue);
		}
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
		const float FinalDamage = GetDamage();
		
		UE_LOG(LogTemp, Warning, TEXT(" [%s] AttributeSet::PostGameplayEffectExecute"), *MonsterName);
		UE_LOG(LogTemp, Warning, TEXT(" Final Damage (after Defense): %.1f"), FinalDamage);
		UE_LOG(LogTemp, Warning, TEXT(" Current Health: %.1f / %.1f"), GetHealth(), GetMaxHealth());
		
		SetDamage(0.0f);

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
				Monster->NotifyAttacked(InstigatorController); 
				
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
			}
			
			UE_LOG(LogTemp, Warning, TEXT(" Health Updated: %.1f → %.1f (Damage: %.1f)"), 
				OldHealth, NewHealth, FinalDamage);

			if (NewHealth <= 0.0f)
			{
				UE_LOG(LogTemp, Error, TEXT(" Health Reached 0 → Calling Die()"));
				HandleOutOfHealth();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT(" Monster Still Alive (%.1f HP remaining)"), NewHealth);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Damage was 0 - No health change"));
		}
	}
	// Health가 직접 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float OldHealth = GetHealth();
		
		UE_LOG(LogTemp, Log, TEXT("[%s] AttributeSet - Health Directly Changed"), *MonsterName);
		
		// MaxHealth를 초과하지 않도록
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		UE_LOG(LogTemp, Log, TEXT("   Health: %.1f → %.1f (Max: %.1f)"), 
			OldHealth, GetHealth(), GetMaxHealth());
		
		if (GetHealth() <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT(" Health Reached 0 → Calling Die()"));
			HandleOutOfHealth();
		}
	}
	// MaxHealth가 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		const float OldHealth = GetHealth();
		
		UE_LOG(LogTemp, Log, TEXT("[%s] AttributeSet - MaxHealth Changed to %.1f"), 
			*MonsterName, GetMaxHealth());
		
		// 현재 체력이 새 최대값을 초과하지 않도록
		SetHealth(FMath::Min(GetHealth(), GetMaxHealth()));
		
		if (OldHealth != GetHealth())
		{
			UE_LOG(LogTemp, Log, TEXT("   Health adjusted: %.1f → %.1f"), OldHealth, GetHealth());
		}
	}
	// 방어력이 변경된 경우
	else if (Data.EvaluatedData.Attribute == GetDefenseAttribute())
	{
		const float NewDef = FMath::Max(0.f, GetDefense());
		SetDefense(NewDef);
		UE_LOG(LogTemp, Log, TEXT("[%s] AttributeSet - Defense Changed to %.1f"), *MonsterName, NewDef);
	}
	else
	{
		// 다른 속성 변경 - 디버깅용
		UE_LOG(LogTemp, Verbose, TEXT("[%s] AttributeSet - Other Attribute Changed: %s"), 
			*MonsterName, *Data.EvaluatedData.Attribute.GetName());
	}
	
	//보스 체력비례 페이즈 체크용
	if (AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(GetOwningActor()))
	{
		Boss->UpdatePhase();
	}
}

void UUK_MonsterAttributeSet::HandleOutOfHealth()
{
	UE_LOG(LogTemp, Error, TEXT("HandleOutOfHealth() Called"));
	
	// Owner가 몬스터인지 확인
	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(GetOwningActor()))
	{
		UE_LOG(LogTemp, Error, TEXT(" Monster Cast Successful: %s"), *Monster->GetName());
		UE_LOG(LogTemp, Error, TEXT("Calling Monster->Die()..."));
		
		Monster->Die();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to cast OwningActor to AAIMonsterBase!"));
		if (AActor* Owner = GetOwningActor())
		{
			UE_LOG(LogTemp, Error, TEXT("   OwningActor: %s (Class: %s)"), 
				*Owner->GetName(), *Owner->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("   OwningActor is nullptr!"));
		}
	}
}