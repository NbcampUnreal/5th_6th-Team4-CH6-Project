#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_UKMonsterMeleeTrace.generated.h"

/**
 * 몬스터 근접 공격 트레이스
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

	/** 트레이스 시작 높이 오프셋 (액터 위치 기준 위로) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceStartHeight = 60.f;

	/** 전방 트레이스 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceForwardLength = 200.f;

	/** 스윕 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 50.f;

	/** 디버그 캡슐 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = true;

	/** 디버그 지속 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDrawDuration = 0.5f;

private:
	UPROPERTY()
	TArray<AActor*> HitActors;
};