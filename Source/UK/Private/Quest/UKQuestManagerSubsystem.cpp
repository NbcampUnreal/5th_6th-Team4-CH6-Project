#include "Quest/UKQuestManagerSubsystem.h"

#include "DataAsset/Data/UK_ItemData.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SoftObjectPath.h"
#include "server/UKSaveGame.h"

// 인벤토리 관련
#include "ActorComponent/UK_InventoryComponent.h"
#include "GameFramework/Pawn.h"

// 캐릭터 xp 부여관련
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"

// Preset
#include "Quest/UKQuestPresetLibrary.h" // ApplyPresetToProgress

// Quest Definition / Objective / Event Parsing
#include "Quest/DataAsset/UKQuestDefinitionAsset.h"
#include "Quest/UKQuestObjectiveTypes.h"
#include "Quest/UKQuestEventParsing.h"
#include "Systems/Data/UK_InGameSave.h"


// [1] Subsystem Lifecycle
void UUKQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// [Preset] Load
	if ( !PresetAssetPath.IsValid() )
	{
		PresetAssetPath = FSoftObjectPath(TEXT("DataAsset'/Game/Quests/Preset/DA_QuestPresets.DA_QuestPresets'"));
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
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Quest][Preset] PresetAsset loaded OK. Path=%s"),
				*PresetAssetPath.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] PresetAssetPath invalid."));
	}


	// [ItemDataTable] Load
	if ( !ItemDataTable ) // 에디터에서 직접 할당했으면 그걸 우선 사용
	{
		if ( !ItemDataTablePath.IsValid() )
		{
			// 경로 하드코딩
			ItemDataTablePath = FSoftObjectPath(TEXT("DataTable'/Game/ItemData/DT_ItemTableble.DT_ItemTableble'"));
		}

		if ( ItemDataTablePath.IsValid() )
		{
			UObject* LoadedItemDT = ItemDataTablePath.TryLoad();
			ItemDataTable = Cast<UDataTable>(LoadedItemDT);

			if ( !ItemDataTable )
			{
				UE_LOG(LogTemp, Warning, TEXT("[ItemID] ItemDataTable load FAILED. Path=%s"),
					*ItemDataTablePath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[ItemID] ItemDataTable loaded OK. Path=%s"),
					*ItemDataTablePath.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ItemID] ItemDataTablePath invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[ItemID] ItemDataTable already assigned in editor."));
	}


	// [NPCDataTable] Load
	if ( !NPCDataTable ) // 에디터에서 직접 할당했으면 그걸 우선 사용
	{
		if ( !NPCDataTablePath.IsValid() )
		{
			// 경로 하드코딩
			NPCDataTablePath = FSoftObjectPath(TEXT("DataTable'/Game/ItemData/DT_NPCTable.DT_NPCTable'"));
		}

		if ( NPCDataTablePath.IsValid() )
		{
			UObject* LoadedNPCDT = NPCDataTablePath.TryLoad();
			NPCDataTable = Cast<UDataTable>(LoadedNPCDT);

			if ( !NPCDataTable )
			{
				UE_LOG(LogTemp, Warning, TEXT("[NPCID] NPCDataTable load FAILED. Path=%s"),
					*NPCDataTablePath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[NPCID] NPCDataTable loaded OK. Path=%s"),
					*NPCDataTablePath.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[NPCID] NPCDataTablePath invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[NPCID] NPCDataTable already assigned in editor."));
	}

	// [MonsterDataTable] Load
	if ( !MonsterDataTable ) // 에디터에서 직접 할당했으면 그걸 우선 사용
	{
		if ( !MonsterDataTablePath.IsValid() )
		{
			// 경로 하드코딩
			MonsterDataTablePath = FSoftObjectPath(TEXT("DataTable'/Game/ItemData/AIMonsterDT/DT_UKMonsterMeta.DT_UKMonsterMeta'"));
		}

		if ( MonsterDataTablePath.IsValid() )
		{
			UObject* LoadedMonsterDT = MonsterDataTablePath.TryLoad();
			MonsterDataTable = Cast<UDataTable>(LoadedMonsterDT);

			if ( !MonsterDataTable )
			{
				UE_LOG(LogTemp, Warning, TEXT("MobID MonsterDataTable load FAILED. Path=%s"),
					*MonsterDataTablePath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("MobID MonsterDataTable loaded OK. Path=%s"),
					*MonsterDataTablePath.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MobID MonsterDataTablePath invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MobID MonsterDataTable already assigned in editor."));
	}

	// [Reward] Load 
	if ( !RewardDataTable ) // 에디터에서 직접 할당했으면 그걸 우선 사용
	{
		if ( !RewardDataTablePath.IsValid() )
		{
			// 경로 하드코딩
			RewardDataTablePath = FSoftObjectPath(TEXT("DataTable'/Game/ItemData/DT_RewardTable.DT_RewardTable'"));
		}

		if ( RewardDataTablePath.IsValid() )
		{
			UObject* LoadedDT = RewardDataTablePath.TryLoad();
			RewardDataTable = Cast<UDataTable>(LoadedDT);

			if ( !RewardDataTable )
			{
				UE_LOG(LogTemp, Warning, TEXT("[RewardID] RewardDataTable load FAILED. Path=%s"),
					*RewardDataTablePath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[RewardID] RewardDataTable loaded OK. Path=%s"),
					*RewardDataTablePath.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[RewardID] RewardDataTablePath invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[RewardID] RewardDataTable already assigned in editor."));
	}

	// [Quest Definitions] Auto Scan & Register
	{
		const FName ScanPath = FName(TEXT("/Game/Quests"));

		FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& Registry = ARM.Get();

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add(ScanPath);

		// UE5 권장: ClassPaths 사용
		Filter.ClassPaths.Add(UUKQuestDefinitionAsset::StaticClass()->GetClassPathName());

		TArray<FAssetData> Assets;
		Registry.GetAssets(Filter, Assets);

		int32 RegisteredCount = 0;

		for ( const FAssetData& AD : Assets )
		{
			// 로드해서 실제 UObject 얻기
			UObject* LoadedObj = AD.GetAsset();
			const UUKQuestDefinitionAsset* Def = Cast<UUKQuestDefinitionAsset>(LoadedObj);
			if ( !Def ) continue;

			if ( RegisterQuestDefinition(Def) )
			{
				RegisteredCount++;
				UE_LOG(LogTemp, Log, TEXT("[Quest][DefScan] Registered: %s"), *Def->QuestId.ToString());
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[Quest][DefScan] Done. Found=%d Registered=%d Path=%s"),
			Assets.Num(), RegisteredCount, *ScanPath.ToString());
	}

	// [Condition Definitions] Auto Scan & Register
	{
		const FName ScanPath = FName(TEXT("/Game/Quests"));

		FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& Registry = ARM.Get();

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add(ScanPath);

		// UE5 권장: ClassPaths 사용
		Filter.ClassPaths.Add(UUKQuestConditionAsset::StaticClass()->GetClassPathName());

		TArray<FAssetData> Assets;
		Registry.GetAssets(Filter, Assets);

		int32 RegisteredCount = 0;

		for ( const FAssetData& AD : Assets )
		{
			UObject* LoadedObj = AD.GetAsset();
			const UUKQuestConditionAsset* Cond = Cast<UUKQuestConditionAsset>(LoadedObj);
			if ( !Cond ) continue;

			if ( RegisterConditionDefinition(Cond) )
			{
				RegisteredCount++;
				UE_LOG(LogTemp, Log, TEXT("[Quest][ConditionScan] Registered: %s"), *Cond->ConditionId.ToString());
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[Quest][ConditionScan] Done. Found=%d Registered=%d Path=%s"),
			Assets.Num(), RegisteredCount, *ScanPath.ToString());
	}

	// [Startup AlreadyGet] Auto Start
	{
		int32 AutoStartedCount = 0;

		for ( const TPair<FName, TObjectPtr<const UUKQuestDefinitionAsset>>& Pair : QuestDefinitions )
		{
			const UUKQuestDefinitionAsset* Def = Pair.Value;
			if ( !Def )
			{
				continue;
			}

			if ( !Def->bStartAlreadyGet )
			{
				continue;
			}

			if ( Def->QuestId.IsNone() )
			{
				continue;
			}

			const bool bStarted = StartQuest(Def->QuestId);

			UE_LOG(
				LogTemp,
				Log,
				TEXT("[Quest][AlreadyGet] Quest=%s bStartAlreadyGet=1 Result=%d"),
				*Def->QuestId.ToString(),
				bStarted ? 1 : 0
			);

			if ( bStarted )
			{
				AutoStartedCount++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[Quest][AlreadyGet] AutoStartedCount=%d"), AutoStartedCount);
	}

	BuildItemIDCache();
	BuildNPCIDCache();
	BuildMobIDCache();
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
	if ( TagStr == "WRP" ) { OutTag = EUKQuestTag::WRP; return true; }
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

bool UUKQuestManagerSubsystem::RegisterConditionDefinition(const UUKQuestConditionAsset* ConditionAsset)
{
	if ( !ConditionAsset || ConditionAsset->ConditionId.IsNone() )
	{
		return false;
	}

	ConditionDefinitions.Add(ConditionAsset->ConditionId, ConditionAsset);
	return true;
}

const UUKQuestConditionAsset* UUKQuestManagerSubsystem::GetConditionDefinition(FName ConditionId) const
{
	if ( const TObjectPtr<const UUKQuestConditionAsset>* Found = ConditionDefinitions.Find(ConditionId) )
	{
		return Found->Get();
	}

	return nullptr;
}

bool UUKQuestManagerSubsystem::EvaluateCompareInt(int32 Lhs, EUKConditionCompareOp Op, int32 Rhs) const
{
	switch ( Op )
	{
	case EUKConditionCompareOp::Equal:          return Lhs == Rhs;
	case EUKConditionCompareOp::NotEqual:       return Lhs != Rhs;
	case EUKConditionCompareOp::Greater:        return Lhs > Rhs;
	case EUKConditionCompareOp::GreaterOrEqual: return Lhs >= Rhs;
	case EUKConditionCompareOp::Less:           return Lhs < Rhs;
	case EUKConditionCompareOp::LessOrEqual:    return Lhs <= Rhs;
	default:                                    return false;
	}
}

bool UUKQuestManagerSubsystem::EvaluateCompareBool(bool bLhs, EUKConditionCompareOp Op, bool bRhs) const
{
	switch ( Op )
	{
	case EUKConditionCompareOp::Equal:    return bLhs == bRhs;
	case EUKConditionCompareOp::NotEqual: return bLhs != bRhs;
	default:                              return false;
	}
}

FName UUKQuestManagerSubsystem::ResolveQuestIdForClause(const FUKQuestConditionClause& Clause, FName OwnerQuestId) const
{
	if ( !Clause.TargetQuestId.IsNone() )
	{
		return Clause.TargetQuestId;
	}

	return OwnerQuestId;
}

bool UUKQuestManagerSubsystem::EvaluateClause(const FUKQuestConditionClause& Clause, FName OwnerQuestId) const
{
	switch ( Clause.OperandType )
	{
	case EUKConditionOperandType::QuestFlag:
	{
		const FName TargetQuestId = ResolveQuestIdForClause(Clause, OwnerQuestId);
		if ( TargetQuestId.IsNone() || Clause.KeyName.IsNone() )
		{
			return false;
		}

		const bool bHas = HasQuestFlag(TargetQuestId, Clause.KeyName);
		return EvaluateCompareBool(bHas, Clause.CompareOp, Clause.BoolValue);
	}

	case EUKConditionOperandType::QuestCounter:
	{
		const FName TargetQuestId = ResolveQuestIdForClause(Clause, OwnerQuestId);
		if ( TargetQuestId.IsNone() || Clause.KeyName.IsNone() )
		{
			return false;
		}

		const int32 CurrentValue = GetQuestCounterValue(TargetQuestId, Clause.KeyName);
		return EvaluateCompareInt(CurrentValue, Clause.CompareOp, Clause.IntValue);
	}

	case EUKConditionOperandType::QuestState:
	{
		const FName TargetQuestId = ResolveQuestIdForClause(Clause, OwnerQuestId);
		if ( TargetQuestId.IsNone() )
		{
			return false;
		}

		FQuestProgress Progress;
		const bool bStarted = GetProgress(TargetQuestId, Progress);

		bool bValue = false;

		switch ( Clause.StateField )
		{
		case EUKQuestStateField::Accepted:
			bValue = bStarted;
			break;

		case EUKQuestStateField::Completed:
			bValue = ( bStarted && Progress.bCompleted );
			break;

		case EUKQuestStateField::Failed:
			bValue = false;
			break;

		default:
			bValue = false;
			break;
		}

		UE_LOG(LogTemp, Warning,
			TEXT("[Quest][Clause][QuestState] Owner=%s Target=%s Started=%d Completed=%d StateField=%d Value=%d CompareBool=%d"),
			*OwnerQuestId.ToString(),
			*TargetQuestId.ToString(),
			bStarted ? 1 : 0,
			( bStarted && Progress.bCompleted ) ? 1 : 0,
			static_cast< int32 >( Clause.StateField ),
			bValue ? 1 : 0,
			Clause.BoolValue ? 1 : 0);

		return EvaluateCompareBool(bValue, Clause.CompareOp, Clause.BoolValue);
	}

	case EUKConditionOperandType::GlobalFlag:
		// 1차 구현 보류
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Condition] GlobalFlag not implemented yet."));
		return false;

	case EUKConditionOperandType::GlobalCounter:
		// 1차 구현 보류
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Condition] GlobalCounter not implemented yet."));
		return false;

	case EUKConditionOperandType::Custom:
	default:
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Condition] Custom / unsupported clause."));
		return false;
	}
}

bool UUKQuestManagerSubsystem::EvaluateConditionAsset(const UUKQuestConditionAsset* ConditionAsset, FName OwnerQuestId) const
{
	if ( !ConditionAsset )
	{
		return false;
	}

	// Clause가 없으면 true 처리
	if ( ConditionAsset->Clauses.Num() == 0 )
	{
		return true;
	}

	if ( ConditionAsset->GroupOp == EUKConditionGroupOp::AllOf )
	{
		for ( const FUKQuestConditionClause& Clause : ConditionAsset->Clauses )
		{
			if ( !EvaluateClause(Clause, OwnerQuestId) )
			{
				return false;
			}
		}
		return true;
	}

	if ( ConditionAsset->GroupOp == EUKConditionGroupOp::AnyOf )
	{
		for ( const FUKQuestConditionClause& Clause : ConditionAsset->Clauses )
		{
			if ( EvaluateClause(Clause, OwnerQuestId) )
			{
				return true;
			}
		}
		return false;
	}

	return false;
}

bool UUKQuestManagerSubsystem::EvaluateConditionGroup(const TArray<FName>& ConditionIds, FName OwnerQuestId) const
{
	if ( ConditionIds.Num() == 0 )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][ConditionGroup] Quest=%s No conditions -> true"), *OwnerQuestId.ToString());
		return true;
	}

	for ( const FName& ConditionId : ConditionIds )
	{
		if ( ConditionId.IsNone() )
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Quest][ConditionGroup] Quest=%s Checking Condition=%s"),
			*OwnerQuestId.ToString(), *ConditionId.ToString());

		const UUKQuestConditionAsset* ConditionAsset = GetConditionDefinition(ConditionId);
		if ( !ConditionAsset )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][ConditionGroup] Missing ConditionAsset: %s"), *ConditionId.ToString());
			return false;
		}

		const bool bPassed = EvaluateConditionAsset(ConditionAsset, OwnerQuestId);

		UE_LOG(LogTemp, Warning, TEXT("[Quest][ConditionGroup] Condition=%s Passed=%d"),
			*ConditionId.ToString(), bPassed ? 1 : 0);

		if ( !bPassed )
		{
			return false;
		}
	}

	return true;
}

bool UUKQuestManagerSubsystem::CanStartQuest(FName QuestId) const
{
	if ( QuestId.IsNone() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][CanStart] QuestId is None"));
		return false;
	}

	if ( RuntimeProgress.Contains(QuestId) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][CanStart] Already started: %s"), *QuestId.ToString());
		return false;
	}

	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][CanStart] Missing QuestDefinition: %s"), *QuestId.ToString());
		return false;
	}

	const bool bResult = EvaluateConditionGroup(Def->StartConditionIds, QuestId);

	UE_LOG(LogTemp, Warning, TEXT("[Quest][CanStart] Quest=%s StartCondCount=%d Result=%d"),
		*QuestId.ToString(),
		Def->StartConditionIds.Num(),
		bResult ? 1 : 0);

	return bResult;
}

bool UUKQuestManagerSubsystem::CanCompleteQuestByConditions(FName QuestId) const
{
	if ( QuestId.IsNone() )
	{
		return false;
	}

	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def )
	{
		return false;
	}

	return EvaluateConditionGroup(Def->CompleteConditionIds, QuestId);
}

bool UUKQuestManagerSubsystem::HasQuestFlag(FName QuestId, FName Category) const
{
	if ( QuestId.IsNone() || Category.IsNone() )
	{
		return false;
	}

	const FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P )
	{
		return false;
	}

	return HasFlag(*P, MakeFlagKey(QuestId, Category));
}

