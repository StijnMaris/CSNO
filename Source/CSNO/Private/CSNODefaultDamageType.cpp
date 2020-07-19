// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNO/Public/CSNODefaultDamageType.h"

#include "CSNOArmourComponent.h"
#include "CSNOWeaponBase.h"

UCSNODefaultDamageType::UCSNODefaultDamageType(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Multiplier = 1;
    bCanBeArmoured = false;
    bWasHeadshot = false;
}

float UCSNODefaultDamageType::ProcessDamage(float Damage, AActor* DamagedActor,
                                            AActor* DamageCauser) {
    float ActualDamage = Damage;
    ActualDamage *= Multiplier;

    if (bCanBeArmoured) {

        UCSNOArmourComponent* ArmourComponent = Cast<UCSNOArmourComponent>(
            DamagedActor->GetComponentByClass(UCSNOArmourComponent::StaticClass()));

        ACSNOWeaponBase* Weapon = Cast<ACSNOWeaponBase>(DamageCauser);

        if (ArmourComponent && Weapon) {
            if (ArmourComponent->GetHasArmour()) {
                if (bWasHeadshot && ArmourComponent->GetHasHeadArmour()) {
                    ArmourComponent->SetHasHeadArmour(false);
                }
                ActualDamage = Damage * Weapon->GetArmourPiercing();
                ArmourComponent->Damage(Damage - ActualDamage);
            }
        }
    }

    return ActualDamage;
}
