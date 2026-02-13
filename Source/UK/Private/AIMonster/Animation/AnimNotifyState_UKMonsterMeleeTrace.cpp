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

	// 트레이스 위치 계산
	FVector TraceStart, TraceEnd;
	GetTraceLocations(MeshComp, TraceStart, TraceEnd);

	// 스윕 트레이스 수행
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.bTraceComplex = false;

	FCollisionShape SweepShape = FCollisionShape::MakeSphere(TraceRadius);

	TArray<FHitResult> HitResults;
	bool bHit = World->SweepMultiByChannel(
		HitResults, TraceStart, TraceEnd,
		FQuat::Identity, ECC_Pawn, SweepShape, QueryParams);

	// 디버그 드로우
	if (bShowDebug)
	{
		FColor DrawColor = bHit ? FColor::Red : FColor::Green;
		FVector Center = (TraceStart + TraceEnd) * 0.5f;
		float HalfHeight = FVector::Dist(TraceStart, TraceEnd) * 0.5f + TraceRadius;
		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();

		DrawDebugCapsule(World, Center, HalfHeight, TraceRadius,
			CapsuleRot, DrawColor, false, DebugDrawDuration, 0, 1.5f);
		DrawDebugSphere(World, TraceStart, 8.f, 8, FColor::Cyan, false, DebugDrawDuration);
		DrawDebugSphere(World, TraceEnd, 8.f, 8, FColor::Magenta, false, DebugDrawDuration);
	}

	if (!bHit) return;

	// 히트 처리
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerActor) continue;

		// 몬스터끼리 안 때림
		if (Cast<AAIMonsterBase>(HitActor)) continue;

		// 중복 히트 방지
		if (HitActors.Contains(HitActor)) continue;
		HitActors.Add(HitActor);

		// 피격 디버그
		if (bShowDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 15.f, 12,
				FColor::Yellow, false, DebugDrawDuration * 2.f, 0, 2.f);
			DrawDebugString(World, Hit.ImpactPoint + FVector(0, 0, 30),
				FString::Printf(TEXT("HIT: %s\nBone: %s\nDmg: %.0f"),
					*HitActor->GetName(), *Hit.BoneName.ToString(), Monster->AttackDamage),
				nullptr, FColor::Yellow, DebugDrawDuration * 2.f, true);
		}

		UE_LOG(LogTemp, Warning,
			TEXT("[MeleeTrace] %s -> %s | Bone: %s | Dmg: %.1f"),
			*OwnerActor->GetName(), *HitActor->GetName(),
			*Hit.BoneName.ToString(), Monster->AttackDamage);

		// 데미지 적용
		if (AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(HitActor))
		{
			Player->ReceiveDamage(Monster->AttackDamage);
		}
	}
}

void UAnimNotifyState_UKMonsterMeleeTrace::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}

void UAnimNotifyState_UKMonsterMeleeTrace::GetTraceLocations(
	USkeletalMeshComponent* MeshComp, FVector& OutStart, FVector& OutEnd) const
{
	if (!MeshComp) return;

	// 시작 소켓
	OutStart = MeshComp->DoesSocketExist(TraceStartSocket)
		? MeshComp->GetSocketLocation(TraceStartSocket)
		: MeshComp->GetOwner()->GetActorLocation() + FVector(0, 0, 50.f);

	// 끝 소켓
	if (MeshComp->DoesSocketExist(TraceEndSocket))
	{
		OutEnd = MeshComp->GetSocketLocation(TraceEndSocket);
	}
	else
	{
		// 끝 소켓이 없으면 액터 전방으로 TraceForwardLength만큼
		OutEnd = OutStart + MeshComp->GetOwner()->GetActorForwardVector() * TraceForwardLength;
	}
}