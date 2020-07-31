// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSNOProjectile.h"
#include "CSNOWeaponBase.h"
#include "CSNOProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CSNO_API ACSNOProjectileWeapon : public ACSNOWeaponBase {
	GENERATED_BODY()

protected:
	virtual void Fire() override;

private:

};
