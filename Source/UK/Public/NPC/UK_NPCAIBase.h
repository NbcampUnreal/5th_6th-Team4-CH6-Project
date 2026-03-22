#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UK_NPCAIBase.generated.h"

UCLASS()
class UK_API AUK_NPCAIBase : public ACharacter
{
	GENERATED_BODY()

public:
	AUK_NPCAIBase();

	UFUNCTION()
	virtual void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UDataTable* ScheduleTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FName MyNPC_ID;
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
};
