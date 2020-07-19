// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSNOArmourComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CSNO_API UCSNOArmourComponent : public UActorComponent {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCSNOArmourComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "ArmourComponent")
    float GetArmour() const;

    UFUNCTION(BlueprintCallable, Category = "ArmourComponent")
    bool GetHasArmour() const;

    UFUNCTION(BlueprintCallable, Category = "ArmourComponent")
    bool GetHasHeadArmour() const;

    UFUNCTION(BlueprintCallable, Category = "ArmourComponent")
    void Damage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "ArmourComponent")
    void SetHasHeadArmour(bool HasHeadArmour);

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    //var
    UPROPERTY(Replicated, VisibleAnywhere, Category = "ArmourComponent")
    float Armour;

    UPROPERTY(EditAnywhere, Category = "ArmourComponent")
    float DefaultArmour;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "ArmourComponent")
    bool bHasArmour;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "ArmourComponent")
    bool bHasHeadArmour;

    UPROPERTY(EditAnywhere, Category = "ArmourComponent")
    bool bStartsWithArmour;

};
