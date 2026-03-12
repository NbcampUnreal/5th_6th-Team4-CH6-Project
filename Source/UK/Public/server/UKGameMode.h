#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AIMonster/AIMonsterBase.h"
#include "UKGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS()
class AUKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;

	virtual void BeginPlay() override;

	/** Constructor */
	AUKGameMode();
	
#pragma region Reward System
	
	/* 몬스터 보상 데이터 테이블 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Rewards")
	UDataTable* MonsterRewardTable;

	/* 몬스터 킬 알림 받는 함수 */
	UFUNCTION()
	void OnMonsterKilled(
		AAIMonsterBase* KilledMonster,
		EMonsterType MonsterType,
		APlayerController* KillerController
	);

	/* 보상 지급 (데이터 테이블 기반) */
	void GiveRewardToPlayer(
		APlayerController* PlayerController,
		EMonsterType MonsterType
	);

	/* 몬스터 등록 */
	UFUNCTION()
	void RegisterMonster(AAIMonsterBase* Monster);

	/* MonsterType → Row Name 변환 */
	FName GetRowNameFromMonsterType(EMonsterType MonsterType);

public:
	/* Spawner가 몬스터 활성화 시 호출 (Blueprint 호출 가능) */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void OnMonsterSpawned(AAIMonsterBase* Monster);
	
#pragma endregion 
	
};