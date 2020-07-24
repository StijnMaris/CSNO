// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOWeaponBase.h"

#include "DrawDebugHelpers.h"
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
}

void ACSNOWeaponBase::Reload() {
    if (!HasAuthority()) {
        ServerReload();
    }
    if (CurrentClipAmmo < MaxClipAmmo) {
        ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
        if (Player) {
            Player->SetPlayerCondition(EPlayerCondition::Reloading);
        }
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

void ACSNOWeaponBase::ChangeWeapon(FWeaponInfo& WeaponInfo, FInventoryItem& InventoryItem) {
    CurrentWeaponName = InventoryItem.ItemName;
    MaxTotalAmmo = WeaponInfo.MaxTotalAmmo;
    CurrentTotalAmmo = MaxTotalAmmo;
    MaxClipAmmo = WeaponInfo.MaxClipAmmo;
    CurrentClipAmmo = MaxClipAmmo;
    RateOfFire = WeaponInfo.RateOfFire;
    bHasAutomaticFire = WeaponInfo.bHasAutomaticFire;
    BaseDamage = WeaponInfo.BaseDamage;
    ActualDamage = BaseDamage;
    ArmourPiercing = WeaponInfo.ArmourPiercing;
    MeshComp->SetSkeletalMesh(WeaponInfo.GunMesh);
    TPMeshComp->SetSkeletalMesh(WeaponInfo.GunMesh);
    ShotSound = WeaponInfo.ShotSound;
    RecoilCurve = WeaponInfo.RecoilCurve;
    MuzzleEffect = WeaponInfo.MuzzleEffect;
    TracerEffect =  WeaponInfo.TracerEffect;

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

void ACSNOWeaponBase::ServerFire_Implementation() {
    Fire();
}

bool ACSNOWeaponBase::ServerFire_Validate() {
    return true;
}

void ACSNOWeaponBase::AddRecoil(ACSNOCharacter* Player) {
    float TimeValue = LastFireTime - StartFireTime;
    UE_LOG(LogTemp, Log, TEXT("Time Value: %s"), *FString::SanitizeFloat(TimeValue));

    Player->AddControllerYawInput(RecoilCurve->GetVectorValue(TimeValue).X);

    Player->AddControllerPitchInput(RecoilCurve->GetVectorValue(TimeValue).Y);
}

void ACSNOWeaponBase::OnRep_WeaponChange() {
    ChangeWeapon(CurrentWeaponInfo, CurrentInventoryItem);
}

void ACSNOWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSNOWeaponBase, CurrentInventoryItem)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentWeaponInfo)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentClipAmmo)
    DOREPLIFETIME(ACSNOWeaponBase, CurrentTotalAmmo)
}