bool UUKQuestManagerSubsystem::SetQuestFlag(FName QuestId, FName Category)
{
	if ( QuestId.IsNone() || Category.IsNone() )
	{
		return false;
	}

	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P )
	{
		return false;
	}

	SetFlag(*P, MakeFlagKey(QuestId, Category));
	return true;
}

int32 UUKQuestManagerSubsystem::GetQuestCounterValue(FName QuestId, FName CounterName) const
{
	if ( QuestId.IsNone() || CounterName.IsNone() )
	{
		return 0;
	}

	const FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P )
	{
		return 0;
	}

	return GetCounter(*P, MakeCounterKey(QuestId, CounterName));
}

bool UUKQuestManagerSubsystem::SetQuestCounterValue(FName QuestId, FName CounterName, int32 NewValue)
{
	if ( QuestId.IsNone() || CounterName.IsNone() )
	{
		return false;
	}

	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P )
	{
		return false;
	}

	P->Counters.Add(MakeCounterKey(QuestId, CounterName), NewValue);
	return true;
}

bool UUKQuestManagerSubsystem::AddQuestCounterValue(FName QuestId, FName CounterName, int32 Delta)
{
	if ( QuestId.IsNone() || CounterName.IsNone() )
	{
		return false;
	}

	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P )
	{
		return false;
	}

	const FName CounterKey = MakeCounterKey(QuestId, CounterName);
	const int32 CurrentValue = P->Counters.FindRef(CounterKey);
	P->Counters.Add(CounterKey, CurrentValue + Delta);
	return true;
}

