#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UK_BTService_DetectPlayer.generated.h"

UCLASS()
class UK_API UUK_BTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTService_DetectPlayer();
#pragma endregion

#pragma region Player Detection
protected:
	virtual void   TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion

#pragma region Blackboard Keys
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PendingTargetKey;
#pragma endregion

#pragma region Settings
	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float ReturnDistanceThreshold = 200.0f;
#pragma endregion

#pragma region Memory
private:
	struct FDetectPlayerMemory
	{
		bool bHadTarget = false; 
		bool bReturning = false;  
	};
#pragma endregion
};