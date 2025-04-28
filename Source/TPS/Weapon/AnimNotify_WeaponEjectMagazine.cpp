// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/AnimNotify_WeaponEjectMagazine.h"
#include "TPS/Weapon/WeaponDefault.h"

void UAnimNotify_WeaponEjectMagazine::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (AWeaponDefault* Weapon = Cast<AWeaponDefault>(Owner))
        {
           // Weapon->EjectMagazine();
        }
    }
}
