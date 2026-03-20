#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_FindUKPlayer.generated.h"

UCLASS()
class UK_API UUK_BTService_FindUKPlayer : public UBTService
{
	GENERATED_BODY()
public:
	UUK_BTService_FindUKPlayer();

protected:

	UPROPERTY(EditAnywhere, Category="Detect")
	float DetectRadius = 5000.f;
	
	UPROPERTY(EditAnywhere, Category="Detect")
	float SearchDuration = 30.f;
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds) override;
};
