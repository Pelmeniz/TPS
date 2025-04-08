#pragma once

#include "CoreMinimal.h"
#include "AnimNotify_WeaponEjectMagazine.generated.h"

UCLASS()
class TPS_API UAnimNotify_WeaponEjectMagazine : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
