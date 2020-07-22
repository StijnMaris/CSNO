// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOLineTraceWeapon.h"

#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "CSNO/CSNO.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "CSNO/Public/CSNOCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("CSNO.DebugWeapons"), DebugWeaponDrawing,
                                               TEXT("Draw Debug Lines For Weapons"), ECVF_Cheat);

void ACSNOLineTraceWeapon::StartFire() {
    if (CurrentClipAmmo > 0) {
        StartFireTime = GetWorld()->TimeSeconds;

        float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - StartFireTime, 0.f);

        GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSNOLineTraceWeapon::Fire,
                                        TimeBetweenShots, bHasAutomaticFire, FirstDelay);
    }
}

void ACSNOLineTraceWeapon::StopFire() {
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ACSNOLineTraceWeapon::BeginPlay() {
    Super::BeginPlay();

    TimeBetweenShots = 60 / RateOfFire;
}

void ACSNOLineTraceWeapon::Fire() {
    if (!HasAuthority()) {
        ServerFire();
    }

    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    if (Player) {
        FVector EyeLocation;
        FRotator EyeRotation;
        Player->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        //EyeLocation = Cast<ACSNOCharacter>(MyOwner)->GetPawnViewLocation();

        FVector ShotDirection = EyeRotation.Vector();
        FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(Player);
        QueryParams.bTraceComplex = true;
        QueryParams.bReturnPhysicalMaterial = true;

        FVector TracerEndPoint = TraceEnd;

        EPhysicalSurface SurfaceType = SurfaceType_Default;

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TracerEndPoint, COLLISION_WEAPON, QueryParams)) {
            AActor* HitActor = Hit.GetActor();

            SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

            UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
                                               Player->GetInstigatorController(), this, DamageType);

            PlayImpactEffect(SurfaceType, Hit.ImpactPoint);

            TracerEndPoint = Hit.ImpactPoint;
        }
        if (DebugWeaponDrawing > 0) {
            DrawDebugLine(GetWorld(), EyeLocation, TracerEndPoint, FColor::White, false, 1.f, 0, 1.f);
        }
        AddRecoil(Player);

        PlayFireEffect(MeshComp, TracerEndPoint);

        FMath::Clamp(CurrentClipAmmo--, 0, MaxClipAmmo);

        if (CurrentClipAmmo == 0) {
            StopFire();
        }

        if (HasAuthority()) {
            HitScanTrace.TraceEnd = TracerEndPoint;
            HitScanTrace.SurfaceType = SurfaceType;
        }

        LastFireTime = GetWorld()->TimeSeconds;
    }
}

void ACSNOLineTraceWeapon::PlayFireEffect(USkeletalMeshComponent* SkelMeshComp, FVector TraceEndPoint) {
    const FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocketName);
    const FRotator MuzzleRotation = SkelMeshComp->GetSocketRotation(MuzzleSocketName);

    if (MuzzleEffect && ShotSound) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleEffect, MuzzleLocation, MuzzleRotation);

        UGameplayStatics::PlaySoundAtLocation(this, ShotSound, MuzzleLocation);
    }

    if (TracerEffect) {
        UNiagaraComponent* TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), TracerEffect, MuzzleLocation);

        if (TracerComp) {
            TracerComp->SetVectorParameter(TracerTargetName, TraceEndPoint);
        }
    }

    // APawn* MyOwner = Cast<APawn>(GetOwner());
    // if (MyOwner) {
    //     APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
    //     if (PC) {
    //         PC->ClientPlayCameraShake(FireCamShake);
    //     }
    // }
}

void ACSNOLineTraceWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
    if (CurrentImpactEffect) {
        FVector MuzzleLocation(FVector::ZeroVector);
        if (HasAuthority()) {
            MuzzleLocation = TPMeshComp->GetSocketLocation(MuzzleSocketName);
        }
        else {
            MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
        }

        FVector ShotDirection = ImpactPoint - MuzzleLocation;
        ShotDirection.Normalize();

        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentImpactEffect, ImpactPoint,
                                                       ShotDirection.Rotation());
    }
}

void ACSNOLineTraceWeapon::SetVarBasedOnSurfaceType(EPhysicalSurface SurfaceType) {
    switch (SurfaceType) {
    case SURFACE_FLESH_DEFAULT: {
        CurrentImpactEffect = FleshImpactEffect;
        ActualDamage *= 1.f;
        break;
    }

    case SURFACE_FLESH_HEAD: {
        CurrentImpactEffect = FleshImpactEffect;
        ActualDamage *= 4.f;
        break;
    }

    case SURFACE_FLESH_ARMOUR: {
        CurrentImpactEffect = FleshImpactEffect;
        ActualDamage *= 1.f;
        break;
    }
    default:
        CurrentImpactEffect = DefaultImpactEffect;
        ActualDamage *= 1.f;
        break;
    }
}

void ACSNOLineTraceWeapon::OnRep_HitScanTrace() {
    PlayFireEffect(TPMeshComp, HitScanTrace.TraceEnd);

    PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceEnd);
}

void ACSNOLineTraceWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ACSNOLineTraceWeapon, HitScanTrace, COND_SkipOwner)
}
