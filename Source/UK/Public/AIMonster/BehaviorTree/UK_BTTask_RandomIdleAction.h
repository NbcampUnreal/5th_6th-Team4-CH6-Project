#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_RandomIdleAction.generated.h"

UENUM(BlueprintType)
enum class EIdleActionType : uint8
{
	Wait,
	LookAround,
	PlayIdleMontage
};

UCLASS()
class UK_API UUK_BTTask_RandomIdleAction : public UBTTaskNode
{
	GENERATED_BODY()

#pragma region Initialization
public:
	UUK_BTTask_RandomIdleAction();
	virtual uint16 GetInstanceMemorySize() const override;
#pragma endregion

#pragma region Execution
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Idle Tick
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion

#pragma region Abort
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion

#pragma region Idle Settings
protected:
	UPROPERTY(EditAnywhere, Category = "Idle")
	float MinWaitTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Idle")
	float MaxWaitTime = 7.0f;

	UPROPERTY(EditAnywhere, Category = "Idle", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float LookAroundSpeed = 2.0f;

	/** 0=Wait, 1=LookAround, 2=PlayIdleMontage */
	UPROPERTY(EditAnywhere, Category = "Idle")
	TArray<float> ActionWeights = { 0.3f, 0.4f, 0.3f };
#pragma endregion

#pragma region Memory
private:
	struct FIdleActionMemory
	{
		float           ElapsedTime        = 0.f;
		float           TargetTime         = 0.f;
		EIdleActionType Action             = EIdleActionType::Wait;
		float           StartYaw           = 0.f;
		float           TargetYaw          = 0.f;
		bool            bWaitingForMontage = false;
		bool            bMontageEnded      = false;
		bool            bPostMontageWait   = false;
		float           PostMontageEndTime = 0.f;
		float			CachedWalkSpeed = 0.f;
		float			MidYaw      = 0.f;  
		bool			bReachedMid = false; 
	};
#pragma endregion

#pragma region Helpers
	EIdleActionType SelectWeightedAction() const;
	void RestoreRotationSettings(ACharacter* Character) const;
#pragma endregion
};