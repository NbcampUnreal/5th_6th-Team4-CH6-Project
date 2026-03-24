#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_CategoryTap.generated.h"

class UButton;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCategoryTapClicked, UUK_CategoryTap*, OnCategoryTap
);


UCLASS()
class UK_API UUK_CategoryTap : public UUserWidget
{
	GENERATED_BODY()
	

	
public:

	UPROPERTY(meta = (BindWidget))
	UButton* CategoryButton;

	//메인 UI의 디테일 패널에서 수정 가능하도록 노출
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvCategoryTap")
	FButtonStyle CustomButtonStyle;

	//델리게이트 속성, 이벤트 호출 시 실행될 동작
	UPROPERTY(BlueprintAssignable, Category = "InvCategoryTap")
	FOnCategoryTapClicked OnCategoryTap;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void CategoryTapClicked();

	UPROPERTY(BlueprintReadOnly)
	bool bSelected = false;

	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bInSelected);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* ClickSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* HoverSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* UnHoverSound;
	
protected:
	UFUNCTION()
	void OnHovered();
	
	UFUNCTION()
	void OnUnHovered();
};
