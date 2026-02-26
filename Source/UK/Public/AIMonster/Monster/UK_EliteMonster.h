#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_EliteMonster.generated.h"

/**
 * 엘리트 몬스터 베이스
 *
 * ■ 웨이포인트 순찰 : 에디터에서 PatrolWaypoints 배열에 AActor* 를 배치하면 순서대로 순찰(루프).
 *                     비어있으면 기존 PatrolRadius 기반 랜덤 순찰로 폴백.
 * ■ 특수 공격      : SpecialAttackMontages 재생. 데미지는 몽타주에 붙은
 *                     AnimNotifyState_UKMonsterMeleeTrace 가 기존과 동일하게 처리.
 */
UCLASS(Abstract)
class UK_API AUK_EliteMonster : public AAIMonsterBase
{
	GENERATED_BODY()

public:
	AUK_EliteMonster();

	//  특수 공격

	/** 특수 공격 몽타주 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|SpecialAttack")
	TArray<UAnimMontage*> SpecialAttackMontages;

	/** 특수 공격 쿨다운 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|SpecialAttack")
	float SpecialAttackCooldown = 8.0f;

	/** 특수 공격 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "Elite|SpecialAttack")
	bool CanUseSpecialAttack() const;

	/**
	 * 특수 공격 실행 (서버 전용).
	 * 몽타주 재생(Multicast)만 담당. 데미지는 MeleeTrace 노티파이가 처리.
	 * @return 실행 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Elite|SpecialAttack")
	bool PlaySpecialAttack();

	DECLARE_DELEGATE_OneParam(FOnSpecialAttackFinished, bool /*bSucceeded*/);
	FOnSpecialAttackFinished OnSpecialAttackFinished;

	float LastSpecialAttackTime = 0.f;

	UFUNCTION()
	void OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialAttackMontage(int32 MontageIndex);

	//  웨이포인트 순찰

	/**
	 * 에디터에서 레벨에 배치한 액터를 순찰 웨이포인트로 지정.
	 * 비어있으면 PatrolRadius 기반 랜덤 순찰 사용.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Elite|Patrol")
	TArray<AActor*> PatrolWaypoints;

	/** 현재 목표 웨이포인트 인덱스 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Elite|Patrol")
	int32 CurrentWaypointIndex = 0;

	UFUNCTION(BlueprintPure, Category = "Elite|Patrol")
	bool HasWaypoints() const { return PatrolWaypoints.Num() > 0; }

	/**
	 * 다음 웨이포인트 위치 반환 + 인덱스 증가 (루프).
	 */
	UFUNCTION(BlueprintCallable, Category = "Elite|Patrol")
	FVector GetNextWaypointLocation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
#pragma region debug
	/** true 시 특수 공격 발동 때 범위 캡슐/원 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug")
	bool bShowSpecialAttackDebug = true;

	/** 바닥 원형 반지름 — 몽타주 MeleeTrace의 TraceForwardLength 와 맞춰 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugRadius = 350.0f;

	/** 전방 캡슐 시작 높이 (몽타주 MeleeTrace의 TraceStartHeight 와 맞춰 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceHeight = 60.0f;

	/** 전방 캡슐 길이 (몽타주 MeleeTrace의 TraceForwardLength 와 맞춰 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceLength = 300.0f;

	/** 전방 캡슐 반지름 (몽타주 MeleeTrace의 TraceRadius 와 맞춰 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugTraceRadius = 80.0f;

	/** 디버그 도형 표시 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite|Debug",
		meta = (EditCondition = "bShowSpecialAttackDebug"))
	float SpecialAttackDebugDuration = 1.5f;

#pragma endregion 
};