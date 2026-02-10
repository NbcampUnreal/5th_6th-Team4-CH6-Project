#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "UK_BTService_NPCDetectPlayer.generated.h"


UCLASS()
class UK_API UUK_BTService_NPCDetectPlayer : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UUK_BTService_NPCDetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Detect")
	float DetectRadius = 100.f;

	bool IsAvoiding;
};