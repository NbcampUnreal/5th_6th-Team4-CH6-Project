// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Systems/Data/UK_InGameSave.h"
#include "Character/UK_CharacterBase.h"
#include "UI/OutGame/UK_Out_Loading.h"
#include "UK_GameInstance.generated.h"


UCLASS()
class UK_API UUK_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class AUK_CharacterBase> CharacterSelected;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	class UUK_Out_Loading* PersistentLoadingWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUK_Out_Loading> LoadingWidgetClass;

	UFUNCTION(BlueprintCallable)
	void LoadLevelWithLoading(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void OnLevelLoaded();

	UFUNCTION(BlueprintCallable)
	void SetLoadingInputMode(APlayerController* PC);
	
	UFUNCTION(BlueprintCallable)
	void SaveEntireGame();
	
	UFUNCTION(BlueprintCallable)
	void LoadEntireGame();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "SaveSystem")
	FString MainSaveSlotName = TEXT("Slot_0");
};
