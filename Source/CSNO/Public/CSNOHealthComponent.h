// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSNOHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UCSNOHealthComponent*, HealthComp, float,
                                             Health, float, HealthDelta, const class UDamageType*, DamageType,
                                             class AController*, InstigatedBy, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDiedSignature, bool, bIsDead, AActor*, VictimActor, AActor*,
                                              KillerActor, AController*, KillerController);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CSNO_API UCSNOHealthComponent : public UActorComponent {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCSNOHealthComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    bool GetIsDead() const;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDiedSignature OnDied;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void Heal(float HealAmount);

protected:
    // func
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
                             class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Health(float OldHealth);

    UFUNCTION()
    void OnRep_IsDead();

private:
    //var
    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "HealthComponent")
    float Health;

    UPROPERTY(EditAnywhere, Category = "HealthComponent")
    float MaxHealth;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "HealthComponent")
    bool bIsDead;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "HealthComponent")
    AActor* Killer;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, Category = "HealthComponent")
    AController* KillerController;
};
