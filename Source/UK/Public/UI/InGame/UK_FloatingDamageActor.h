#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_FloatingDamageActor.generated.h"

class USceneComponent;
class UWidgetComponent;
class UCurveFloat;
class UUK_FloatingDamage;

UCLASS()
class UK_API AUK_FloatingDamageActor : public AActor
{
	GENERATED_BODY()

public:
	AUK_FloatingDamageActor();

	virtual void BeginPlay() override;

	void SetDamageAmount(float InDamage);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	TObjectPtr<UCurveFloat> FloatCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float ActorDamageAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MoveDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float TimerInterval = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MinOffsetX = -30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MaxOffsetX = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MinOffsetY = -30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MaxOffsetY = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MinOffsetZ = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float MaxOffsetZ = 150.f;

private:
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<UUK_FloatingDamage> FloatingDamageWidget;

	float ElapsedTime = 0.f;

	FTimerHandle MoveTimerHandle;

	void InitWidget();
	void InitMoveData();
	void StartMoveTimer();
	void UpdateMove();
	void FinishMove();
};