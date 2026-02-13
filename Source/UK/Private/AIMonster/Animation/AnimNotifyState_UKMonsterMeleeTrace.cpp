#include "AIMonster/Animation/AnimNotifyState_UKMonsterMeleeTrace.h"
#include "AIMonster/AIMonsterBase.h"
#include "Character/UK_CharacterBase.h"
#include "DrawDebugHelpers.h"

UAnimNotifyState_UKMonsterMeleeTrace::UAnimNotifyState_UKMonsterMeleeTrace() {}

void UAnimNotifyState_UKMonsterMeleeTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Empty();
}

void UAnimNotifyState_UKMonsterMeleeTrace::NotifyTick(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = OwnerActor->GetWorld();
	if (!World || !OwnerActor->HasAuthority()) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(OwnerActor);
	if (!Monster) return;

	// 소켓 없이 전방 트레이스
	const FVector ActorLocation = OwnerActor->GetActorLocation();
	const FVector ActorForward = OwnerActor->GetActorForwardVector();
	const FVector TraceStart = ActorLocation + FVector(0, 0, TraceStartHeight);
	const FVector TraceEnd = TraceStart + ActorForward * TraceForwardLength;

	// 스윕 트레이스
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;

	FCollisionShape SweepShape = FCollisionShape::MakeSphere(TraceRadius);

	TArray<FHitResult> HitResults;
	bool bHit = World->SweepMultiByChannel(
		HitResults, TraceStart, TraceEnd,
		FQuat::Identity, ECC_Pawn, SweepShape, QueryParams);

	// 디버그: 캡슐만 (초록=미스, 빨강=히트, 노랑=피격점)
	if (bShowDebug)
	{
		FColor DrawColor = bHit ? FColor::Red : FColor::Green;
		FVector Center = (TraceStart + TraceEnd) * 0.5f;
		float HalfHeight = FVector::Dist(TraceStart, TraceEnd) * 0.5f + TraceRadius;
		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();

		DrawDebugCapsule(World, Center, HalfHeight, TraceRadius,
			CapsuleRot, DrawColor, false, DebugDrawDuration, 0, 3.f);
	}

	if (!bHit) return;

	// 플레이어(UK_CharacterBase)만 히트 처리
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerActor) continue;

		// 플레이어만 통과, 나머지 전부 무시
		AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor);
		if (!Player) continue;

		// 중복 히트 방지
		if (HitActors.Contains(HitActor)) continue;
		HitActors.Add(HitActor);

		// 피격 디버그 (노란 구체만)
		if (bShowDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 20.f, 12,
				FColor::Yellow, false, DebugDrawDuration, 0, 3.f);
		}

		Player->ReceiveDamage(Monster->AttackDamage);
	}
}

void UAnimNotifyState_UKMonsterMeleeTrace::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}