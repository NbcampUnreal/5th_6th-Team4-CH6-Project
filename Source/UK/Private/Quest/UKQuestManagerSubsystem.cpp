#include "Quest/UKQuestManagerSubsystem.h"

#include "DataAsset/Data/UK_ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "server/UKSaveGame.h"

// Preset
#include "Quest/UKQuestPresetLibrary.h" // ApplyPresetToProgress

// Quest Definition / Objective / Event Parsing
#include "Quest/DataAsset/UKQuestDefinitionAsset.h"
#include "Quest/UKQuestObjectiveTypes.h"
#include "Quest/UKQuestEventParsing.h"


// [1] Subsystem Lifecycle
void UUKQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// (선택) PresetAssetPath가 비어있다면 기본 경로를 코드에 박아두는 방식(초기 뼈대용)
	// 아래 문자열은 "DA_QuestPresets 에셋 우클릭 -> Copy Reference"로 가져온 값으로 교체
	// 예: DataAsset'/Game/Quests/Presets/DA_QuestPresets.DA_QuestPresets'
	if ( !PresetAssetPath.IsValid() )
	{
		PresetAssetPath = FSoftObjectPath(TEXT("DataAsset'/Game/Quests/Presets/DA_QuestPresets.DA_QuestPresets'"));
	}

	if ( PresetAssetPath.IsValid() )
	{
		UObject* Loaded = PresetAssetPath.TryLoad();
		PresetAsset = Cast<UUKQuestPresetAsset>(Loaded);

		if ( !PresetAsset )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] PresetAsset load FAILED. Path=%s"),
				*PresetAssetPath.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] PresetAssetPath invalid."));
	}
}


// [2] Preset Helper
bool UUKQuestManagerSubsystem::ParseQuestTagFromQuestId(FName QuestId, EUKQuestTag& OutTag) const
{
	// QuestID 규칙: Q_<Zone>_<M|S|T>_<Tag>_<NNN>
	const FString S = QuestId.ToString();

	TArray<FString> Parts;
	S.ParseIntoArray(Parts, TEXT("_"), true);

	// 예: Q Start S HNT 001 -> 5토큰
	// 즉, 퀘스트 영구키의 정상확인장치
	if ( Parts.Num() < 5 ) return false;

	const FString& TagStr = Parts[ 3 ];

	if ( TagStr == "WPN" ) { OutTag = EUKQuestTag::WPN; return true; }
	if ( TagStr == "SKL" ) { OutTag = EUKQuestTag::SKL; return true; }
	if ( TagStr == "CFT" ) { OutTag = EUKQuestTag::CFT; return true; }
	if ( TagStr == "EXP" ) { OutTag = EUKQuestTag::EXP; return true; }
	if ( TagStr == "HNT" ) { OutTag = EUKQuestTag::HNT; return true; }
	if ( TagStr == "DLV" ) { OutTag = EUKQuestTag::DLV; return true; }
	if ( TagStr == "DIA" ) { OutTag = EUKQuestTag::DIA; return true; }
	if ( TagStr == "ESC" ) { OutTag = EUKQuestTag::ESC; return true; }
	if ( TagStr == "DEF" ) { OutTag = EUKQuestTag::DEF; return true; }
	if ( TagStr == "COL" ) { OutTag = EUKQuestTag::COL; return true; }
	if ( TagStr == "BOS" ) { OutTag = EUKQuestTag::BOS; return true; }
	if ( TagStr == "DNG" ) { OutTag = EUKQuestTag::DNG; return true; }

	return false;
}


// [3] Quest 기본 API
bool UUKQuestManagerSubsystem::StartQuest(FName QuestId)
{
	if ( QuestId.IsNone() ) return false;

	UE_LOG(LogTemp, Log, TEXT("[Quest] StartQuest called: %s"), *QuestId.ToString());

	if ( RuntimeProgress.Contains(QuestId) )
		return true;

	// 1) 빈 진행도 생성
	FQuestProgress NewProgress{};

	// 경고 StartQuest에서 Definition 존재를 체크
	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest] StartQuest but no Definition registered: %s"), *QuestId.ToString());
	}

	// 2) QuestID에서 Tag 파싱 -> 프리셋 적용
	if ( PresetAsset )
	{
		EUKQuestTag Tag;
		if ( ParseQuestTagFromQuestId(QuestId, Tag) )
		{
			UUKQuestPresetLibrary::ApplyPresetToProgress(PresetAsset, Tag, NewProgress);

			UE_LOG(LogTemp, Log, TEXT("[Quest][Preset] Applied. Step=%d Counters=%d Flags=%d"),
				NewProgress.Step, NewProgress.Counters.Num(), NewProgress.Flags.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] Tag parse FAILED: %s"), *QuestId.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] PresetAsset is null. Check PresetAssetPath."));
	}

	// Accepted 플래그 세팅 (명명규칙: F.<QuestID>.Accepted)
	SetFlag(NewProgress, MakeFlagKey(QuestId, FName("Accepted")));

	// 3) 런타임 등록
	RuntimeProgress.Add(QuestId, NewProgress);
	return true;
}

