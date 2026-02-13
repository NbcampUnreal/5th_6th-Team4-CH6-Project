#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_UKMonsterMeleeTrace.generated.h"

/**
 * 공격 애니메이션 구간에서 스윕 트레이스를 수행하는 노티파이 스테이트
 *
 * NotifyBegin : 히트 목록 초기화
 * NotifyTick  : 매 프레임 SweepMulti → 히트 시 데미지 적용
 * NotifyEnd   : 정리
 */
UCLASS(DisplayName = "Monster Melee Trace")
class UK_API UAnimNotifyState_UKMonsterMeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_UKMonsterMeleeTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("MeleeTrace"); }

	/* 트레이스 설정 (몽타주 에디터에서 조절 가능) */

	/** 트레이스 시작 소켓 (무기 끝 or 손) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName TraceStartSocket = TEXT("Hand_R");

	/** 트레이스 끝 소켓 (없으면 전방으로 TraceForwardLength만큼) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName TraceEndSocket = TEXT("Hand_R_End");

	/** 끝 소켓이 없을 때 전방 트레이스 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceForwardLength = 100.f;

	/** 스윕 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 30.f;

	/** 디버그 드로우 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = true;

	/** 디버그 지속 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDrawDuration = 1.0f;

private:
	/** 이번 공격에서 이미 맞은 액터 (중복 히트 방지) */
	UPROPERTY()
	TArray<AActor*> HitActors;

	void GetTraceLocations(USkeletalMeshComponent* MeshComp, FVector& OutStart, FVector& OutEnd) const;
};