// [3] Quest 기본 API
bool UUKQuestManagerSubsystem::StartQuest(FName QuestId)
{
	if ( QuestId.IsNone() ) return false;

	UE_LOG(LogTemp, Log, TEXT("[Quest] StartQuest called: %s"), *QuestId.ToString());

	if ( RuntimeProgress.Contains(QuestId) )
		return true;

	if ( !CanStartQuest(QuestId) )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest] StartQuest blocked by condition: %s"), *QuestId.ToString());
		return false;
	}

	// 1) 빈 진행도 생성
	FQuestProgress NewProgress{};

	// 경고 StartQuest에서 Definition 존재를 체크
	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest] StartQuest but no Definition registered: %s"), *QuestId.ToString());
	}
	else
	{
		if ( Def->RewardId.IsNone() )
		{
			UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s (no RewardId)"), *QuestId.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s has RewardId=%s (will apply on Complete)"),
				*QuestId.ToString(), *Def->RewardId.ToString());
		}
	}

	// 2) QuestID에서 Tag 파싱 -> 프리셋 적용
	if ( PresetAsset )
	{
		EUKQuestTag Tag;
		if ( ParseQuestTagFromQuestId(QuestId, Tag) )
		{
			const bool bApplied = UUKQuestPresetLibrary::ApplyPresetToProgress(
				PresetAsset,
				QuestId,
				Tag,
				NewProgress
			);

			if ( bApplied )
			{
				UE_LOG(LogTemp, Log, TEXT("[Quest][Preset] Applied. Step=%d Counters=%d Flags=%d"),
					NewProgress.Step,
					NewProgress.Counters.Num(),
					NewProgress.Flags.Num());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] Apply failed: Quest=%s"),
					*QuestId.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] Tag parse FAILED: %s"),
				*QuestId.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Preset] PresetAsset is null. Check PresetAssetPath."));
	}

	// 3) 런타임 등록
	RuntimeProgress.Add(QuestId, NewProgress);
	return true;
}

