// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSNOWeaponBase.h"
#include "CSNOLineTraceWeapon.generated.h"

USTRUCT()
struct FHitScanTrace {
    GENERATED_BODY()

public:
    UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;

    UPROPERTY()
    FVector_NetQuantize TraceEnd;
};

/**
 * 
 */
UCLASS()
class CSNO_API ACSNOLineTraceWeapon : public ACSNOWeaponBase {
    GENERATED_BODY()

public:
    virtual void StartFire() override;

    virtual void StopFire()override;

protected:
    virtual void BeginPlay() override;
    
    virtual void Fire() override;

    void PlayFireEffect(USkeletalMeshComponent* SkelMeshComp, FVector TraceEndPoint);

    void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

    void SetVarBasedOnSurfaceType(EPhysicalSurface SurfaceType);

    UFUNCTION()
    void OnRep_HitScanTrace();

private:
    FTimerHandle TimerHandle_TimeBetweenShots;

    float TimeBetweenShots;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UNiagaraSystem* CurrentImpactEffect;

    UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

};
