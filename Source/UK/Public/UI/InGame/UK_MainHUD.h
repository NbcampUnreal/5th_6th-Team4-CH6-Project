#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/StatusComponent.h"
#include "Containers/Map.h"
#include "UK_MainHUD.generated.h"

class UTextBlock;
class UButton;
class UUK_InvMain;

class UUK_ItemNotify;
class UVerticalBox;
class UDataTable;
class UUK_InventoryComponent;

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

	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* HealthBar;

	// 체력 수치를 표시할 텍스트 
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* CurrentHealthText; // 현재 체력

	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* MaxHealthText;     // 최대 체력

	// 마나가 변할 때 실행될 함수
	UFUNCTION()
	void UpdateMpBar(float CurrentMp, float MaxMp);

	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* MpBar;

	// 마나 수치 텍스트
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* CurrentMpText; // 현재 마나

	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* MaxMpText; // 최대 마나

	// 레벨이 변할 때 실행될 함수
	UFUNCTION()
	void UpdateLevel(int32 NewLevel);

	// 레벨 표시용 텍스트 
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* LevelText;
	// 인벤토리 버튼
	UFUNCTION()
	void OnInventoryButtonClicked();

	UPROPERTY(meta = ( BindWidget ))
	UButton* InventoryButton; 

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_InvMain> InvMainClass; // 에디터에서 인벤토리 블루프린트 할당

	UPROPERTY()
	UUK_InvMain* InvMainWidget; // 생성된 위젯 참조 저장용
	//Notify
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_ItemNotify = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemNotify")
	TSubclassOf<UUK_ItemNotify> ItemNotifyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemNotify")
	UDataTable* ItemDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemNotify", meta = ( ClampMin = "1" ))
	int32 MaxNotifyCount = 3;

	UFUNCTION(BlueprintCallable)
	void ShowItemNotify(FName ItemID, int32 Amount);

	void NotifyChildren();

	UPROPERTY()
	UUK_InventoryComponent* InvComp = nullptr;


	UPROPERTY()
	TMap<FName, TObjectPtr<UUK_ItemNotify>> ActiveNotifyMap;
};
