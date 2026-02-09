#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "UKGameInstance.generated.h"


/* 목록
1. Lan 기반 세션 구현 및 가능한 오류방지 시스템
2. savegame 관련추가 내용 (서브시스템 분리시 조정할것, 수정엄금*/

// [1] Lan 기반 세션

UCLASS()
class UK_API UUKGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    void CreateSessionLAN(int32 PublicConnections = 4);

    UFUNCTION(BlueprintCallable)
    void FindSessionsLAN(int32 MaxResults = 50);

    UFUNCTION(BlueprintCallable)
    void JoinFoundSession(int32 Index = 0);

    UFUNCTION(BlueprintCallable)
    void DestroySessionLAN();

    UFUNCTION(BlueprintCallable)
    void CloseSessionLAN(); // 유저가 '방 닫기' 누를 때, 재생성 없이 종료만

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    bool bDestroyInProgress = false; // Destroy 중 중복 호출 방지

    bool bCreateAfterDestroy = false;  // 유저가 직접 닫을 땐 보통 false.
    int32 PendingPublicConnections = 4; // 공개슬롯 수. 즉 임시값은 4명방 재생성

    FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
    FDelegateHandle OnCreateSessionCompleteHandle;

    FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
    FDelegateHandle OnFindSessionsCompleteHandle;

    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
    FDelegateHandle OnJoinSessionCompleteHandle;

    FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
    FDelegateHandle OnDestroySessionCompleteHandle;

private:
    void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void HandleFindSessionsComplete(bool bWasSuccessful);
    void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

// [2] savegame 관련

public:
    UFUNCTION(BlueprintCallable)
    bool SaveToSlotSimple(const FString& SlotName = TEXT("UK_SaveSlot"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable)
    bool LoadFromSlotSimple(const FString& SlotName = TEXT("UK_SaveSlot"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable)
    bool ApplyLoadedTransformToHost();

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<class UUKSaveGame> LoadedSave;
};