#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UK_InteractionComponent.generated.h"

class AActor;
class AUK_QuestNPC;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UUK_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUK_InteractionComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	AActor* NearActor;

public:	
	void SetNearActor(AActor* NewActor);
	void ClearNearActor();
	void TryInteract();

	UFUNCTION(Server, Reliable)
	void Server_TryInteract(AActor* Target);
		
};
