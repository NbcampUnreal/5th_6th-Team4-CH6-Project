#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UK_QuestComponent.generated.h"

USTRUCT(BlueprintType)
struct FQuestData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 QuestID;

	UPROPERTY()
	bool bCompleted;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UUK_QuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUK_QuestComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	TArray<FQuestData> QuestList;

public:	

	void ProcessQuest(int32 QuestID, class AUK_QuestNPC* NPC);

	UFUNCTION(Server, Reliable)
	void Server_ProcessQuest(int32 QuestID);

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
};
