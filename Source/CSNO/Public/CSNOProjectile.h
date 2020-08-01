// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSNOProjectile.generated.h"

class UProjectileMovementComponent;
class URadialForceComponent;

UCLASS(config=Game)
class ACSNOProjectile : public AActor {
	GENERATED_BODY()

public:
	ACSNOProjectile();

	/** called when projectile hits something */
	UFUNCTION(BlueprintNativeEvent)
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	           const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class UStaticMeshComponent* GetMeshComp() const { return MeshComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void Explode();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayExplodeEffects();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Radius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Damage;

private:

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ImpulseVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ExplodeDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	bool bExplodeOnImpact;

	FTimerHandle TimerHandle_ExplodeDalay;

};
