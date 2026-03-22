#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UK_TimeSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChanged, int32, NewHour);

UCLASS()
class UK_API UUK_TimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UUK_TimeSubsystem, STATGROUP_Tickables); }
	
	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnHourChanged OnHourChanged;
	
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 CurrentHour = 9;
	
private:
	float TimeAccumulator = 0.f;
	const float SecondsPerGameHour = 60.f;
	
	
};
