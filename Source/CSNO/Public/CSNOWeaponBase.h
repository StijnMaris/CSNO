// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSNOInventoryComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "CSNOWeaponBase.generated.h"

struct FInventoryItem;
class UCSNOInventoryComponent;
class ACSNOWeaponBase;
class UNiagaraSystem;
class USoundBase;
class UCurveVector;

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSubclassOf<ACSNOWeaponBase> WeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bHasAutomaticFire;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int MaxClipAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int MaxTotalAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ArmourPiercing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMesh* GunMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USoundBase* ShotSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class UCurveVector* RecoilCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* TracerEffect;
};

UCLASS(Abstract)
class CSNO_API ACSNOWeaponBase : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACSNOWeaponBase();

	void Init(FWeaponInfo& WeaponInfo, FInventoryItem& InventoryItem);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire() { }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFire() { }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Reload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE float GetArmourPiercing() const { return ArmourPiercing; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ChangeWeapon(FWeaponInfo& WeaponInfo, FInventoryItem& InventoryItem);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE int GetCurrentClipAmmo() const { return CurrentClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE int GetCurrentTotalAmmo() const { return CurrentTotalAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE int GetMaxClipAmmo() const { return MaxClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE int GetMaxTotalAmmo() const { return MaxTotalAmmo; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* TPMeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* TracerEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* ShotSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UCurveVector* RecoilCurve;

	// UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	// TSubclassOf<UCameraShake> FireCamShake;

	virtual void Fire() { }

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	virtual void AddRecoil(class ACSNOCharacter* Player);

	UFUNCTION()
	void OnRep_WeaponChange();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int MaxClipAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int MaxTotalAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ArmourPiercing;

	float StartFireTime;

	UPROPERTY(Replicated)
	int CurrentClipAmmo;

	UPROPERTY(Replicated)
	int CurrentTotalAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float LastFireTime;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float ActualDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bHasAutomaticFire;

private:

	FName CurrentWeaponName;

	UPROPERTY(Replicated,VisibleAnywhere, Category = "Weapon")
	FInventoryItem CurrentInventoryItem;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponChange, EditDefaultsOnly, Category = "Weapon")
	FWeaponInfo CurrentWeaponInfo;

};
