// AnimNotifyState_UKMonsterMeleeTrace.cpp
#include "AIMonster/Animation/AnimNotifyState_UKMonsterMeleeTrace.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_MonsterTypes.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
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
	CachedMonster = nullptr;
	bCachedCanParry = false;
	
	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(MeshComp->GetOwner());
	if (!Monster) return;
	CachedMonster = Monster;

	const bool bIsBoss  = (Monster->MonsterType == EMonsterType::Grux);
	const bool bIsElite = (Monster->MonsterType == EMonsterType::EliteGolem
						|| Monster->MonsterType == EMonsterType::EliteWolf
						|| Monster->MonsterType == EMonsterType::EliteInsectBeast);
	bCachedCanParry = bIsBoss || (bIsElite && AttackType == EMonsterAttackType::Special);
	
	if (AttackSound)
		UGameplayStatics::PlaySound2D(MeshComp->GetWorld(), AttackSound);
	
	if (!Monster->MonsterCombatTable) return;
	FString EnumStr = UEnum::GetValueAsString(Monster->MonsterType);
	FString RowStr;
	EnumStr.Split(TEXT("::"), nullptr, &RowStr);
	if (FUK_MonsterCombatRow* Row = Monster->MonsterCombatTable->FindRow<FUK_MonsterCombatRow>(FName(*RowStr), TEXT("")))
	{
		CachedHitType = (AttackType == EMonsterAttackType::Normal)
			? Row->NormalAttackHit : Row->SpecialAttackHit;
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

	if (!MeshComp || !CachedMonster) return;
	if (HitActors.Num() > 0) return;

	UWorld* World = CachedMonster->GetWorld();
	if (!World) return;

	const FVector BaseLocation  = CachedMonster->GetActorLocation();
	const FVector ForwardVector = CachedMonster->GetActorForwardVector();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedMonster);
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	 for (float HeightOffset : TraceHeightOffsets)
    {
        const FVector TraceStart = BaseLocation + FVector(0, 0, HeightOffset);
        const FVector TraceEnd   = TraceStart + ForwardVector * TraceForwardLength;

        TArray<FHitResult> HitResults;
        if (!World->SweepMultiByObjectType(HitResults, TraceStart, TraceEnd,
            FQuat::Identity, ObjectQueryParams,
            FCollisionShape::MakeSphere(TraceRadius), QueryParams)) continue;

        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor || HitActors.Contains(HitActor)) continue;

            AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor);
            if (!Player) continue;

            HitActors.Add(HitActor);

            if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Player))
            {
                UAbilitySystemComponent* PlayerASC = ASCInterface->GetAbilitySystemComponent();
                if (!PlayerASC) continue;

                if (PlayerASC->HasMatchingGameplayTag(UK_GameplayTags::Action::Parrying) && bCachedCanParry)
                {
                    FGameplayEventData ParriedPayload;
                    ParriedPayload.Instigator = Player;
                    ParriedPayload.Target     = CachedMonster;
                    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                        CachedMonster, UK_GameplayTags::Action::Parry, ParriedPayload);
                    return;
                }

                const UUK_PlayerStatusAttributeSet* AttrSet = PlayerASC->GetSet<UUK_PlayerStatusAttributeSet>();
                const float Defence     = AttrSet ? AttrSet->GetDefence() : 0.f;
                const float FinalDamage = FMath::Max(CachedMonster->AttackDamage - Defence, 0.f);

                if (FinalDamage > 0.f)
                {
                    PlayerASC->SetNumericAttributeBase(
                        UUK_PlayerStatusAttributeSet::GetDamageAttribute(), FinalDamage);
                }

                FGameplayEventData EventData;
                EventData.EventMagnitude = (float)CachedHitType;
                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                    Player, UK_GameplayTags::Action::BeAttacked, EventData);
            }
        }
    }
}
#pragma endregion