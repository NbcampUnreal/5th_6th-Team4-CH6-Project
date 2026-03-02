#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "UKGameInstance.generated.h"


/* 목록
1. 현재 ue 5.7_steam기반 리슨서버는 c++ 네이처로 사용불가하므로, 플러그인 대체함.
	관련 5.7 fix가 들어오면 c++ 기반 세션구현으로 재작성할것임
2. savegame 관련추가 내용 (서브시스템 분리시 조정할것, 수정엄금*/



UCLASS()
class UK_API UUKGameInstance : public UGameInstance
{
	GENERATED_BODY()

// [1] 세션 구역 _ 현재는 플러그인+bp로 대체중임

// [2] savegame 관련

public:
	virtual void Init() override;
	virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    bool SaveToSlotSimple(const FString& SlotName = TEXT("UK_SaveSlot"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable)
    bool LoadFromSlotSimple(const FString& SlotName = TEXT("UK_SaveSlot"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable)
    bool ApplyLoadedTransformToHost();

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<class UUKSaveGame> LoadedSave;
};