bool UUKQuestManagerSubsystem::CompleteQuest(FName QuestId)
{
	FQuestProgress* P = RuntimeProgress.Find(QuestId);
	if ( !P ) return false;

	if ( P->bCompleted ) return true;

	// 진짜 완료 상태 기록
	P->bCompleted = true;

	// Completed 플래그 세팅 (명명규칙: F.<QuestID>.Completed)
	SetFlag(*P, MakeFlagKey(QuestId, FName("Completed")));

	// 보상처리
	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( Def && !Def->RewardId.IsNone() )
	{
		UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s RewardId=%s"),
			*QuestId.ToString(),
			*Def->RewardId.ToString());

		ApplyRewardById(Def->RewardId, QuestId);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Quest=%s (no RewardId)"), *QuestId.ToString());
	}

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
			case EUKQuestObjectiveType::Delivered:   return C == EUKQuestEventCategory::Delivered;
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
		if ( bAnyChanged && bAutoCompleteOnObjectivesSatisfied )
		{
			TryAutoCompleteQuest(QuestId);
		}
	}
}

// [5] Save/Load (보존형 저장)
bool UUKQuestManagerSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UUK_InGameSave* SaveObj = nullptr;

	// 1) 기존 슬롯이 있으면 로드해서 기존 필드 보존
	if ( UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) )
	{
		SaveObj = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}

	// 2) 없거나 로드 실패면 새로 생성
	if ( !SaveObj )
	{
		SaveObj = Cast<UUK_InGameSave>(UGameplayStatics::CreateSaveGameObject(UUK_InGameSave::StaticClass()));
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

bool UUKQuestManagerSubsystem::AreObjectivesSatisfied(FName QuestId) const
{
	const FQuestProgress* Prog = RuntimeProgress.Find(QuestId);
	if ( !Prog )
	{
		return false;
	}

	const UUKQuestDefinitionAsset* Def = GetQuestDefinition(QuestId);
	if ( !Def )
	{
		return false;
	}

	for ( const FUKQuestObjectiveDef& Obj : Def->Objectives )
	{
		if ( !IsObjectiveComplete(*Prog, Obj, QuestId) )
		{
			return false;
		}
	}

	return true;
}

// [7] Item EntityID / ItemDataTable
void UUKQuestManagerSubsystem::BuildItemIDCache()
{
	ItemIDToRowName.Empty();

	if ( !ItemDataTable )
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemID] ItemDataTable is null. Cache build skipped."));
		return;
	}

	const TArray<FName> RowNames = ItemDataTable->GetRowNames();

	for ( const FName RowName : RowNames )
	{
		const FUK_ItemData* Row = ItemDataTable->FindRow<FUK_ItemData>(RowName, TEXT("BuildItemIDCache"));
		if ( !Row ) continue;

		// ItemID 컬럼이 비어있으면 스킵
		if ( Row->ItemID.IsNone() )
		{
			UE_LOG(LogTemp, Warning, TEXT("[ItemID] Row has None ItemID. RowName=%s"), *RowName.ToString());
			continue;
		}

		// 중복 ItemID 방지
		if ( ItemIDToRowName.Contains(Row->ItemID) )
		{
			UE_LOG(LogTemp, Error, TEXT("[ItemID] Duplicate ItemID=%s (RowName=%s)"),
				*Row->ItemID.ToString(), *RowName.ToString());
			continue;
		}

		ItemIDToRowName.Add(Row->ItemID, RowName);
	}

	UE_LOG(LogTemp, Log, TEXT("[ItemID] Cache built. Count=%d"), ItemIDToRowName.Num());
}