bool UUKQuestManagerSubsystem::CompleteQuest(FName QuestId)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->bCompleted = true;

	// Completed 플래그 세팅 (명명규칙: F.<QuestID>.Completed)
	SetFlag(*P, MakeFlagKey(QuestId, FName("Completed")));

	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( Def && !Def->RewardId.IsNone() )
	{
		UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s RewardId=%s"),
			*QuestId.ToString(),
			*Def->RewardId.ToString());

		// TODO: RewardId -> 실제 보상 지급 연결(Reward DA/DT/Subsystem 등)
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s (no RewardId)"), *QuestId.ToString());
	}
	// - RewardID(R_<QuestID>_<Name>) 기반 보상 지급 로직 연결
	// - 또는 QuestDefinition(DA)의 RewardId를 참조하여 처리

	return true;
}

bool UUKQuestManagerSubsystem::SetQuestStep(FName QuestId, int32 NewStep)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	P->Step = NewStep;
	return true;
}


// [4] Event → Objective 매칭 → Progress 갱신 + 자동완료
void UUKQuestManagerSubsystem::EmitQuestEvent(FName EventId)
{
	// 0) 이벤트 파싱: QuestEvent.<Category>.<Detail>
	FUKQuestParsedEvent Ev;
	if ( !UK_ParseQuestEventId(EventId, Ev) || !Ev.bValid )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest] Invalid EventId: %s"), *EventId.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Quest] EmitQuestEvent: %s (Cat=%d, Detail=%s)"),
		*EventId.ToString(),
		static_cast< int32 >( Ev.Category ),
		*Ev.Detail.ToString());

	// 상태 이벤트는 목표 매칭 전에 "즉시 처리"
	// JSON에서 emit 되는 QuestEvent.Accepted.<QuestId>, QuestEvent.Completed.<QuestId> 처리용
	if ( Ev.Category == EUKQuestEventCategory::Accepted )
	{
		// Detail이 QuestId여야 함: QuestEvent.Accepted.<QuestId>
		StartQuest(Ev.Detail);
		return;
	}
	if ( Ev.Category == EUKQuestEventCategory::Completed )
	{
		CompleteQuest(Ev.Detail);
		return;
	}
	// (선택) Failed도 나중에 FailQuest 만들면 여기서 처리




	// 공통 타입 매칭 람다(중복 선언 방지)
	auto MatchesType = [ & ] (EUKQuestObjectiveType T, EUKQuestEventCategory C) -> bool
		{
			switch ( T )
			{
			case EUKQuestObjectiveType::EnteredZone: return C == EUKQuestEventCategory::EnteredZone;
			case EUKQuestObjectiveType::TalkedTo:    return C == EUKQuestEventCategory::TalkedTo;
			case EUKQuestObjectiveType::GotItem:     return C == EUKQuestEventCategory::GotItem;
			case EUKQuestObjectiveType::Killed:      return C == EUKQuestEventCategory::Killed;
			case EUKQuestObjectiveType::Custom:      return C == EUKQuestEventCategory::Custom;
			default: return false;
			}
		};

	// 1) 활성 퀘스트 순회
	for ( auto& It : RuntimeProgress )
	{
		const FName QuestId = It.Key;
		FQuestProgress& Prog = It.Value;

		if ( Prog.bCompleted )
			continue;

		const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
		if ( !Def )
			continue;

		// [Filter 1] 이번 이벤트 카테고리에 반응할 목표가 하나라도 있는지 빠르게 검사
		bool bHasAnyMatchingType = false;
		for ( const FUKQuestObjectiveDef& ObjDef : Def->Objectives )
		{
			if ( MatchesType(ObjDef.Type, Ev.Category) )
			{
				bHasAnyMatchingType = true;
				break;
			}
		}
		if ( !bHasAnyMatchingType )
			continue;

		// [Filter 2] EnteredZone 이벤트라면 ZoneName 체크 (Common은 예외)
		if ( Ev.Category == EUKQuestEventCategory::EnteredZone )
		{
			const bool bIsCommonQuest = ( Def->ZoneName == FName("Common") );
			if ( !bIsCommonQuest && !Def->ZoneName.IsNone() && Def->ZoneName != Ev.Detail )
			{
				continue;
			}
		}

		bool bAnyChanged = false;

		// 2) 목표 순회하며 매칭
		for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
		{
			if ( IsObjectiveComplete(Prog, Obj, QuestId) )
				continue;

			// Type ↔ Category 매칭
			if ( !MatchesType(Obj.Type, Ev.Category) )
				continue;

			// Target 매칭(Detail == TargetId)
			if ( !Obj.TargetId.IsNone() && Obj.TargetId != Ev.Detail )
				continue;

			UE_LOG(LogTemp, Log, TEXT("[Quest][Match] Quest=%s Obj=%s Type=%d Target=%s"),
				*QuestId.ToString(),
				*Obj.ObjectiveId.ToString(),
				static_cast< int32 >( Obj.Type ),
				*Obj.TargetId.ToString());

			// 3) 카운터 갱신(있으면 1 증가)
			if ( !Obj.CounterName.IsNone() )
			{
				const FName CounterKey = MakeCounterKey(QuestId, Obj.CounterName);
				AddCounter(Prog, CounterKey, 1);
				bAnyChanged = true;

				UE_LOG(LogTemp, Log, TEXT("[Quest][Counter] %s = %d / %d"),
					*CounterKey.ToString(),
					GetCounter(Prog, CounterKey),
					Obj.RequiredCount);
			}

			// 4) 플래그 갱신(CompleteFlagCategory가 있으면)
			if ( !Obj.CompleteFlagCategory.IsNone() )
			{
				// 카운터 목표(RequiredCount>1)면 도달 시 플래그 찍기
				if ( !Obj.CounterName.IsNone() && Obj.RequiredCount > 1 )
				{
					const FName CounterKey = MakeCounterKey(QuestId, Obj.CounterName);
					if ( GetCounter(Prog, CounterKey) >= Obj.RequiredCount )
					{
						const FName FlagKey = MakeFlagKey(QuestId, Obj.CompleteFlagCategory);
						SetFlag(Prog, FlagKey);
						bAnyChanged = true;

						UE_LOG(LogTemp, Log, TEXT("[Quest][Flag] Set %s"), *FlagKey.ToString());
					}
				}
				else
				{
					// 그 외: 이벤트 1회 매칭으로 완료 플래그 찍기
					const FName FlagKey = MakeFlagKey(QuestId, Obj.CompleteFlagCategory);
					SetFlag(Prog, FlagKey);
					bAnyChanged = true;

					UE_LOG(LogTemp, Log, TEXT("[Quest][Flag] Set %s"), *FlagKey.ToString());
				}
			}
		}

		// 5) 변경이 있었다면 자동 완료 시도
		if ( bAnyChanged )
		{
			TryAutoCompleteQuest(QuestId);
		}
	}
}

