#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Map.h"
#include "AbilitySystemComponent.h"
#include "UK_MainHUD.generated.h"

class UTextBlock;
class UButton;
class UUK_InvMain;

class UUK_ItemNotify;
class UVerticalBox;
class UDataTable;
class UUK_InventoryComponent;
class AUK_CharacterBase;
//struct FOnAttributeChangeData;

UCLASS()
class UK_API UUK_MainHUD : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 초기화를 위해 NativeConstruct 오버라이드 
	virtual void NativeConstruct() override;
public:
	// 체력이 변할 때 실행될 함수 
	//UFUNCTION()
	void UpdateHealthBar(const FOnAttributeChangeData& Data);
	
	// 마나가 변할 때 실행될 함수
	//UFUNCTION()
	void UpdateMpBar(const FOnAttributeChangeData& Data);

	// 레벨이 변할 때 실행될 함수
	//UFUNCTION()
	void UpdateLevel(const FOnAttributeChangeData& Data);
	
	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* HealthBar;

	// 체력 수치를 표시할 텍스트 
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* CurrentHealthText; // 현재 체력

	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* MaxHealthText;     // 최대 체력


	UPROPERTY(meta = ( BindWidget ))
	class UProgressBar* MpBar;

	// 마나 수치 텍스트
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* CurrentMpText; // 현재 마나

	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* MaxMpText; // 최대 마나


	// 레벨 표시용 텍스트 
	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* LevelText;
	
	// 인벤토리 버튼
	UFUNCTION()
	void OnInventoryButtonClicked();

	UPROPERTY(meta = ( BindWidget ))
	UButton* InventoryButton; 

	//NewText
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* NewText;


	UFUNCTION(BlueprintCallable)
	void OpenInventory();

	UFUNCTION(BlueprintCallable)
	void CloseInventory();

	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

	UFUNCTION(BlueprintCallable)
	void HandleItemAdded_ShowNew(FName ItemID, int32 Amount);

	void SetInventoryNewVisible(bool bVisible);
	
	void RefreshAllStatus();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUK_InvMain> InvMainClass; // 에디터에서 인벤토리 블루프린트 할당

	UPROPERTY()
	UUK_InvMain* InvMainWidget; // 생성된 위젯 참조 저장용
	//Notify
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VB_ItemNotify = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemNotify")
	TSubclassOf<UUK_ItemNotify> ItemNotifyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UDataTable>> ItemDataTables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemNotify", meta = ( ClampMin = "1" ))
	int32 MaxNotifyCount = 3;

	UFUNCTION(BlueprintCallable)
	void ShowItemNotify(FName ItemID, int32 Amount);

	void NotifyChildren();

	UPROPERTY()
	UUK_InventoryComponent* InvComp = nullptr;


	UPROPERTY()
	TMap<FName, TObjectPtr<UUK_ItemNotify>> ActiveNotifyMap;
	
	UPROPERTY()
	AUK_CharacterBase* PlayerPawn;
	
	UPROPERTY()
	UAbilitySystemComponent* ASC;
};
