// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_NormalMonster.generated.h"

/**
 * 일반 몬스터 클래스
 * AIMonsterBase를 상속받아 메쉬와 기본 설정만 추가
 */
UCLASS()
class UK_API AUK_NormalMonster : public AAIMonsterBase
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AUK_NormalMonster();

protected:
	virtual void BeginPlay() override;
#pragma endregion
};