const FUK_ItemData* UUKQuestManagerSubsystem::GetItemDataByItemID(FName ItemID) const
{
	if ( !ItemDataTable ) return nullptr;
	if ( ItemID.IsNone() ) return nullptr;

	const FName* RowName = ItemIDToRowName.Find(ItemID);
	if ( !RowName )
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemID] Not found in cache: %s"), *ItemID.ToString());
		return nullptr;
	}

	return ItemDataTable->FindRow<FUK_ItemData>(*RowName, TEXT("GetItemDataByItemID"));
}

// [8] NPC EntityID / NPCDataTable
void UUKQuestManagerSubsystem::BuildNPCIDCache()
{
	NPCIDToRowName.Empty();

	if ( !NPCDataTable )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPCID] NPCDataTable is null. Cache build skipped."));
		return;
	}

	const TArray<FName> RowNames = NPCDataTable->GetRowNames();

	for ( const FName RowName : RowNames )
	{
		const FUK_NPCData* Row = NPCDataTable->FindRow<FUK_NPCData>(RowName, TEXT("BuildNPCIDCache"));
		if ( !Row ) continue;

		// NPCID 컬럼이 비어있으면 스킵
		if ( Row->NPCID.IsNone() )
		{
			UE_LOG(LogTemp, Warning, TEXT("[NPCID] Row has None NPCID. RowName=%s"), *RowName.ToString());
			continue;
		}

		// 중복 NPCID 방지
		if ( NPCIDToRowName.Contains(Row->NPCID) )
		{
			UE_LOG(LogTemp, Error, TEXT("[NPCID] Duplicate NPCID=%s (RowName=%s)"),
				*Row->NPCID.ToString(), *RowName.ToString());
			continue;
		}

		NPCIDToRowName.Add(Row->NPCID, RowName);
	}

	UE_LOG(LogTemp, Log, TEXT("[NPCID] Cache built. Count=%d"), NPCIDToRowName.Num());
}

