#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/UKQuestConditionTypes.h"
#include "UKQuestConditionAsset.generated.h"

/*
	[Condition Asset]
	- ConditionID 1개 = Clause 묶음 1개
	- 명명규칙:
		Cond_Common_<Name>
		Cond_<QuestID>_<Name>
*/
UCLASS(BlueprintType)
class UK_API UUKQuestConditionAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition|ID")
	FName ConditionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition|Logic")
	EUKConditionGroupOp GroupOp = EUKConditionGroupOp::AllOf;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition|Logic")
	TArray<FUKQuestConditionClause> Clauses;
};