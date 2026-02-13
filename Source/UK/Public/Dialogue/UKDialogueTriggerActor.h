#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UKDialogueTriggerActor.generated.h"

class UBoxComponent;
class AUKDialogueRunnerActor;

UCLASS()
class UK_API AUKDialogueTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	AUKDialogueTriggerActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerBox;

	// 이 트리거가 시작시킬 대화 ID
	UPROPERTY(EditAnywhere, Category = "DialogueTrigger")
	FString DialogueIdToStart = TEXT("D_TestQuest_Handsome_001_v1");

	// 맵에 배치된 러너를 직접 지정(안전하게/빠름추구)
	UPROPERTY(EditAnywhere, Category = "DialogueTrigger")
	AUKDialogueRunnerActor* RunnerRef = nullptr;

	// 1회만 발동
	UPROPERTY(EditAnywhere, Category = "DialogueTrigger")
	bool bTriggerOnce = true;

	bool bTriggered = false;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};