const FUK_NPCData* UUKQuestManagerSubsystem::GetNPCDataByNPCID(FName NPCID) const
{
	if ( !NPCDataTable ) return nullptr;
	if ( NPCID.IsNone() ) return nullptr;

	const FName* RowName = NPCIDToRowName.Find(NPCID);
	if ( !RowName )
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPCID] Not found in cache: %s"), *NPCID.ToString());
		return nullptr;
	}

	return NPCDataTable->FindRow<FUK_NPCData>(*RowName, TEXT("GetNPCDataByNPCID"));
}

// [9] Monster EntityID / MonsterDataTable

void UUKQuestManagerSubsystem::BuildMobIDCache()
{
	MobIDToRowName.Empty();

	if ( !MonsterDataTable )
	{
		UE_LOG(LogTemp, Warning, TEXT("MobID MonsterDataTable is null. Cache build skipped."));
		return;
	}

	const TArray<FName> RowNames = MonsterDataTable->GetRowNames();

	for ( const FName RowName : RowNames )
	{
		const FUK_MonsterMetaRow* Row = MonsterDataTable->FindRow<FUK_MonsterMetaRow>(RowName, TEXT("BuildMobIDCache"));
		if ( !Row ) continue;

		// MobEntityId 컬럼이 비어있으면 스킵
		if ( Row->MobEntityId.IsNone() )
		{
			UE_LOG(LogTemp, Warning, TEXT("MobID Row has None MobEntityId. RowName=%s"), *RowName.ToString());
			continue;
		}

		// 중복 MobID 방지
		if ( MobIDToRowName.Contains(Row->MobEntityId) )
		{
			UE_LOG(LogTemp, Error, TEXT("MobID Duplicate MobEntityId=%s (RowName=%s)"),
				*Row->MobEntityId.ToString(), *RowName.ToString());
			continue;
		}

		MobIDToRowName.Add(Row->MobEntityId, RowName);
	}

	UE_LOG(LogTemp, Log, TEXT("MobID Cache built. Count=%d"), MobIDToRowName.Num());
}

