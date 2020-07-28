// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSNOProjectile.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

ACSNOProjectile::ACSNOProjectile() {

	ImpulseVelocity = 100.f;
	Radius = 400.f;
	Damage = 100.f;
	ExplodeDelay = 2.f;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName("Projectile");
	// Players can't walk on it
	MeshComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	MeshComp->CanCharacterStepUpOn = ECB_No;
	// Set as root component
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->ImpulseStrength = Damage * 10.f;
	RadialForceComp->Radius = Radius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->Falloff = RIF_Linear;
	RadialForceComp->SetupAttachment(MeshComp);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = MeshComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 4000.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void ACSNOProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            FVector NormalImpulse, const FHitResult& Hit) {
	// Only add impulse and destroy projectile if we hit a physics
	if (bExplodeOnImpact) {
		Explode();
	}

}

void ACSNOProjectile::BeginPlay() {
	Super::BeginPlay();
	MeshComp->OnComponentHit.AddUniqueDynamic(this, &ACSNOProjectile::OnHit);

	if (ExplodeDelay > 0.f) {
		GetWorldTimerManager().SetTimer(TimerHandle_ExplodeDalay, this, &ACSNOProjectile::Explode, ExplodeDelay);
	}
}

void ACSNOProjectile::Explode() {
	if (HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("%s %d: Projectile Exploded"), *UEnum::GetValueAsString(GetLocalRole()),
		       GPlayInEditorID);

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		bool ApliedDamage = UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage,
		                                                        MeshComp->GetComponentLocation(),
		                                                        Radius, nullptr, IgnoredActors,
		                                                        this, GetInstigatorController(),false);
		PlayExplodeEffects();

		if (ApliedDamage) {
			DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, FColor::Red,
			                false, 5.f, 0, 2.f);
		}

		SetLifeSpan(0.1);
		//Destroy();
	}
}

void ACSNOProjectile::PlayExplodeEffects_Implementation() {
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	RadialForceComp->FireImpulse();

}
