#pragma once

#include "CoreMinimal.h"

//정다훈 외 조작금지


/*이 헤더는 퀘스트에 관련된 네이밍 id를 미리 정하는곳
1) 이벤트 이름 규칙(포맷)을 정해놓기
2) 자주 쓰는 이벤트는 “상수(const)로” 만들어두기
*/


/*
UKQuestEvents.h (초기 단계 운영 방침)
----------------------------------
- 이 파일은 "퀘스트 진행 신호(EventId)" 네이밍 규칙을 문서화하는 용도입니다.
- 초기에는 상수/코드를 거의 두지 않고, 규칙을 주석으로만 유지합니다.
- 이벤트가 반복 사용되기 시작하면(오타/중복이 늘어나면) 그때부터 일부를 FName 상수로 등록합니다.

[용어]
- QuestID: 퀘스트 자체의 ID (예: Q_Start_001)  -> 퀘스트 정의(DA)에서 주로 사용
- EventId: 퀘스트 진행을 위한 사건 ID (예: QuestEvent.EnteredZone.Start) -> EmitQuestEvent에 전달

[EventId 네이밍 규칙(고정)]
- 포맷: "QuestEvent.<Category>.<Detail>"
- 카테고리 예시:
  - EnteredZone: 구역 진입
  - TalkedTo: NPC 대화/만남
  - GotItem: 아이템 획득
  - Killed: 몬스터 처치
  - Custom: 기타 커스텀 사건

[ZoneName 규칙(이미 확정)]
- 공통사용(Common) 제외 시, 반드시 동일 ZoneName 사용:
  - Start / Steampunk / ... (폴더/에셋/리소스/이벤트 모두 동일 ZoneName)
- 공통은 "Common"으로 통일 권장

[예시]
- QuestEvent.EnteredZone.Start
- QuestEvent.EnteredZone.Steampunk
- QuestEvent.TalkedTo.OldMan
- QuestEvent.GotItem.Permit
- QuestEvent.Killed.Wolf

[상수 등록 기준(나중에)]
- 상수로 등록: 자주 쓰는 이벤트(구역 진입, 핵심 NPC 대화, 핵심 아이템 획득 등)
- 상수로 미등록: 종류가 너무 많은 것(모든 몬스터 처치 등)은 문자열 조합/헬퍼로 생성
*/


/*
QuestID = “퀘스트 자체의 ID”
ex: Q_Start_001
 이건 Quest 정의(DataAsset)에서 주로 씀

EventId = “퀘스트를 진행시키는 사건의 ID”
예: QuestEvent.EnteredZone.Start
 이게 UKQuestEvents.h에서 정하는 대상
*/


/*
Quest Event ID Naming Rule (FName):
- QuestEvent.TalkedTo.<NpcId>
- QuestEvent.GotItem.<ItemId>
- QuestEvent.Killed.<MonsterId>
- QuestEvent.EnteredZone.<ZoneName>
- QuestEvent.Custom.<Something>

ZoneName rule:
- 공통이 아니라면 반드시 Start/Steampunk 같은 동일 ZoneName을 사용.
- 공통이면 Common 사용.
*/

namespace UKQuestEvent
{
	// Zone Enter (예시)
	//static const FName EnteredZone_Start(TEXT("QuestEvent.EnteredZone.Start"));
	//static const FName EnteredZone_Steampunk(TEXT("QuestEvent.EnteredZone.Steampunk"));

	// TalkedTo (예시)
	//static const FName TalkedTo_OldMan(TEXT("QuestEvent.TalkedTo.OldMan"));

	// Item (예시)
	//static const FName GotItem_Permit(TEXT("QuestEvent.GotItem.Permit"));
}
