#include "Server/UKGameInstance.h"
#include "Engine/Engine.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/PlayerController.h"
#include "Server/UKSaveGame.h"
#include "Kismet/GameplayStatics.h"

static const FName SESSION_NAME = NAME_GameSession;

/* 목록
1. Lan 기반 세션 구현 및 가능한 오류방지 시스템 
2. savegame 관련추가 내용 (서브시스템 분리시 조정할것, 수정엄금*/



// [1] Lan 기반 세션

/*최종 사용 규칙(중요)
재생성하고 싶을 때 : CreateSessionLAN() (이미 있으면 자동으로 Destroy→Create)
그냥 방을 닫고 끝낼 때 : CloseSessionLAN() (재생성 없이 Destroy만)*/


void UUKGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("[GI] Init"));
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
    {
        SessionInterface = OSS->GetSessionInterface();
    }

    // Delegate 바인딩 준비(핸들은 함수 호출 시 AddOn...로 잡는 편이 안전)
    OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UUKGameInstance::HandleCreateSessionComplete);
    OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UUKGameInstance::HandleFindSessionsComplete);
    OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UUKGameInstance::HandleJoinSessionComplete);
    OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UUKGameInstance::HandleDestroySessionComplete);
}

void UUKGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("[GI] Shutdown"));
	Super::Shutdown();
}

void UUKGameInstance::CreateSessionLAN(int32 PublicConnections)
{
    if (!SessionInterface.IsValid()) return;

    // Destroy 진행 중에는 새 Create 요청을 “예약값만 갱신”하고 종료
    if (bDestroyInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Session] Destroy in progress. Update pending create settings only."));
        bCreateAfterDestroy = true;
        PendingPublicConnections = PublicConnections;
        return;
    }

    // 이미 세션 있으면 정리하고 다시 만들기
    if (SessionInterface->GetNamedSession(SESSION_NAME))
    {
        bCreateAfterDestroy = true;
        PendingPublicConnections = PublicConnections;
        DestroySessionLAN();
        return;
    }

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = true;
    Settings.NumPublicConnections = PublicConnections;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowJoinViaPresence = false;   // Null에선 의미 적음
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = false;
    Settings.bUseLobbiesIfAvailable = false;

    // 검색 키워드용(선택)
    Settings.Set(FName("SERVER_NAME"), FString("LAN_TEST"), EOnlineDataAdvertisementType::ViaOnlineService);

    OnCreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

    const ULocalPlayer* LP = GetFirstGamePlayer();
    const int32 UserNum = LP ? LP->GetControllerId() : 0;

    SessionInterface->CreateSession(UserNum, SESSION_NAME, Settings);
}

void UUKGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("[Session] Create complete: %s, Success=%d"), *SessionName.ToString(), bWasSuccessful);

}

void UUKGameInstance::FindSessionsLAN(int32 MaxResults)
{
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShared<FOnlineSessionSearch>();
    SessionSearch->MaxSearchResults = MaxResults;
    SessionSearch->bIsLanQuery = true;

    OnFindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

    const ULocalPlayer* LP = GetFirstGamePlayer();
    const int32 UserNum = LP ? LP->GetControllerId() : 0;

    SessionInterface->FindSessions(UserNum, SessionSearch.ToSharedRef());
}

void UUKGameInstance::HandleFindSessionsComplete(bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("[Session] Find complete: Success=%d"), bWasSuccessful);

    if (!bWasSuccessful || !SessionSearch.IsValid()) return;

    UE_LOG(LogTemp, Log, TEXT("[Session] Results: %d"), SessionSearch->SearchResults.Num());
    for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
    {
        const auto& R = SessionSearch->SearchResults[i];
        FString ServerName;
        R.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

        UE_LOG(LogTemp, Log, TEXT("  [%d] Ping=%d, Name=%s"), i, R.PingInMs, *ServerName);
    }
}

