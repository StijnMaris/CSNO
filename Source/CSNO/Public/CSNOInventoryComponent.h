// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CSNOInventoryComponent.generated.h"

class ACSNOWeaponBase;
class ACSNOCharacter;
class UDataTable;

USTRUCT(BlueprintType)
struct FInventoryItem {
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FName ItemName;

    UPROPERTY(BlueprintReadWrite)
    int TotalAmmo;

    UPROPERTY(BlueprintReadWrite)
    int ClipAmmo;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CSNO_API UCSNOInventoryComponent : public UActorComponent {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCSNOInventoryComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    UFUNCTION()
    bool ChangeInventorySlot(int SlotNr);

    UFUNCTION()
    void ChangeToPreviousInventorySlot();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerChangeInventorySlot(int SlotNr);

    UFUNCTION(BlueprintCallable)
    FORCEINLINE FName GetWeaponAttachSocketName() const { return WeaponAttachSocketName; }

    UFUNCTION(BlueprintCallable)
    FORCEINLINE ACSNOWeaponBase* GetCurrentFPWeapon() const { return CurrentFPWeapon; }

    UFUNCTION(BlueprintCallable)
    void AddItemToInventorySlot(int SlotNr, FInventoryItem Item);

    UFUNCTION(BlueprintCallable)
    FInventoryItem RemoveItemFromInventorySlot(int SlotNr);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRemoveItemFromInventorySlot(int SlotNr);

    UFUNCTION(BlueprintCallable)
    FInventoryItem RemoveItemFromInventory(FName WeaponName);

    UFUNCTION(BlueprintCallable)
    bool HasItemInInventory(FName WeaponName);

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    UFUNCTION()
    void SpawnWeapon(TSubclassOf<ACSNOWeaponBase> ClassToSpawn, FInventoryItem& InventoryItem);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    UDataTable* WeaponInfoDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventoryItem> InventoryArray;

private:
    //func
    ACSNOCharacter* GetCharacter();

    //var
    UPROPERTY(Replicated, VisibleDefaultsOnly, Category = "Inventory")
    ACSNOWeaponBase* CurrentFPWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int CurrentItemSlot;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int PreviousItemSlot;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<ACSNOWeaponBase> StartWeaponClass;

    UPROPERTY(VisibleDefaultsOnly, Category = "Inventory" )
    FName WeaponAttachSocketName;

    ACSNOCharacter* Player;
};
