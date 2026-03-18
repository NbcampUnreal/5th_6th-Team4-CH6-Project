#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_FloatingDamage.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class UK_API UUK_FloatingDamage : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// 액터 쪽에서 데미지 값 넣어줄 함수
	UFUNCTION(BlueprintCallable)
	void SetDamageAmount(float InDamage);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FloatingDamage_Text;

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	TObjectPtr<UWidgetAnimation> Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingDamage")
	float DamageAmount = 0.f;

	void RefreshDamageText();
};
