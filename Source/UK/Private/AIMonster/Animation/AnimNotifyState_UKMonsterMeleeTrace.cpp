// AnimNotifyState_UKMonsterMeleeTrace.cpp
#include "AIMonster/Animation/AnimNotifyState_UKMonsterMeleeTrace.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DataAsset/DataTable/AIMonster/UK_MonsterCombatRow.h"
#include "Tags/UK_GameplayTags.h"

UAnimNotifyState_UKMonsterMeleeTrace::UAnimNotifyState_UKMonsterMeleeTrace() {}

#pragma region Notify Events
void UAnimNotifyState_UKMonsterMeleeTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Empty();
	CachedHitType = EHitReactionType::None;

	if (AttackSound)
		UGameplayStatics::PlaySound2D(MeshComp->GetWorld(), AttackSound);
		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(MeshComp->GetOwner());
	if (!Monster || !Monster->MonsterCombatTable) return;
	
	// "EMonsterType::Wolf" → "Wolf"
	FString EnumStr = UEnum::GetValueAsString(Monster->MonsterType);
	FString RowStr;
	EnumStr.Split(TEXT("::"), nullptr, &RowStr);

	if (FUK_MonsterCombatRow* Row = Monster->MonsterCombatTable->FindRow<FUK_MonsterCombatRow>(FName(*RowStr), TEXT("")))
	{
		CachedHitType = (AttackType == EMonsterAttackType::Normal)
			? Row->NormalAttackHit
			: Row->SpecialAttackHit;
	}
}

void UAnimNotifyState_UKMonsterMeleeTrace::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}
#pragma endregion

#pragma region Hit Detection
void UAnimNotifyState_UKMonsterMeleeTrace::NotifyTick(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = OwnerActor->GetWorld();
	if (!World) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(OwnerActor);
	if (!Monster) return;

	// ── 캡슐 높이 가져오기 ───────────────────────────────────────
	float CapsuleHalf = 90.f;
	if (ACharacter* Char = Cast<ACharacter>(OwnerActor))
	{
		if (UCapsuleComponent* Cap = Char->GetCapsuleComponent())
		{
			CapsuleHalf = Cap->GetScaledCapsuleHalfHeight();
		}
	}

	const FVector BaseLocation = OwnerActor->GetActorLocation();
	const FVector ForwardVector = OwnerActor->GetActorForwardVector();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);

	// ── 여러 높이에서 트레이스 수행 ────────────────────────────────
	bool bAnyHit = false;

	for (float HeightOffset : TraceHeightOffsets)
	{
		const FVector TraceStart = BaseLocation + FVector(0, 0, HeightOffset);
		const FVector TraceEnd = TraceStart + ForwardVector * TraceForwardLength;

		TArray<FHitResult> HitResults;
		const bool bHit = World->SweepMultiByObjectType(
			HitResults, TraceStart, TraceEnd,
			FQuat::Identity, ObjectQueryParams,
			FCollisionShape::MakeSphere(TraceRadius), QueryParams);

		if (bHit) bAnyHit = true;

		if (!bHit) continue;

		// ── 히트 처리: 플레이어(UK_CharacterBase)만 대상 ─────────────
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == OwnerActor) continue;

			AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor);
			if (!Player) continue;

			// 중복 히트 방지
			if (HitActors.Contains(HitActor)) continue;
			HitActors.Add(HitActor);

			// ── GAS 데미지 처리 ──────────────────────────────────────
			if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Player))
			{
				if (UAbilitySystemComponent* PlayerASC = ASCInterface->GetAbilitySystemComponent())
				{
					bool bIsPlayerParrying = PlayerASC->HasMatchingGameplayTag(UK_GameplayTags::Action::Parrying);
					
					bool bIsBoss = (Monster->MonsterType == EMonsterType::Grux);
					bool bIsElite = (Monster->MonsterType == EMonsterType::EliteGolem
								  || Monster->MonsterType == EMonsterType::EliteWolf
								  || Monster->MonsterType == EMonsterType::EliteInsectBeast);
					bool bCanParry = bIsBoss || (bIsElite && AttackType == EMonsterAttackType::Special);
					
					// 패리 체크
					if (bIsPlayerParrying && bCanParry)
					{
						UE_LOG(LogTemp, Warning,
							TEXT("[MeleeTrace] %s → %s : PARRIED! Attack cancelled."),
							*Monster->GetName(), *Player->GetName());

						// 몬스터에게 Parry 이벤트
						FGameplayEventData ParriedPayload;
						ParriedPayload.Instigator = Player;
						ParriedPayload.Target = Monster;
						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
							Monster,
							UK_GameplayTags::Action::Parry,
							ParriedPayload
						);

						return;
					}

					// 데미지 계산 및 적용
					const UUK_PlayerStatusAttributeSet* AttrSet = PlayerASC->GetSet<UUK_PlayerStatusAttributeSet>();
					const float Defence = AttrSet ? AttrSet->GetDefence() : 0.f;
					const float FinalDamage = FMath::Max(Monster->AttackDamage - Defence, 0.f);

					if (FinalDamage > 0.f)
					{
						UE_LOG(LogTemp, Warning,
							TEXT("[Player] Hit | Raw: %.1f | Defence: %.1f | Final: %.1f | HP: %.1f -> %.1f"),
							Monster->AttackDamage, Defence, FinalDamage,
							AttrSet->GetHealth(), AttrSet->GetHealth() - FinalDamage);

						PlayerASC->SetNumericAttributeBase(
							UUK_PlayerStatusAttributeSet::GetDamageAttribute(), FinalDamage);
					}
					
					FGameplayEventData EventData;
					EventData.EventMagnitude = (float)CachedHitType;
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
						Player,
						UK_GameplayTags::Action::BeAttacked,
						EventData);
				}
			}
		}
	}
}
#pragma endregion