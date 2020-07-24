// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSNO/Public/CSNOCharacter.h"
#include "CSNO/Public/CSNOWeaponBase.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "../CSNO.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "CSNOHealthComponent.h"
#include "CSNOArmourComponent.h"
#include "CSNOInventoryComponent.h"
#include "CSNODefusableBomb.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACSNOCharacter

ACSNOCharacter::ACSNOCharacter() {
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(45.f, 90.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-20.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	MeshFP->SetOnlyOwnerSee(true);
	MeshFP->SetupAttachment(FirstPersonCameraComponent);
	MeshFP->bCastDynamicShadow = false;
	MeshFP->CastShadow = false;
	MeshFP->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshFP->SetRelativeLocation(FVector(0.f, 0.f, -160.f));

	HealthComponent = CreateDefaultSubobject<UCSNOHealthComponent>(TEXT("HealthComp"));

	ArmourComponent = CreateDefaultSubobject<UCSNOArmourComponent>(TEXT("ArmourComp"));

	InventoryComponent = CreateDefaultSubobject<UCSNOInventoryComponent>(TEXT("InventoryComp"));

	GetMesh()->SetOwnerNoSee(true);
}

void ACSNOCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled()) {
		FRotator NewRot = FirstPersonCameraComponent->GetRelativeRotation();
		NewRot.Pitch = RemoteViewPitch * 360.f / 255.f;

		FirstPersonCameraComponent->SetRelativeRotation(NewRot);
	}
}

FRotator ACSNOCharacter::GetAimOffsets() const {
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

void ACSNOCharacter::SetWalking(bool bNewWalking) {
	if (!HasAuthority()) {
		ServerSetWalking(bNewWalking);
	}

	bWantsToWalk = bNewWalking;
	if (bWantsToWalk) {
		GetCharacterMovement()->MaxWalkSpeed *= 0.5;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed *= 2;
	}

}

void ACSNOCharacter::SetPlayerCondition(EPlayerCondition NewCondition) {
	if (!HasAuthority()) { ServerSetPlayerCondition(NewCondition); }
	PlayerCondition = NewCondition;
}

FName ACSNOCharacter::GetWeaponAttachSocketName() const {
	return InventoryComponent->GetWeaponAttachSocketName();
}

void ACSNOCharacter::BeginPlay() {
	// Call the base class  
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &ACSNOCharacter::OnHealthChanged);
	HealthComponent->OnDied.AddUniqueDynamic(this, &ACSNOCharacter::OnDied);
	//OnPlantedBomb.AddUniqueDynamic(this, &ACSNOCharacter::BombPlanted);
}

void ACSNOCharacter::MoveForward(float Value) {
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACSNOCharacter::MoveRight(float Value) {
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

#pragma optimize("",off)
void ACSNOCharacter::StartFire() {
	ACSNOWeaponBase* CurrentWeapon = InventoryComponent->GetCurrentFPWeapon();
	if (CurrentWeapon && PlayerCondition == EPlayerCondition::Idle) {
		CurrentWeapon->StartFire();
	}
}
#pragma optimize("",on)

void ACSNOCharacter::StopFire() {
	ACSNOWeaponBase* CurrentWeapon = InventoryComponent->GetCurrentFPWeapon();
	if (CurrentWeapon && PlayerCondition == EPlayerCondition::Shooting) {
		CurrentWeapon->StopFire();
	}
}

void ACSNOCharacter::Reload() {
	ACSNOWeaponBase* CurrentWeapon = InventoryComponent->GetCurrentFPWeapon();
	if (CurrentWeapon && PlayerCondition == EPlayerCondition::Idle) {
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopReload, 2.16);
		CurrentWeapon->Reload();
	}
}

void ACSNOCharacter::StopReload() {
	SetPlayerCondition(EPlayerCondition::Idle);
}

void ACSNOCharacter::BeginCrouch() {
	Crouch();
}

void ACSNOCharacter::EndCrouch() {
	UnCrouch();
}

void ACSNOCharacter::BeginWalk() {
	SetWalking(true);
}

void ACSNOCharacter::EndWalk() {
	SetWalking(false);
}

void ACSNOCharacter::AlternateFire() {}

void ACSNOCharacter::Action() {
	if (DefusableBomb) {
		DefusableBomb->StartDefuse();
	}
}

void ACSNOCharacter::StopAction() {
	if (DefusableBomb) {
		DefusableBomb->StopDefuse();
	}
}

void ACSNOCharacter::ChangeToInventorySlot1() {
	if (InventoryComponent && PlayerCondition == EPlayerCondition::Idle) {
		SetPlayerCondition(EPlayerCondition::Switching_Weapon);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopChangingWeapon, 1.73);
		InventoryComponent->ChangeInventorySlot(0);
	}
}

