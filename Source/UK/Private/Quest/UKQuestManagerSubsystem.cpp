#include "Quest/UKQuestManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "server/UKSaveGame.h"

bool UUKQuestManagerSubsystem::StartQuest(FName QuestId)
{
	if ( QuestId.IsNone() ) return false;
	if ( RuntimeProgress.Contains(QuestId) ) return true;

	RuntimeProgress.Add(QuestId, FQuestProgress{});
	return true;
}

// 런타임에서 QuestId에 해당하는 FQuestProgress를 찾음
// 찾으면 그 퀘스트의 Step 값을 NewStep으로 갱신
// step은 “현재 퀘스트가 몇 번째 단계인지”를 나타내는 정수.
// ex) 0=미시작, 1=NPC 대화 완료, 2=아이템 획득, 3=보고 완료
// 규칙은 별도로 확인해야함
bool UUKQuestManagerSubsystem::SetQuestStep(FName QuestId, int32 NewStep)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->Step = NewStep;
	P->LastUpdatedAt = FDateTime::UtcNow();
	return true;
}

// 런타임에서 QuestId에 해당하는 FQuestProgress를 찾음
// 찾았으면 그 퀘스트의 bCompleted를 true로 설정해서 완료 처리
// 이 퀘스트는 끝났다라는 완료 플래그를 남기는 함수
bool UUKQuestManagerSubsystem::CompleteQuest(FName QuestId)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->bCompleted = true;
	P->LastUpdatedAt = FDateTime::UtcNow();
	return true;
}

void UUKQuestManagerSubsystem::EmitQuestEvent(FName EventId)
{
	// TODO: 나중에 DA/JSON과 연결해서 이벤트->진행도 갱신 로직을 여기서 수행
	// 지금은 뼈대만.
}

bool UUKQuestManagerSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UUKSaveGame* SaveObj = nullptr;

	// 1) 기존 슬롯이 있으면 먼저 로드해서 "기존 필드 보존"
	if ( UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) )
	{
		SaveObj = Cast<UUKSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}

	// 2) 없거나 로드 실패면 새로 생성
	if ( !SaveObj )
	{
		SaveObj = Cast<UUKSaveGame>(UGameplayStatics::CreateSaveGameObject(UUKSaveGame::StaticClass()));
	}

	if ( !SaveObj ) return false;

	// 3) 내가 책임지는 필드만 갱신 (나머지는 자동 보존)
	SaveObj->QuestProgressMap = RuntimeProgress;

	// (선택) SaveGame 메타 갱신: 저장 시각/버전
	SaveObj->LastSavedAt = FDateTime::UtcNow();
	SaveObj->SaveVersion = UUKSaveGame::CURRENT_SAVE_VERSION;

	// 4) 저장
	return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool UUKQuestManagerSubsystem::LoadFromSlot(const FString& SlotName, int32 UserIndex)
{
	if ( !UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) )
	{
		RuntimeProgress.Empty();
		return false;
	}

	UUKSaveGame* SaveObj = Cast<UUKSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if ( !SaveObj ) return false;

	// 버전 체크 자리(지금은 경고/로그만, 마이그레이션은 나중)
	// 현재 미완성
	if ( SaveObj->SaveVersion != UUKSaveGame::CURRENT_SAVE_VERSION )
	{
		// TODO: 필요 시 여기서 마이그레이션 처리
		// UE_LOG(LogTemp, Warning, TEXT("Save version mismatch: %d -> %d"),
		//        SaveObj->SaveVersion, UUKSaveGame::CURRENT_SAVE_VERSION);
	}

	RuntimeProgress = SaveObj->QuestProgressMap;
	return true;
}

bool UUKQuestManagerSubsystem::GetProgress(FName QuestId, FQuestProgress& OutProgress) const
{
	if ( const FQuestProgress* P = RuntimeProgress.Find(QuestId) )
	{
		OutProgress = *P;
		return true;
	}
	return false;
}