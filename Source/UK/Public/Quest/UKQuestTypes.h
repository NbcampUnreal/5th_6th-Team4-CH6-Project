#pragma once

#include "CoreMinimal.h"
#include "UKQuestTypes.generated.h"

/*
퀘스트 관련, 데이터 타입(구조체) 정의 파일입니다.

ex: FQuestProgress

Step(단계)
bCompleted(완료 여부)
Flags(플래그들)
Counters(카운트들)
*/



USTRUCT(BlueprintType)
struct FQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Step = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> Flags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> Counters;
};