void ACSNOCharacter::ChangeToInventorySlot2() {
	if (InventoryComponent && PlayerCondition == EPlayerCondition::Idle) {
		SetPlayerCondition(EPlayerCondition::Switching_Weapon);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopChangingWeapon, 1.73);
		InventoryComponent->ChangeInventorySlot(1);
	}
}

void ACSNOCharacter::ChangeToInventorySlot3() {
	if (InventoryComponent && PlayerCondition == EPlayerCondition::Idle) {
		SetPlayerCondition(EPlayerCondition::Switching_Weapon);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopChangingWeapon, 1.73);
		InventoryComponent->ChangeInventorySlot(2);
	}
}

void ACSNOCharacter::ChangeToInventorySlot4() {
	if (InventoryComponent && PlayerCondition == EPlayerCondition::Idle) {
		SetPlayerCondition(EPlayerCondition::Switching_Weapon);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopChangingWeapon, 1.73);
		InventoryComponent->ChangeInventorySlot(3);
	}
}

void ACSNOCharacter::ChangeToInventorySlot5() {
	if (InventoryComponent && PlayerCondition == EPlayerCondition::Idle) {
		SetPlayerCondition(EPlayerCondition::Switching_Weapon);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ACSNOCharacter::StopChangingWeapon, 1.73);
		InventoryComponent->ChangeInventorySlot(4);
	}
}

void ACSNOCharacter::StopChangingWeapon() {
	SetPlayerCondition(EPlayerCondition::Idle);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACSNOCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSNOCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACSNOCharacter::StopFire);

	PlayerInputComponent->BindAction("AlternateFire", IE_Pressed, this, &ACSNOCharacter::AlternateFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACSNOCharacter::Reload);

	//Action
	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ACSNOCharacter::Action);
	PlayerInputComponent->BindAction("Action", IE_Released, this, &ACSNOCharacter::StopAction);

	//crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACSNOCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACSNOCharacter::EndCrouch);

	//Walk
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ACSNOCharacter::BeginWalk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ACSNOCharacter::EndWalk);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACSNOCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSNOCharacter::MoveRight);

	// Rotation bindings 
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	//Inventory
	PlayerInputComponent->BindAction("InventorySlot1", IE_Pressed, this, &ACSNOCharacter::ChangeToInventorySlot1);
	PlayerInputComponent->BindAction("InventorySlot2", IE_Pressed, this, &ACSNOCharacter::ChangeToInventorySlot2);
	PlayerInputComponent->BindAction("InventorySlot3", IE_Pressed, this, &ACSNOCharacter::ChangeToInventorySlot3);
	PlayerInputComponent->BindAction("InventorySlot4", IE_Pressed, this, &ACSNOCharacter::ChangeToInventorySlot4);
	PlayerInputComponent->BindAction("InventorySlot5", IE_Pressed, this, &ACSNOCharacter::ChangeToInventorySlot5);
}

void ACSNOCharacter::OnHealthChanged(UCSNOHealthComponent* HealthComp, float Health, float HealthDelta,
                                     const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) {
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (%s)"), *FString::SanitizeFloat(Health),
	       *FString::SanitizeFloat(HealthDelta));
}

void ACSNOCharacter::OnDied(bool bIsDead, AActor* VictimActor, AActor* KillerActor, APlayerState* KillerPlayerState) {
	GetMovementComponent()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true);
	DetachFromControllerPendingDestroy();

	SetLifeSpan(2.f);
}

void ACSNOCharacter::ServerSetPlayerCondition_Implementation(EPlayerCondition NewCondition) {
	SetPlayerCondition(NewCondition);
}

bool ACSNOCharacter::ServerSetPlayerCondition_Validate(EPlayerCondition NewCondition) { return true; }

void ACSNOCharacter::ServerSetWalking_Implementation(bool bNewWalking) {
	SetWalking(bNewWalking);
}

bool ACSNOCharacter::ServerSetWalking_Validate(bool bNewWalking) {
	return true;
}

void ACSNOCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACSNOCharacter, bWantsToWalk, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ACSNOCharacter, PlayerCondition, COND_SkipOwner);

}

/*
void ACSNOCharacter::OnFire() {
    // try and fire a projectile
    if (ProjectileClass != NULL) {
        UWorld* const World = GetWorld();
        if (World != NULL) {

            const FRotator SpawnRotation = GetControlRotation();
            // MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
            const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr)
                                               ? FP_MuzzleLocation->GetComponentLocation()
                                               : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

            //Set Spawn Collision Handling Override
            FActorSpawnParameters ActorSpawnParams;
            ActorSpawnParams.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

            // spawn the projectile at the muzzle
            World->SpawnActor<ACSNOProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
        }

    }

    // try and play the sound if specified
    if (FireSound != NULL) {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    // try and play a firing animation if specified
    if (FireAnimation != NULL) {
        // Get the animation object for the arms mesh
        UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
        if (AnimInstance != NULL) {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}*/
