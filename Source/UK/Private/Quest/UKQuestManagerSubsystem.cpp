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

bool UUKQuestManagerSubsystem::CompleteQuest(FName QuestId)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->bCompleted = true;
	return true;
}

bool UUKQuestManagerSubsystem::SetQuestStep(FName QuestId, int32 NewStep)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->Step = NewStep;
	return true;
}

void UUKQuestManagerSubsystem::EmitQuestEvent(FName EventId)
{
	// TODO: 나중에 DA/JSON과 연결해서 이벤트->진행도 갱신 로직을 여기서 수행
	// 지금은 뼈대만 둡니다.
}

bool UUKQuestManagerSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UUKSaveGame* SaveObj = Cast<UUKSaveGame>(UGameplayStatics::CreateSaveGameObject(UUKSaveGame::StaticClass()));
	if ( !SaveObj ) return false;

	// 기존 SaveGame 필드가 있을 수 있으니, QuestProgressMap만 갱신하는 방식으로
	SaveObj->QuestProgressMap = RuntimeProgress;

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