const FUK_MonsterMetaRow* UUKQuestManagerSubsystem::GetMonsterDataByMobID(FName MobID) const
{
	if ( !MonsterDataTable ) return nullptr;
	if ( MobID.IsNone() ) return nullptr;

	const FName* RowName = MobIDToRowName.Find(MobID);
	if ( !RowName )
	{
		UE_LOG(LogTemp, Warning, TEXT("MobID Not found in cache: %s"), *MobID.ToString());
		return nullptr;
	}

	return MonsterDataTable->FindRow<FUK_MonsterMetaRow>(*RowName, TEXT("GetMonsterDataByMobID"));
}

// [10] Reward / RewardDataTable / 보상적용

// 인벤토리 컴포넌트 찾는 함수
UUK_InventoryComponent* UUKQuestManagerSubsystem::GetPlayerInventoryComponent() const
{
	UWorld* World = GetWorld();
	if ( !World ) return nullptr;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if ( !PlayerPawn ) return nullptr;

	return PlayerPawn->FindComponentByClass<UUK_InventoryComponent>();
}

// 아이템 지급
void UUKQuestManagerSubsystem::GiveQuestReward(FName ItemID, int32 Amount)
{
	if ( ItemID.IsNone() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GiveQuestReward failed: ItemID is None"));
		return;
	}

	if ( Amount <= 0 )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GiveQuestReward failed: invalid Amount=%d, ItemID=%s"),
			Amount, *ItemID.ToString());
		return;
	}

	const FUK_ItemData* Data = GetItemDataByItemID(ItemID);
	if ( !Data )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GiveQuestReward failed: Item data not found for ItemID=%s"),
			*ItemID.ToString());
		return;
	}

	// 인벤토리 컴포넌트 찾기
	UUK_InventoryComponent* InventoryComp = GetPlayerInventoryComponent();
	if ( !InventoryComp )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GiveQuestReward failed: InventoryComponent not found. ItemID=%s"),
			*ItemID.ToString());
		return;
	}

	// ItemID -> RowName 변환
	const FName* FoundRowName = ItemIDToRowName.Find(ItemID);
	if ( !FoundRowName )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] GiveQuestReward failed: RowName not found for ItemID=%s"),
			*ItemID.ToString());
		return;
	}

	// 인벤토리는 RowName 기준으로 동작하므로 RowName 전달
	// 반환값 해석이 애매한 상태
	const int32 AddResult = InventoryComp->AddItem(*FoundRowName, Amount);
	if ( AddResult != 0 )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Reward] AddItem partial/failed. ItemID=%s RowName=%s Requested=%d Result=%d"),
			*ItemID.ToString(),
			*FoundRowName->ToString(),
			Amount,
			AddResult);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Reward] Item Given: ItemID=%s RowName=%s Name=%s x%d"),
		*ItemID.ToString(),
		*FoundRowName->ToString(),
		*Data->ItemName.ToString(),
		Amount);
}

