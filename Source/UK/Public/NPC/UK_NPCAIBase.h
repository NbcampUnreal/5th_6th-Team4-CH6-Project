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

	UFUNCTION(BlueprintNativeEvent)
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor);


	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
};
