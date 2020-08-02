// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOWeaponBase.h"

#include "DrawDebugHelpers.h"
#include "CSNO/Public/CSNOCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
//#include "../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraFunctionLibrary.h"
#include "../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h"
//#include "CSNO/Public/CSNOArmourDamageType.h"
#include "CSNO/Public/CSNODefaultDamageType.h"
//#include "CSNO/Public/CSNOHeadDamageType.h"
#include "Curves/CurveVector.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeapon, Log, All);

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

void ACSNOWeaponBase::StartFire() {
	ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
	if (CurrentClipAmmo > 0 && Player) {
		StartFireTime = GetWorld()->TimeSeconds;

		Player->SetPlayerCondition(EPlayerCondition::Shooting);

		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - StartFireTime, 0.f);
		
		UE_LOG(LogWeapon, Log, TEXT("%s %d: FirstDelay: %f"), *UEnum::GetValueAsString(GetLocalRole()),
           GPlayInEditorID,FirstDelay);

		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSNOWeaponBase::Fire,
		                                TimeBetweenShots, bHasAutomaticFire, FirstDelay);
	}
}

void ACSNOWeaponBase::StopFire() {
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
	if (Player) {
		Player->SetPlayerCondition(EPlayerCondition::Idle);
	}

}

void ACSNOWeaponBase::Reload() {
	if (!HasAuthority()) {
		ServerReload();
	}
	if (CurrentClipAmmo < MaxClipAmmo) {
		ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
		if (Player) {
			Player->SetPlayerCondition(EPlayerCondition::Reloading);
			//if (!HasAuthority()) {
			PlayAnimations(CharacterReloadMontageFP, CharacterReloadMontage, ReloadMontage);
			/*}
			else {
				MulticastAnimations(CharacterReloadMontage, ReloadMontage);
			}*/
		}
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
	MeshComp->SetAnimInstanceClass(WeaponInfo.AnimBlueprint);
	TPMeshComp->SetSkeletalMesh(WeaponInfo.GunMesh);
	TPMeshComp->SetAnimInstanceClass(WeaponInfo.AnimBlueprint);
	ShotSound = WeaponInfo.ShotSound;
	RecoilCurve = WeaponInfo.RecoilCurve;
	MuzzleEffect = WeaponInfo.MuzzleEffect;
	TracerEffect = WeaponInfo.TracerEffect;
	ShellEjectionEffect = WeaponInfo.ShellEjectionEffect;
	ReloadMontage = WeaponInfo.ReloadMontage;
	CharacterReloadMontage = WeaponInfo.CharacterReloadMontage;
	CharacterReloadMontageFP = WeaponInfo.CharacterReloadMontageFP;
	FireMontage = WeaponInfo.FireMontage;
	CharacterFireMontage = WeaponInfo.CharacterFireMontage;
	CharacterFireMontageFP = WeaponInfo.CharacterFireMontageFP;
	CharacterSwitchWeaponMontage = WeaponInfo.CharacterSwitchWeaponMontage;
	CharacterSwitchWeaponMontageFP = WeaponInfo.CharacterSwitchWeaponMontageFP;
	ProjectileClass = WeaponInfo.ProjectileClass;

	PlayAnimations(CharacterSwitchWeaponMontageFP, CharacterSwitchWeaponMontage);

	if (InventoryItem.ClipAmmo >= 0 && InventoryItem.TotalAmmo >= 0) {
		CurrentClipAmmo = InventoryItem.ClipAmmo;
		CurrentTotalAmmo = InventoryItem.TotalAmmo;
	}
}

// Called when the game starts or when spawned 
void ACSNOWeaponBase::BeginPlay() {
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

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

void ACSNOWeaponBase::Fire() {
	PlayAnimations(CharacterFireMontageFP, CharacterFireMontage, FireMontage);
}

void ACSNOWeaponBase::MulticastAnimations_Implementation(UAnimMontage* CharacterFPAnim, UAnimMontage* CharacterAnim,
                                                         UAnimMontage* WeaponAnim) {
	ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
	if (Player) {
		if (GetLocalRole() == ROLE_SimulatedProxy && !Player->IsLocallyControlled()) {
			PlayAnimations(CharacterFPAnim, CharacterAnim, WeaponAnim);
		}
	}
}

void ACSNOWeaponBase::AddRecoil(ACSNOCharacter* Player) {
	float TimeValue = LastFireTime - StartFireTime;
	//UE_LOG(LogTemp, Log, TEXT("Time Value: %s"), *FString::SanitizeFloat(TimeValue));

	Player->AddControllerYawInput(RecoilCurve->GetVectorValue(TimeValue).X);

	Player->AddControllerPitchInput(RecoilCurve->GetVectorValue(TimeValue).Y);
}

void ACSNOWeaponBase::PlayAnimations(UAnimMontage* CharacterFPAnim, UAnimMontage* CharacterAnim,
                                     UAnimMontage* WeaponAnim) {
	if (HasAuthority()) {
		MulticastAnimations(CharacterFPAnim, CharacterAnim, WeaponAnim);
	}

	UE_LOG(LogWeapon, Log, TEXT("%s %d: PlayAnimation"), *UEnum::GetValueAsString(GetLocalRole()), GPlayInEditorID);

	ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
	if (Player) {
		if (CharacterAnim && CharacterFPAnim) {
			UAnimInstance* AnimInstFP = Player->GetMeshFP()->GetAnimInstance();
			UAnimInstance* AnimInstTP = Player->GetMesh()->GetAnimInstance();

			if (AnimInstFP && AnimInstTP) {
				//if (!AnimInstFP->Montage_IsPlaying(nullptr) && !AnimInstTP->Montage_IsPlaying(nullptr)) {
				AnimInstFP->Montage_Play(CharacterFPAnim);
				AnimInstTP->Montage_Play(CharacterAnim);
				//}

				if (WeaponAnim && !HasAuthority()) {
					UAnimInstance* AnimInstWeaponTP = TPMeshComp->GetAnimInstance();
					UAnimInstance* AnimInstWeaponFP = MeshComp->GetAnimInstance();

					if (AnimInstWeaponFP && AnimInstWeaponTP) {
						AnimInstWeaponFP->Montage_Play(WeaponAnim);
						AnimInstWeaponTP->Montage_Play(WeaponAnim);
					}
				}
			}
		}
	}
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
