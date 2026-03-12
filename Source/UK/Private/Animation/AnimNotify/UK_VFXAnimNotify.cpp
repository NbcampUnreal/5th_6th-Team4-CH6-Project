#include "Animation/AnimNotify/UK_VFXAnimNotify.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"

void UUK_VFXAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    
    if (!MeshComp || !NiagaraVFX) return;

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor) return;

    USceneComponent* FinalTarget = nullptr;

    TArray<USceneComponent*> Children;
    MeshComp->GetChildrenComponents(true, Children);

    for (USceneComponent* Child : Children)
    {
        if (Child && Child->DoesSocketExist(SocketName))
        {
            FinalTarget = Child;
            break;
        }
    }

    if (!FinalTarget)
    {
        TArray<AActor*> AttachedActors;
        OwnerActor->GetAttachedActors(AttachedActors);
        for (AActor* Actor : AttachedActors)
        {
            UMeshComponent* WeaponMesh = Actor->FindComponentByClass<UMeshComponent>();
            if (WeaponMesh && WeaponMesh->DoesSocketExist(SocketName))
            {
                FinalTarget = WeaponMesh;
                break;
            }
        }
    }

    if (!FinalTarget) 
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Notify: [%s] 소켓을 찾을 수 없어 캐릭터 메쉬에 소환합니다!"), *SocketName.ToString());
        FinalTarget = MeshComp;
    }
	
	// 4. 이펙트 부착 소환 (다시 따라오게 설정)
	UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraVFX,
		FinalTarget,
		SocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::SnapToTargetIncludingScale,
		true, // bAutoDestroy
		true  // bAutoActivate
	);

	// 5. 스케일 및 시간 설정
	if (SpawnedVFX)
	{
		SpawnedVFX->SetRelativeScale3D(VFXScale);

		if (VFXDuration > 0.0f)
		{
			// 타이머를 사용하여 지정된 시간(VFXDuration) 후에 컴포넌트 파괴
			FTimerHandle TimerHandle;
			MeshComp->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [SpawnedVFX]()
			{
				if (IsValid(SpawnedVFX))
				{
					SpawnedVFX->DestroyComponent();
				}
			}, VFXDuration, false);
		}
	}
}