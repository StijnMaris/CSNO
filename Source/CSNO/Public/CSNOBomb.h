// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSNOWeaponBase.h"
#include "CSNOBomb.generated.h"

/**
 * 
 */

class ACSNODefusableBomb;

UCLASS()
class CSNO_API ACSNOBomb : public ACSNOWeaponBase {
    GENERATED_BODY()
public:
    ACSNOBomb();

    virtual void StartFire() override;

    virtual void StopFire() override;

protected:
    virtual void Fire() override;

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerStopFire();

    virtual void AddRecoil(class ACSNOCharacter* Player) override;

    void BombPlanted();

    UFUNCTION()
    void SpawnDefusableBomb(AActor* Planter);
    
    UFUNCTION(Server,Reliable,WithValidation)
    void ServerSpawnDefusableBomb(AActor* Planter);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    USoundBase* BombPlantedSound;

private:
    FTimerHandle TimerHandle_BombPlant;

    UPROPERTY(EditDefaultsOnly)
    float BombPlantTime;

    UPROPERTY(EditDefaultsOnly, Category = "BombClass")
    TSubclassOf<ACSNODefusableBomb> DefusableBombClass;

    UPROPERTY(EditDefaultsOnly, Category = "BombLocation")
    float TraceOffset;
};
