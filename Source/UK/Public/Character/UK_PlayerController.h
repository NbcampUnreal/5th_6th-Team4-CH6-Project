// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/InGame/UK_Stamina.h"
#include "UK_PlayerController.generated.h"


#pragma region ForwardDeclaration
class UInputMappingContext;
class UInputAction;
#pragma endregion

UCLASS()
class UK_API AUK_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AUK_PlayerController();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;
	virtual void OnPossess(APawn* pawn) override;
	virtual void Tick(float DeltaSeconds) override;
	void ToggleMouseCursor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bMouseCursorEnabled = false;

	// UI 위치 조정

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	FVector Stemina_Location = FVector(0.f, 0.f, 110.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float SideDistance = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float DistanceMin = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float DistanceMax = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float ScaleNear = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float ScaleFar = 0.55f;

private:

	void EnableMouseCursorMode();
	void DisableMouseCursorMode();

private:

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_Stamina> StaminaWidgetClass;

	UPROPERTY()
	UUK_Stamina* StaminaWidget;

	void ConnectStaminaWidget();
};
