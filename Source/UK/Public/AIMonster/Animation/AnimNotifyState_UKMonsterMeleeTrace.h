#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_UKMonsterMeleeTrace.generated.h"

class USoundCue;

/**
 * 몬스터 근접 공격 트레이스
 */
UCLASS(DisplayName = "Monster Melee Trace")
class UK_API UAnimNotifyState_UKMonsterMeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()

#pragma region Notify Events
public:
	UAnimNotifyState_UKMonsterMeleeTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("MeleeTrace"); }
#pragma endregion

#pragma region Trace Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceForwardLength = 150.f;

	// 여러 높이에서 트레이스 (캡슐 중심 기준 오프셋)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TArray<float> TraceHeightOffsets = {-30.f, 20.f, 70.f};
	
#pragma endregion

#pragma region Debug Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDrawDuration = 2.f;
#pragma endregion

#pragma region Private
private:
	UPROPERTY()
	TArray<AActor*> HitActors;
#pragma endregion
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds | attack")
	USoundCue* AttackSound;
};