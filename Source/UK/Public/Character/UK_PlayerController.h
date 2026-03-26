// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h" 
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerController.h"
#include "UI/InGame/UK_Stamina.h"
#include "UI/InGame/UK_Setting.h"
#include "UI/InGame/UK_Quest.h"
#include "UK_PlayerController.generated.h"


#pragma region ForwardDeclaration
class UInputMappingContext;
class UInputAction;
class UUK_MainHUD;
class UUK_GameOver;
class UAbilitySystemComponent;
class UUK_InvMain;
class UUserWidget;
class UUK_Crafting;
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

	UFUNCTION(BlueprintCallable)
	void ClearAllWidgets();

	//UFUNCTION()
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	void CloseAllExceptSetting();

private:
	UFUNCTION(Client, Reliable)
	void Client_CreatePlayerUI();

public:

	UUK_MainHUD* GetMainHUD() const { return MainHUD; }
	// ----- Input -----

	bool InputKey_Check(const FInputKeyParams& Params);

	UFUNCTION(BlueprintCallable)
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

	void CloseSettingUI();

	void SetAllGameUIInputVisibility(bool bVisible);

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

	UPROPERTY(BlueprintReadWrite, Category = "HUD")
	TObjectPtr<UUK_MainHUD> MainHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUK_MainHUD> MainHUDClass;

public:

	UPROPERTY(EditAnywhere)
	UInputMappingContext* IMC;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_Stamina> StaminaWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUK_Stamina* StaminaWidget;

	void ConnectStaminaWidget();

public:
	// ----- GameOver ----- 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUK_GameOver> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UUK_GameOver* GameOverWidget;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

	UFUNCTION()
	void ShowGameOverUI();

	UFUNCTION()
	void HideGameOverUI();

	//UFUNCTION()
	
	//  ------ Interaction ------ (무현 수정중)

protected:

	UPROPERTY(BlueprintReadWrite, Category = "UI|Interaction")
	TSubclassOf<UUK_Quest> QuestWidgetClass;

	UPROPERTY()
	UUK_Quest* QuestWidget;
 
	UPROPERTY()
	class UUserWidget* ShopWidget;
	
	
public:
	void ShowQuestUI(const FName& QuestID, const FText& NPCName, const FText& Dialogue, const FText& QuestDesc);
	void HideQuestUI();
	
	void ShowShopUI(TSubclassOf<UUserWidget>ShopWidgetClass);
	void HideShopUI();

public:
	UFUNCTION()
	bool CloseOpenWidget();

	UFUNCTION(BlueprintCallable)
	void OpenSettingAndCloseOtherUI();

	// ----- Inventory ----- //추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUK_InvMain> InventoryWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUK_InvMain* InventoryWidget;

	UFUNCTION(BlueprintCallable)
	void Inventory_UI();

	UFUNCTION(BlueprintCallable)
	void CloseInventoryUI();

	// ----- Weapon Crafting ----- //추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUK_Crafting> WeaponCraftingWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUK_Crafting* WeaponCraftingWidget;

	UFUNCTION(BlueprintCallable)
	void WeaponCrafting_UI();

	UFUNCTION(BlueprintCallable)
	void CloseWeaponCraftingUI();
};