// [5] Save/Load (보존형 저장)
bool UUKQuestManagerSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UUKSaveGame* SaveObj = nullptr;

	// 1) 기존 슬롯이 있으면 로드해서 기존 필드 보존
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

	// 3) 내가 책임지는 필드만 갱신
	SaveObj->QuestProgressMap = RuntimeProgress;

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

	RuntimeProgress = SaveObj->QuestProgressMap;
	return true;
}

// [6] 조회
bool UUKQuestManagerSubsystem::GetProgress(FName QuestId, FQuestProgress& OutProgress) const
{
	if ( const FQuestProgress* P = RuntimeProgress.Find(QuestId) )
	{
		OutProgress = *P;
		return true;
	}
	return false;
}

// [7] Reward / ItemDataTable (기존 유지)
FUK_ItemData* UUKQuestManagerSubsystem::GetItemData(FName ItemRowName)
{
	if ( !ItemDataTable ) return nullptr;

	return ItemDataTable->FindRow<FUK_ItemData>(ItemRowName, TEXT("QuestRewardLookup"));
}

void UUKQuestManagerSubsystem::GiveQuestReward(FName ItemRowName, int32 Amount)
{
	FUK_ItemData* Data = GetItemData(ItemRowName);
	if ( !Data ) return;

	// 인벤토리 컴포넌트 연결 (추후)
	// InventoryComponent->AddItem(Data, Amount);

	UE_LOG(LogTemp, Warning, TEXT("Reward Given: %s x%d"), *Data->ItemName.ToString(), Amount);
}


