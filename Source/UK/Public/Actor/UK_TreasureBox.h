#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "UK_TreasureBox.generated.h"

class UNigaraSystem;
class USoundBase;

UCLASS()
class UK_API AUK_TreasureBox : public AActor
{
	GENERATED_BODY()
	
public:	
	AUK_TreasureBox();

protected:

	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:	
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void TryOpen(AActor* InteractingPlayer);
	
	UPROPERTY(EditAnywhere, Category = "UK|Data")
	class UDataTable* RewardTable;
	
	UPROPERTY(VisibleAnywhere, Category = "UK|Collision")
	class USphereComponent* InteractionSphere;
	
	UPROPERTY(EditAnywhere, Category = "UK|Reward")
	int32 MinGold = 1000;
	
	UPROPERTY(EditAnywhere, Category = "UK|Reward")
	int32 MaxGold = 5000;
	
	UPROPERTY(EditAnywhere, Category = "UK|Reward")
	int32 MinItemCount = 3;
	
	UPROPERTY(EditAnywhere, Category = "UK|Reward")
	int32 MaxItemCount = 5;
	
	bool bIsOpened = false;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* BoxMesh;
	
	UPROPERTY(EditAnywhere, Category = "UK|Effects")
	class UNiagaraSystem* OpenVFX;
	
	UPROPERTY(EditAnywhere, Category = "UK|Effects")
	class USoundBase* OpenSound;
	
	UPROPERTY(EditAnywhere, Category = "UK|Effects")
	FVector VFXLocationOffset = FVector(0.0f, 0.0f, 50.0f);
};
