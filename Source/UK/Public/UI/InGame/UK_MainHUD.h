#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/StatusComponent.h"
#include "UK_MainHUD.generated.h"

UCLASS()
class UK_API UUK_MainHUD : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 초기화를 위해 NativeConstruct 오버라이드 
	virtual void NativeConstruct() override;

	// 체력이 변할 때 실행될 함수 
	UFUNCTION()
	void UpdateHealthBar(float CurrentHp, float MaxHp);

	// 위젯에서 만든 프로그레스 바나 이미지를 바인딩 
	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* HealthBar;
};
