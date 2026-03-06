#include "Animation/AnimNotify/UK_VFXAnimNotify.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
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
	
    UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraVFX,
        FinalTarget,
        SocketName,
        LocationOffset,
        RotationOffset,
        EAttachLocation::SnapToTargetIncludingScale,
        true,
        true
    );
}