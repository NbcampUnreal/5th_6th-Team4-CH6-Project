#include "AIMonster/Animation/AnimNotifyState_UKMonsterMeleeTrace.h"
#include "AIMonster/AIMonsterBase.h"
#include "Character/UK_CharacterBase.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UAnimNotifyState_UKMonsterMeleeTrace::UAnimNotifyState_UKMonsterMeleeTrace() {}

#pragma region Notify Events
void UAnimNotifyState_UKMonsterMeleeTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Empty();
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

	if (!MeshComp || !MeshComp->GetOwner())	return;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = OwnerActor->GetWorld();

	if (!World)	return;
	
	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(OwnerActor);
	if (!Monster)
	{
		return;
	}

	// ── 전방 구체 스윕  ─────────────────
	float CapsuleHalf = 90.f;
	if (ACharacter* Char = Cast<ACharacter>(OwnerActor))
	{
		if (UCapsuleComponent* Cap = Char->GetCapsuleComponent())
		{
			CapsuleHalf = Cap->GetScaledCapsuleHalfHeight();
		}
	}
	const FVector TraceStart = OwnerActor->GetActorLocation() + FVector(0, 0, CapsuleHalf * 0.1f);
	const FVector TraceEnd   = TraceStart + OwnerActor->GetActorForwardVector() * TraceForwardLength;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);

	TArray<FHitResult> HitResults;
	const bool bHit = World->SweepMultiByObjectType(
		HitResults, TraceStart, TraceEnd,
		FQuat::Identity, ObjectQueryParams,
		FCollisionShape::MakeSphere(TraceRadius), QueryParams);

#pragma region Debug
	if (bShowDebug)
	{
		const FColor DrawColor = bHit ? FColor::Red : FColor::Green;
		const FVector Center   = (TraceStart + TraceEnd) * 0.5f;
		const float HalfHeight = FVector::Dist(TraceStart, TraceEnd) * 0.5f + TraceRadius;
		const FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();
		DrawDebugCapsule(World, Center, HalfHeight, TraceRadius,
			CapsuleRot, DrawColor, false, DebugDrawDuration, 0, 3.f);
	}
#pragma endregion

	if (!bHit) return;

	// ── 히트 처리: 플레이어(UK_CharacterBase)만 대상 ────────────────────
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerActor) continue;

		AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor);
		if (!Player) continue;

		if (HitActors.Contains(HitActor)) continue;
		HitActors.Add(HitActor);

#pragma region Debug
		if (bShowDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 20.f, 12,
				FColor::Yellow, false, DebugDrawDuration, 0, 3.f);
		}
#pragma endregion

		UE_LOG(LogTemp, Warning,
			TEXT("[MeleeTrace] %s → %s | Damage: %.1f | ImpactPoint: %s"),
			*Monster->GetName(),
			*Player->GetName(),
			Monster->AttackDamage,
			*Hit.ImpactPoint.ToString());

		Player->ReceiveDamage(Monster->AttackDamage);
	}
}
#pragma endregion