// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CSNOHealthComponent.h"
#include "CSNOInventoryComponent.h"
#include "GameFramework/Character.h"
#include "CSNOCharacter.generated.h"

class UInputComponent;
class UCSNOHealthComponent;
class UCSNOArmourComponent;
class UCSNOInventoryComponent;
class ACSNOWeaponBase;
class UAnimMontage;
class UCameraComponent;
class ACSNODefusableBomb;

UENUM(BlueprintType)
enum class EPlayerCondition : uint8 {
	Idle,
	Shooting,
	Reloading,
	Switching_Weapon,
	Preforming_Action
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlantedBomb, AActor*, Planter, AController*, PlanterController);

UCLASS(config=Game)
class ACSNOCharacter : public ACharacter {
	GENERATED_BODY()

public:
	ACSNOCharacter();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	void SetWalking(bool bNewWalking);

	void SetPlayerCondition(EPlayerCondition NewCondition);

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMeshFP() const { return MeshFP; }

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	FORCEINLINE class UCSNOHealthComponent* GetHealthComponent() const { return HealthComponent; }

	FORCEINLINE class UCSNOArmourComponent* GetArmourComponent() const { return ArmourComponent; }

	FORCEINLINE class UCSNOInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	FName GetWeaponAttachSocketName() const;

	UFUNCTION()
	void SetIsInBombPlantingSite(bool bIsInPlantingSite) {
		bIsInBombPlantingSite = bIsInPlantingSite;
	}

	UFUNCTION()
	FORCEINLINE bool IsInBombPlantingSite() const {
		return bIsInBombPlantingSite;
	}

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlantedBomb OnPlantedBomb;

	UPROPERTY()
	ACSNODefusableBomb* DefusableBomb;

	UPROPERTY(Replicated, BlueprintReadOnly)
	EPlayerCondition PlayerCondition;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	bool bWantsToWalk;

protected:
	virtual void BeginPlay() override;

	//input

	void StartFire();
	void StopFire();

	void Reload();
	void StopReload();

	void BeginCrouch();
	void EndCrouch();

	void BeginWalk();
	void EndWalk();

	void AlternateFire();

	void Action();
	void StopAction();

	void ChangeToInventorySlot1();
	void ChangeToInventorySlot2();
	void ChangeToInventorySlot3();
	void ChangeToInventorySlot4();
	void ChangeToInventorySlot5();

	void StopChangingWeapon();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	UFUNCTION()
	void OnHealthChanged(UCSNOHealthComponent* HealthComp, float Health, float HealthDelta,
	                     const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnDied(bool bIsDead, AActor* VictimActor, AActor* KillerActor, APlayerState* KillerPlayerState);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetWalking(bool bNewWalking);

	UFUNCTION(reliable, server, WithValidation)
    void ServerSetPlayerCondition(EPlayerCondition NewCondition);

private:
	//var
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* MeshFP;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCSNOHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCSNOArmourComponent* ArmourComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCSNOInventoryComponent* InventoryComponent;

	FTimerHandle TimerHandle_ReloadTime;

	FTimerHandle TimerHandle_SwitchWeaponTime;

	UPROPERTY()
	bool bIsInBombPlantingSite;
};
