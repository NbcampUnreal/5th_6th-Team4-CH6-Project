// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_GameOver.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class UK_API UUK_GameOver : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(Transient, BlueprintReadWrite, meta = ( BindWidgetAnim ))
	class UWidgetAnimation* FadeIn;

	UPROPERTY(meta = ( BindWidget ))
	UButton* RestartButton;

	UFUNCTION()
	void OnRestartButtonClicked();

	// 띄워줄 지도 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverMapClass;

public:
	// 위젯이 생성된 후 호출될 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupGameOverUI();
};