// [8] Quest Definition 등록/조회
bool UUKQuestManagerSubsystem::RegisterQuestDefinition(const UUKQuestDefinitionAsset* Definition)
{
	if ( !Definition ) return false;
	if ( Definition->QuestId.IsNone() ) return false;

	QuestDefinitions.Add(Definition->QuestId, Definition);
	return true;
}

const UUKQuestDefinitionAsset* UUKQuestManagerSubsystem::GetQuestDefinition(FName QuestId) const
{
	if ( const TObjectPtr<const UUKQuestDefinitionAsset>* Found = QuestDefinitions.Find(QuestId) )
	{
		return Found->Get();
	}
	return nullptr;
}


// [9] Progress Helpers (명명규칙 키 생성)
FName UUKQuestManagerSubsystem::MakeCounterKey(FName QuestId, FName CounterName) const
{
	// C.<QuestID>.<Name>
	return FName(*FString::Printf(TEXT("C.%s.%s"), *QuestId.ToString(), *CounterName.ToString()));
}

FName UUKQuestManagerSubsystem::MakeFlagKey(FName QuestId, FName Category) const
{
	// F.<QuestID>.<Category>
	return FName(*FString::Printf(TEXT("F.%s.%s"), *QuestId.ToString(), *Category.ToString()));
}

int32 UUKQuestManagerSubsystem::GetCounter(const FQuestProgress& P, FName CounterKey) const
{
	if ( const int32* V = P.Counters.Find(CounterKey) ) return *V;
	return 0;
}

void UUKQuestManagerSubsystem::SetCounter(FQuestProgress& P, FName CounterKey, int32 Value)
{
	P.Counters.FindOrAdd(CounterKey) = Value;
}

void UUKQuestManagerSubsystem::AddCounter(FQuestProgress& P, FName CounterKey, int32 Delta)
{
	const int32 Cur = GetCounter(P, CounterKey);
	SetCounter(P, CounterKey, Cur + Delta);
}

bool UUKQuestManagerSubsystem::HasFlag(const FQuestProgress& P, FName FlagKey) const
{
	return P.Flags.Contains(FlagKey);
}

void UUKQuestManagerSubsystem::SetFlag(FQuestProgress& P, FName FlagKey)
{
	P.Flags.Add(FlagKey);
}

bool UUKQuestManagerSubsystem::IsObjectiveComplete(const FQuestProgress& P, const FUKQuestObjectiveDef& Obj, FName QuestId) const
{
	// 1) 목표 완료 플래그 우선
	if ( !Obj.CompleteFlagCategory.IsNone() )
	{
		const FName FlagKey = MakeFlagKey(QuestId, Obj.CompleteFlagCategory);
		if ( HasFlag(P, FlagKey) ) return true;
	}

	// 2) 카운터 기반 목표면 RequiredCount 비교
	if ( !Obj.CounterName.IsNone() && Obj.RequiredCount > 0 )
	{
		const FName CounterKey = MakeCounterKey(QuestId, Obj.CounterName);
		return GetCounter(P, CounterKey) >= Obj.RequiredCount;
	}

	// 3) 카운터/플래그 둘 다 없다면, 운영상 CompleteFlagCategory를 쓰는 편이 안정적
	return false;
}

// [10] Auto Complete
void UUKQuestManagerSubsystem::TryAutoCompleteQuest(FName QuestId)
{
	FQuestProgress* Prog = RuntimeProgress.Find(QuestId);
	if ( !Prog ) return;
	if ( Prog->bCompleted ) return;

	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def ) return;

	// 목표 전부 완료인지 검사
	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		if ( !IsObjectiveComplete(*Prog, Obj, QuestId) )
		{
			return;
		}
	}

	// 전부 완료 => 완료 처리
	CompleteQuest(QuestId);
	UE_LOG(LogTemp, Log, TEXT("[Quest] Auto Completed: %s"), *QuestId.ToString());
}


void UUKQuestManagerSubsystem::Deinitialize()
{
	RuntimeProgress.Empty();
	QuestDefinitions.Empty();
	PresetAsset = nullptr;
	Super::Deinitialize();
}