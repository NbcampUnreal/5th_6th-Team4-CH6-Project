#include "Systems/Time/UK_TimeSubsystem.h"

void UUK_TimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TimeAccumulator += DeltaTime;
	
	if (TimeAccumulator >= SecondsPerGameHour)
	{
		CurrentHour = (CurrentHour + 1) % 24;
		TimeAccumulator = 0.f;
		
		OnHourChanged.Broadcast(CurrentHour);
		
		UE_LOG(LogTemp, Warning, TEXT("현재 월드 시간: %d시"), CurrentHour);
	}
}
