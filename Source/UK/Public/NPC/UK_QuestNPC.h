#pragma once

#include "CoreMinimal.h"
#include "NPC/UK_NPCAIBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UI/InGame/UK_Quest.h"
#include "DataAsset/NPCData/UK_NPCData.h"
#include "Engine/DataTable.h"
#include "UK_QuestNPC.generated.h"

class AUK_CharacterBase;
class UUKQuestManagerSubsystem;
class UUKQuestDefinitionAsset;

UCLASS()
class UK_API AUK_QuestNPC : public AUK_NPCAIBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AUK_QuestNPC();

	bool CanInteract() const { return bPlayerInRange; }
	void Interact(AActor* Interactor);

	// 퀘스트 마커를 표시할 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	USphereComponent* InteractionSphere;

	// 퀘스트가 있는 NPC일 때 표시할 마커
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	UStaticMeshComponent* QuestMarker;

	FTimerHandle MarkerTimerHandle;

	// 가까이 있으면 상호작용 가능
	bool bPlayerInRange;

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnPlayerExit(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void UpdateMarkerRotation();

	// 하위 호환용 / 단일 퀘스트 fallback
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	// 중장기 구조: 이 NPC가 순서대로 담당하는 퀘스트 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FName> OfferedQuestIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TSubclassOf<UUK_Quest> QuestUIClass;

	void HandleQuestInteract(AUK_CharacterBase* Player);

	// DT에서 읽어온 공식 NPC EntityID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Data")
	FName NPCID;

	// DT에서 읽어온 표시용 이름
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Data")
	FText NPCDisplayName;

	// DT에서 읽어온 NPC 설명
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Data")
	FText NPCDescription;

protected:
	bool IsQuestStarted(UUKQuestManagerSubsystem* QuestSys, FName InQuestId) const;
	bool IsQuestCompleted(UUKQuestManagerSubsystem* QuestSys, FName InQuestId) const;

	FName ResolveQuestIdToShow(UUKQuestManagerSubsystem* QuestSys) const;
	FName ResolveDialogueIdForQuest(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const;

	FName MakeDialogueIdBySuffix(const UUKQuestDefinitionAsset* Def, const FString& Suffix) const;
	FText BuildInProgressDialogueText(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const;
	FText BuildInProgressQuestDesc(UUKQuestManagerSubsystem* QuestSys, const UUKQuestDefinitionAsset* Def, FName InQuestId) const;

private:
	bool TryProcessDelivery(UUKQuestManagerSubsystem* QuestSys, AUK_CharacterBase* Player, FName QuestId) const;
};