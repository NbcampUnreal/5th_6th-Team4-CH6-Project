#pragma once

#include "CoreMinimal.h"

#include "UK_CheckPoint.generated.h"

UCLASS()
class UK_API AUK_CheckPoint : public AActor
{
	GENERATED_BODY()

public:
	AUK_CheckPoint();
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Sphere;
	
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* WidgetComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SetLocation;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUK_MarkerWidget> MarkerWidgetClass;

	UPROPERTY()
	UUK_MarkerWidget* MarkerWidget;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
};
