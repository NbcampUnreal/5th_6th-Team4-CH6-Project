#include "Server/UKGameInstance.h"
#include "Engine/Engine.h"

#include "GameFramework/PlayerController.h"
#include "Server/UKSaveGame.h"
#include "Kismet/GameplayStatics.h"


/* 목록
1. 현재 ue 5.7_steam기반 리슨서버는 c++ 네이처로 사용불가하므로, 플러그인 대체함.
	관련 5.7 fix가 들어오면 c++ 기반 세션구현으로 재작성할것임
2. savegame 관련추가 내용 (서브시스템 분리시 조정할것, 수정엄금
*/



// [1] 세션 구역 _ 현재는 플러그인+bp로 대체중임

/*최종 사용 규칙(중요)
재생성하고 싶을 때 : CreateSessionLAN() (이미 있으면 자동으로 Destroy→Create)
그냥 방을 닫고 끝낼 때 : CloseSessionLAN() (재생성 없이 Destroy만)*/




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

void UUKGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("[GI] Init - Steam BP mode active"));
}

void UUKGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("[GI] Shutdown"));
	Super::Shutdown();
}