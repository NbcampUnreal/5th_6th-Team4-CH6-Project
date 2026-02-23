#pragma once

#include "CoreMinimal.h"
#include "NPC/UK_NPCAIBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UK_QuestNPC.generated.h"

class AUK_CharacterBase;

UCLASS()
class UK_API AUK_QuestNPC : public AUK_NPCAIBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AUK_QuestNPC();

	bool CanInteract() const {return bPlayerInRange;}

	virtual void Interact_Implementation(AActor* Interactor) override;

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
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateMarkerRotation();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Quest")
	int32 QuestID;

	UFUNCTION(Server, Reliable)
	void Server_Interact(AUK_CharacterBase* Player);
};
