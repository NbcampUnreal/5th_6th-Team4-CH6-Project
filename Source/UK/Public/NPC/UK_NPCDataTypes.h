#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UK_NPCDataTypes.generated.h"

UENUM(BlueprintType)
enum class ENPCState : uint8
{
	Idle,
	Walking,
	Working,
	Sleeping,
	Shelter
};

USTRUCT(BlueprintType)
struct FNPCScheduleRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NPC_ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartHour;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENPCState ActivityState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* ActionMontage;
};