// ApplyRewardById 구현
bool UUKQuestManagerSubsystem::ApplyRewardById(FName RewardId, FName QuestId)
{
	if ( RewardId.IsNone() )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] RewardId is None. Quest=%s"), *QuestId.ToString());
		return false;
	}

	if ( !RewardDataTable )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] RewardDataTable is null. Quest=%s RewardId=%s"),
			*QuestId.ToString(), *RewardId.ToString());
		return false;
	}

	// RowName == RewardId 규칙
	const FUKRewardRow* Row = RewardDataTable->FindRow<FUKRewardRow>(RewardId, TEXT("QuestRewardLookup"));
	if ( !Row )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] Row NOT found. Quest=%s RewardId=%s"),
			*QuestId.ToString(), *RewardId.ToString());
		return false;
	}

	// 1) Gold / XP
	if ( Row->Gold != 0 )
	{
		UUK_InventoryComponent* InventoryComp = GetPlayerInventoryComponent();
		if ( !InventoryComp )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] InventoryComponent not found. Gold reward failed. Quest=%s RewardId=%s"),
				*QuestId.ToString(), *RewardId.ToString());
		}
		else
		{
			const bool bGoldAdded = InventoryComp->AddGold(Row->Gold);
			if ( bGoldAdded )
			{
				UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Gold +%d applied. CurrentGold=%d (Quest=%s RewardId=%s)"),
					Row->Gold,
					InventoryComp->GetGold(),
					*QuestId.ToString(),
					*RewardId.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] AddGold failed. Value=%d Quest=%s RewardId=%s"),
					Row->Gold,
					*QuestId.ToString(),
					*RewardId.ToString());
			}
		}
	}

	if ( Row->XP != 0 )
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if ( !PlayerPawn )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] PlayerPawn not found. XP reward failed. Quest=%s RewardId=%s"),
				*QuestId.ToString(), *RewardId.ToString());
		}
		else if ( IAbilitySystemInterface* ASIPlayer = Cast<IAbilitySystemInterface>(PlayerPawn) )
		{
			if ( UAbilitySystemComponent* PlayerASC = ASIPlayer->GetAbilitySystemComponent() )
			{
				const float CurrentExp = PlayerASC->GetNumericAttribute(
					UUK_PlayerStatusAttributeSet::GetEXPAttribute());

				PlayerASC->SetNumericAttributeBase(
					UUK_PlayerStatusAttributeSet::GetEXPAttribute(),
					CurrentExp + Row->XP);

				UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] XP +%d applied. CurrentEXP=%.1f (Quest=%s RewardId=%s)"),
					Row->XP,
					CurrentExp + Row->XP,
					*QuestId.ToString(),
					*RewardId.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] Player ASC not found. XP reward failed. Quest=%s RewardId=%s"),
					*QuestId.ToString(), *RewardId.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] PlayerPawn has no AbilitySystemInterface. XP reward failed. Quest=%s RewardId=%s"),
				*QuestId.ToString(), *RewardId.ToString());
		}
	}

	// 2) Items 지급
	for ( const FUKRewardItemGrant& Grant : Row->Items )
	{
		if ( Grant.ItemID.IsNone() || Grant.Amount <= 0 )
			continue;

		GiveQuestReward(Grant.ItemID, Grant.Amount);

		UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] Item %s x%d (Quest=%s RewardId=%s)"),
			*Grant.ItemID.ToString(), Grant.Amount,
			*QuestId.ToString(), *RewardId.ToString());
	}

	// 3) SetFlags 반영 (전역/퀘스트 둘 다 가능)
	// - 현재는 QuestProgress 내부 Flags(TSet<FName>)에 넣는 방식으로만 처리(뼈대)
	// - 전역 플래그(F.Common.*)는 SaveGame의 별도 GlobalFlags로 확장 예정
	if ( FQuestProgress* Prog = RuntimeProgress.Find(QuestId) )
	{
		for ( const FName FlagKey : Row->SetFlags )
		{
			if ( !FlagKey.IsNone() )
			{
				Prog->Flags.Add(FlagKey);
				UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] SetFlag %s"), *FlagKey.ToString());
			}
		}

		// 4) AddCounters 반영
		for ( const auto& Pair : Row->AddCounters )
		{
			const FName CounterKey = Pair.Key;
			const int32 Delta = Pair.Value;

			if ( CounterKey.IsNone() || Delta == 0 )
				continue;

			const int32 Cur = Prog->Counters.FindRef(CounterKey);
			Prog->Counters.Add(CounterKey, Cur + Delta);

			UE_LOG(LogTemp, Log, TEXT("[Quest][Reward] AddCounter %s %+d => %d"),
				*CounterKey.ToString(), Delta, Cur + Delta);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Quest][Reward] Quest progress not found when applying flags/counters. Quest=%s"),
			*QuestId.ToString());
	}

	return true;
}


// [11] Quest Definition 등록/조회
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


// [12] Progress Helpers (명명규칙 키 생성)
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

// [13] Auto Complete
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
	ItemIDToRowName.Empty();
	NPCIDToRowName.Empty();
	MobIDToRowName.Empty();
	RuntimeProgress.Empty();
	QuestDefinitions.Empty();
	ConditionDefinitions.Empty();

	PresetAsset = nullptr;
	ItemDataTable = nullptr;
	NPCDataTable = nullptr;
	MonsterDataTable = nullptr;
	RewardDataTable = nullptr;
	Super::Deinitialize();
}