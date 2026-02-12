#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "UK_QuestGuideComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UUK_QuestGuideComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUK_QuestGuideComponent();

protected:
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, Category = "QuestGuide")
	void SetTarget(AActor* NewTarget);

	// 길 갱신
	void UpdatePath();

	UFUNCTION(BlueprintCallable, Category = "QuestGuide")
	void ClearGuide();

private:
	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	USplineComponent* GuideSpline;

	UPROPERTY()
	TArray<USplineMeshComponent*> MeshSegments;

	UPROPERTY(EditDefaultsOnly, Category = "QuestGuide")
	UStaticMesh* GuideMesh;

	void BuildSplineMesh();
	void ClearSpline();

	FTimerHandle GuideTimer;

	FVector ProjectToGround(const FVector& Point);
	void DrawPath(class UNavigationPath* Path);
};
