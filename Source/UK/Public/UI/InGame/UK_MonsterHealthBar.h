#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIMonster/Component/AI_MonsterStatComponent.h"
#include "UK_MonsterHealthBar.generated.h"

UCLASS()
class UK_API UUK_MonsterHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 초기화를 위해 NativeConstruct 오버라이드 
	virtual void NativeConstruct() override;
	
	// 델리게이트를 통해 호출될 함수
	UFUNCTION()
	void UpdateHPBar(float CurrentHP, float MaxHP);

	// 에디터 위젯과 연결
	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* MonsterHPBar;

public:
	// 몬스터의 스탯 컴포넌트와 UI를 연결해주는 핵심 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BindMonsterStats(class UAI_MonsterStatComponent* StatComp);
};
