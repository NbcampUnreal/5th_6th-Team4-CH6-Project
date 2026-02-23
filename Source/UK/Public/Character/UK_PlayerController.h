// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	void ToggleMouseCursor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bMouseCursorEnabled = false;

private:

	void EnableMouseCursorMode();
	void DisableMouseCursorMode();
};
