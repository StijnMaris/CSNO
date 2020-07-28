// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOProjectileWeapon.h"
#include "CSNOCharacter.h"
#include "Kismet/GameplayStatics.h"

void ACSNOProjectileWeapon::Fire() {
	// try and fire a projectile
	if (!HasAuthority()) {
		ServerFire();
		return;
	}

	if (ProjectileClass) {
		UWorld* const World = GetWorld();
		if (World) {
			ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
			if (Player) {
				FVector EyeLocation;
				FRotator EyeRotation;
				Player->GetActorEyesViewPoint(EyeLocation, EyeRotation);
				EyeLocation += EyeRotation.Vector() * 3000;

				const FRotator ShootRotation = (EyeLocation - GetActorLocation()).Rotation();
				const FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride =
					ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<ACSNOProjectile>(ProjectileClass, MuzzleLocation, ShootRotation, ActorSpawnParams);

				FMath::Clamp(CurrentClipAmmo--, 0, MaxClipAmmo);

				if (CurrentClipAmmo == 0) {
					StopFire();
				}

				LastFireTime = GetWorld()->TimeSeconds;
			}
		}
	}

	// try and play the sound if specified
	if (ShotSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ShotSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	/*if (FireAnimation) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL) {
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}*/
}
