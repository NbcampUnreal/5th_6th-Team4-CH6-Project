// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/InGame/UK_Stamina.h"
#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/UK_Quest.h"
#include "UK_PlayerController.generated.h"


#pragma region ForwardDeclaration
class UInputMappingContext;
class UInputAction;
#pragma endregion

UENUM(BlueprintType)
enum class EInputState : uint8
{
	Game,
	UI,
	Cutscene
};

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

private:
	UFUNCTION(Client, Reliable)
	void Client_CreatePlayerUI();

public:
	// ----- Input -----

	void ApplyInputState(EInputState NewState);

private:
	EInputState CurrentInputState = EInputState::Game;

	// ----- Cursor ----- 

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bMouseCursorEnabled = false;

	void ToggleMouseCursor();
	void SetCursorVisible(bool bVisible);

	// ----- Setting -----

	UPROPERTY()
	UUK_Setting* SettingWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_Setting> SettingWidgetClass;

	bool bIsSetting = false;
	void Setting_UI();

	// ----- Stamina ----- 

public:
	FTimerHandle StaminaTrackingTimer;
	void UpdateStaminaTracking();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	FVector Stemina_Location = FVector(0.f, 0.f, 50.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float SideDistance = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float DistanceMin = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float DistanceMax = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float ScaleNear = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrackingUI")
	float ScaleFar = 0.5f;


private:

	UPROPERTY(EditAnywhere)
	UInputMappingContext* IMC;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_Stamina> StaminaWidgetClass;

	UPROPERTY()
	UUK_Stamina* StaminaWidget;

	void ConnectStaminaWidget();

	//  ------ Interaction ------ (무현 수정중)

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<UUK_Quest> QuestWidgetClass;

	UPROPERTY()
	UUK_Quest* QuestWidget;

public:

	UFUNCTION(Client, Reliable)
	void Client_ShowQuestUI(const FName& QuestID,const FText& NPCName,const FText& Dialogue,const FText& QuestDesc);

	UFUNCTION(Client, Reliable)
	void Client_HideQuestUI();
};
