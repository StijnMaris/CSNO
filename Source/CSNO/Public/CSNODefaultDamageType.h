// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "CSNODefaultDamageType.generated.h"

/**
 * 
 */
UCLASS()
class CSNO_API UCSNODefaultDamageType : public UDamageType {
    GENERATED_UCLASS_BODY()
public:
    //func
    UFUNCTION()
    float ProcessDamage(float Damage, AActor* DamagedActor, AActor* DamageCauser);

protected:
    //var
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float Multiplier;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    bool bCanBeArmoured;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    bool bWasHeadshot;
};
