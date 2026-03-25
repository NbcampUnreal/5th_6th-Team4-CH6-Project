#pragma once
#include "Engine/DataTable.h"
#include "UK_MonsterStatRow.generated.h"

USTRUCT(BlueprintType)
struct FUK_MonsterStatRow : public FTableRowBase
{
	GENERATED_BODY()

	// ── HP ─────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HP")
	float BaseHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|HP")
	float HPPerLevel = 10.f;

	// ── 공격력 ─────────────────────────────────
	/** 기본 공격력 (레벨 무관 고정값) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack")
	float BaseAttack = 5.f; 
	
	/** 일반 공격: PlayerLevel × AttackMultiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack")
	float AttackMultiplier = 3.14f;

	/** 광역 공격: 일반 공격 × AoEMultiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack")
	float AoEMultiplier = 1.5f;

	// ── 방어력 ─────────────────────────────────
	/** 방어력 = (PlayerLevel / 2) + BaseDefense */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense")
	float BaseDefense = 5.f;

	// ── 전투 행동 ──────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Combat")
	float AttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Combat")
	float AttackCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Combat")
	float DetectionRadius = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Combat")
	float MaxChaseDistance = 1500.f;
	
	// ── 보상 ────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Reward")
	float ExpPercent = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Reward")
	float BaseGoldMin = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Reward")
	float BaseGoldMax = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Reward")
	float GoldPerLevel = 5.f;
};