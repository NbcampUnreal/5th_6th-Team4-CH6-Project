#include "Dialogue/UKDialogueTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

static bool UK_TestLoadDialogueJson()
{
	const FString FullPath = FPaths::ProjectContentDir() / TEXT("Dialogue/quest_test/Dialogues_Test_v1.json");

	FString JsonStr;
	if ( !FFileHelper::LoadFileToString(JsonStr, *FullPath) )
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueTest] Cannot read: %s"), *FullPath);
		return false;
	}

	FUKDialogueRoot Root;
	if ( !FJsonObjectConverter::JsonObjectStringToUStruct(JsonStr, &Root, 0, 0) )
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueTest] Parse failed"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[DialogueTest] Loaded dialogues=%d"), Root.dialogues.Num());
	if ( Root.dialogues.Num() > 0 )
	{
		UE_LOG(LogTemp, Log, TEXT("[DialogueTest] First dialogueId=%s"), *Root.dialogues[ 0 ].dialogueId);
	}
	return true;
}