void UUKGameInstance::JoinFoundSession(int32 Index)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;
    if (!SessionSearch->SearchResults.IsValidIndex(Index)) return;

    OnJoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

    const ULocalPlayer* LP = GetFirstGamePlayer();
    const int32 UserNum = LP ? LP->GetControllerId() : 0;

    SessionInterface->JoinSession(UserNum, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UUKGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("[Session] Join complete: %s, Result=%d"), *SessionName.ToString(), (int32)Result);

    // 1. 성공이 아니면 이유 로그 찍고 종료
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Session] Join failed. Result=%d"), (int32)Result);
        return;
    }

    // 2. ConnectString 못 얻으면 로그
    FString ConnectString;
    if (!SessionInterface.IsValid() || !SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Session] Failed to resolve connect string for session %s"), *SessionName.ToString());
        return;
    }

    // 3. PC 없으면 로그
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Session] PlayerController is null (cannot ClientTravel)."));
        return;
    }

    // 4. Join 후 실제 접속 이동(필수)
    UE_LOG(LogTemp, Log, TEXT("[Session] ClientTravel to: %s"), *ConnectString);
    PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
}

void UUKGameInstance::DestroySessionLAN()
{
    if (!SessionInterface.IsValid()) return;

    // Destroy가 이미 진행 중이면 중복 호출 방지
    if (bDestroyInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Session] Destroy already in progress. Ignored."));
        return;
    }

    // 실제로 세션이 있을 때만 Destroy 시도(불필요 호출 방지)
    if (!SessionInterface->GetNamedSession(SESSION_NAME))
    {
        UE_LOG(LogTemp, Log, TEXT("[Session] No session to destroy."));
        return;
    }

    bDestroyInProgress = true;

    OnDestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
    SessionInterface->DestroySession(SESSION_NAME);
}

void UUKGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteHandle);
    }

    bDestroyInProgress = false; // Destroy 종료(성공/실패 상관없이)

    UE_LOG(LogTemp, Log, TEXT("[Session] Destroy complete: %s, Success=%d"), *SessionName.ToString(), bWasSuccessful);

    // Destroy가 성공했고, “재생성 예약” 상태라면 여기서 Create 실행
    if (bCreateAfterDestroy && bWasSuccessful)
    {
        bCreateAfterDestroy = false;
        CreateSessionLAN(PendingPublicConnections);
    }
    else
    {
        // 실패했는데 재생성 예약이 걸려있었다면 안전하게 꺼줌
        if (bCreateAfterDestroy && !bWasSuccessful)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Session] Destroy failed, cancel recreate."));
            bCreateAfterDestroy = false;
        }
    }
}

void UUKGameInstance::CloseSessionLAN()
{
    // 유저가 명시적으로 “닫기” 누른 경우 → 재생성 예약은 끈다
    bCreateAfterDestroy = false;
    DestroySessionLAN();

    // 방 닫기 버튼은 DestroySessionLAN() 말고 CloseSessionLAN()을 호출해야함
}



// [2] savegame 관련

bool UUKGameInstance::SaveToSlotSimple(const FString& SlotName, int32 UserIndex)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    UUKSaveGame* SaveObj = Cast<UUKSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UUKSaveGame::StaticClass())
    );
    if (!SaveObj) return false;

    SaveObj->SavedMapName = FName(*World->GetMapName());

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
    {
        if (APawn* P = PC->GetPawn())
        {
            SaveObj->HostPlayer.Transform = P->GetActorTransform();
        }
    }

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool UUKGameInstance::LoadFromSlotSimple(const FString& SlotName, int32 UserIndex)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
        return false;

    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
    LoadedSave = Cast<UUKSaveGame>(Loaded);
    return (LoadedSave != nullptr);
}

bool UUKGameInstance::ApplyLoadedTransformToHost()
{
    if (!LoadedSave) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
    {
        if (APawn* P = PC->GetPawn())
        {
            P->SetActorTransform(LoadedSave->HostPlayer.Transform);
            return true;
        }
    }
    return false;
}