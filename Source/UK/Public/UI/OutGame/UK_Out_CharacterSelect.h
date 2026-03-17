// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_Out_CharacterSelect.generated.h"

class UButton;
class UEditableText;
class AUK_CharacterBase;
/**
 * 
 */
UCLASS()
class UK_API UUK_Out_CharacterSelect : public UUserWidget
{
	GENERATED_BODY()
public:
	UUK_Out_CharacterSelect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	bool bAlreadySelected = false;
protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnStartButtonClicked();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	/*UFUNCTION()
	void SaveCharacterChoice(TSubclassOf<AUK_CharacterBase> ChosenClass);*/

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = ( AllowPrivateAccess, BindWidget ))
	TObjectPtr<UButton> StartButton;
	
};
