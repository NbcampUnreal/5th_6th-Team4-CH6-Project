#pragma once

#include "CoreMinimal.h"
#include "NPC/UK_NPCAIBase.h"
#include "UK_PatrolNPC.generated.h"

class ATargetPoint;

UENUM(BlueprintType)
enum class EPatrolType : uint8
{
	RandomArea,
	FixedRoute
};

UCLASS()
class UK_API AUK_PatrolNPC : public AUK_NPCAIBase
{
	GENERATED_BODY()
	
public:
	AUK_PatrolNPC();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	EPatrolType PatrolType;

	/* 랜덤 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Random")
	float PatrolRadius = 800.f;

	/* 루트 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Route")
	TArray<AActor*> PatrolPoints;

	/* 현재 인덱스 */
	int32 CurrentIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsWaiting;

public:

	FVector GetNextPatrolPoint();

	bool bUseControllerRotationYaw = true;
};
