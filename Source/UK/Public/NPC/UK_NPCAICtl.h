#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "UK_NPCAICtl.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;


UCLASS()
class UK_API AUK_NPCAICtl : public AAIController
{
	GENERATED_BODY()

public:
	AUK_NPCAICtl();
	void DrawSightDebug();
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBehaviorTreeComponent* BTComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAISenseConfig_Sight* SightConfig;
};
