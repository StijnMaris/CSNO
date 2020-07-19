// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOWeaponBase.h"

#include "DrawDebugHelpers.h"
#include "CSNO/CSNO.h"
#include "CSNO/Public/CSNOCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraFunctionLibrary.h"
#include "../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h"
//#include "CSNO/Public/CSNOArmourDamageType.h"
#include "CSNO/Public/CSNODefaultDamageType.h"
//#include "CSNO/Public/CSNOHeadDamageType.h"
#include "Curves/CurveVector.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("CSNO.DebugWeapons"), DebugWeaponDrawing,
                                               TEXT("Draw Debug Lines For Weapons"), ECVF_Cheat);

ACSNOWeaponBase::ACSNOWeaponBase() {
    PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetOnlyOwnerSee(true);
    RootComponent = MeshComp;

    TPMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPMeshComp"));
    TPMeshComp->SetOwnerNoSee(true);

    DamageType = UCSNODefaultDamageType::StaticClass();

    MuzzleSocketName = "MuzzleSocket";
    TracerTargetName = "BeamTarget";

    BaseDamage = 20.f;
    RateOfFire = 600;
    MaxTotalAmmo = 0;
    MaxClipAmmo = 30;
    ArmourPiercing = 0.5;

    SetReplicates(true);
}

// Sets default values

void ACSNOWeaponBase::Init(FWeaponInfo& WeaponInfo, FInventoryItem& InventoryItem) {
    CurrentInventoryItem = InventoryItem;
    CurrentWeaponInfo = WeaponInfo;

    ChangeWeapon(CurrentWeaponInfo, InventoryItem);

    // NetUpdateFrequency = 66.f;
    // MinNetUpdateFrequency = 33.f;
}

// Called every frame
void ACSNOWeaponBase::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    TimeBetweenShots = 60 / RateOfFire;
}

void ACSNOWeaponBase::StartFire() {
    if (CurrentClipAmmo > 0) {
        StartFireTime = GetWorld()->TimeSeconds;

        float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - StartFireTime, 0.f);

        GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSNOWeaponBase::Fire, TimeBetweenShots,
                                        true, FirstDelay);
    }
}

void ACSNOWeaponBase::StopFire() {
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ACSNOWeaponBase::Reload() {
    if (!HasAuthority()) {
        ServerReload();
    }
    if (CurrentClipAmmo < MaxClipAmmo) {
        if (MaxClipAmmo - CurrentClipAmmo <= CurrentTotalAmmo) {
            CurrentTotalAmmo -= MaxClipAmmo - CurrentClipAmmo;
            CurrentClipAmmo = MaxClipAmmo;
        }
        else if (CurrentTotalAmmo != 0) {
            CurrentClipAmmo += CurrentTotalAmmo;
            CurrentTotalAmmo = 0;
        }

    }
}

void ACSNOWeaponBase::ServerReload_Implementation() {
    Reload();
}

bool ACSNOWeaponBase::ServerReload_Validate() {
    return true;
}

float ACSNOWeaponBase::GetArmourPiercing() {
    return ArmourPiercing;
}

void ACSNOWeaponBase::ChangeWeapon(FWeaponInfo& WeaponInfo, FInventoryItem& InventoryItem) {
    CurrentWeaponName = InventoryItem.ItemName;
    MaxTotalAmmo = WeaponInfo.MaxTotalAmmo;
    CurrentTotalAmmo = MaxTotalAmmo;
    MaxClipAmmo = WeaponInfo.MaxClipAmmo;
    CurrentClipAmmo = MaxClipAmmo;
    RateOfFire = WeaponInfo.RateOfFire;
    BaseDamage = WeaponInfo.BaseDamage;
    ActualDamage = BaseDamage;
    ArmourPiercing = WeaponInfo.ArmourPiercing;
    MeshComp->SetSkeletalMesh(WeaponInfo.GunMesh);
    TPMeshComp->SetSkeletalMesh(WeaponInfo.GunMesh);
    ShotSound = WeaponInfo.ShotSound;
    RecoilCurve = WeaponInfo.RecoilCurve;

    if (InventoryItem.ClipAmmo >= 0 && InventoryItem.TotalAmmo >= 0) {
        CurrentClipAmmo = InventoryItem.ClipAmmo;
        CurrentTotalAmmo = InventoryItem.TotalAmmo;
    }
}

// Called when the game starts or when spawned 
void ACSNOWeaponBase::BeginPlay() {
    Super::BeginPlay();

    ActualDamage = BaseDamage;

    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    if (Player) {
        TPMeshComp->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                      Player->GetWeaponAttachSocketName());
    }
}

void ACSNOWeaponBase::Fire() {
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

void ACSNOWeaponBase::ServerFire_Implementation() {
    Fire();
}

bool ACSNOWeaponBase::ServerFire_Validate() {
    return true;
}

void ACSNOWeaponBase::PlayFireEffect(USkeletalMeshComponent* SkelMeshComp, FVector TraceEndPoint) {
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

void ACSNOWeaponBase::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
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

void ACSNOWeaponBase::SetVarBasedOnSurfaceType(EPhysicalSurface SurfaceType) {
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

void ACSNOWeaponBase::AddRecoil(ACSNOCharacter* Player) {
    float TimeValue = LastFireTime - StartFireTime;
    UE_LOG(LogTemp, Log, TEXT("Time Value: %s"), *FString::SanitizeFloat(TimeValue));

    Player->AddControllerYawInput(RecoilCurve->GetVectorValue(TimeValue).X);

    Player->AddControllerPitchInput(RecoilCurve->GetVectorValue(TimeValue).Y);
}

void ACSNOWeaponBase::OnRep_HitScanTrace() {
    PlayFireEffect(TPMeshComp, HitScanTrace.TraceEnd);

    PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceEnd);
}

void ACSNOWeaponBase::OnRep_WeaponChange() {
    ChangeWeapon(CurrentWeaponInfo, CurrentInventoryItem);
}

void ACSNOWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ACSNOWeaponBase, HitScanTrace, COND_SkipOwner)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentInventoryItem)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentWeaponInfo)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentClipAmmo)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentTotalAmmo)
}
