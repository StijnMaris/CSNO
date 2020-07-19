// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOInventoryComponent.h"

#include "CSNOCharacter.h"
#include "CSNOWeaponBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCSNOInventoryComponent::UCSNOInventoryComponent() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    WeaponAttachSocketName = "WeaponSocket";

    FInventoryItem Item{};
    InventoryArray.Init(Item, 6);

    Item = {"Weapon_M4", -1, -1};
    AddItemToInventorySlot(0, Item);
    Item = {"Bomb_C4", -1, -1};
    AddItemToInventorySlot(4, Item);
    Item = {"Defuse_Kit", -1, -1};
    AddItemToInventorySlot(5, Item);

    SetIsReplicatedByDefault(true);
}

void UCSNOInventoryComponent::AddItemToInventorySlot(int SlotNr, FInventoryItem Item) {
    if (InventoryArray.Num() > 0) {
        InventoryArray[SlotNr] = Item;
    }
}

FInventoryItem UCSNOInventoryComponent::RemoveItemFromInventorySlot(int SlotNr) {
    if (!GetOwner()->HasAuthority()) {
        ServerRemoveItemFromInventorySlot(SlotNr);
    }
    
    FInventoryItem OldItem = InventoryArray[SlotNr];
    InventoryArray[SlotNr] = FInventoryItem{};
    return OldItem;
}

void UCSNOInventoryComponent::ServerRemoveItemFromInventorySlot_Implementation(int SlotNr) {
    RemoveItemFromInventorySlot(SlotNr);
}

bool UCSNOInventoryComponent::ServerRemoveItemFromInventorySlot_Validate(int SlotNr) {
    return true;
}

FInventoryItem UCSNOInventoryComponent::RemoveItemFromInventory(FName WeaponName) {
    for (int i = 0; i < InventoryArray.Num(); ++i) {
        if (InventoryArray[i].ItemName == WeaponName) {
            return RemoveItemFromInventorySlot(i);
        }
    }
    return FInventoryItem{};
}

bool UCSNOInventoryComponent::HasItemInInventory(FName WeaponName) {
    for (int i = 0; i < InventoryArray.Num(); ++i) {
        if (InventoryArray[i].ItemName == WeaponName) {
            return true;
        }
    }
    return false;
}

// Called when the game starts
void UCSNOInventoryComponent::BeginPlay() {
    Super::BeginPlay();
    Player = GetCharacter();

    for (int i = 0; i < InventoryArray.Num(); ++i) {
        if (InventoryArray[i].ItemName != "") {
            SpawnWeapon(StartWeaponClass, InventoryArray[i]);
            return;
        }
    }
}

void UCSNOInventoryComponent::SpawnWeapon(TSubclassOf<ACSNOWeaponBase> ClassToSpawn, FInventoryItem& InventoryItem) {
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = GetOwner();
    if (GetOwner()->HasAuthority()) {
        CurrentFPWeapon = GetWorld()->SpawnActor<ACSNOWeaponBase>(ClassToSpawn, FVector::ZeroVector,
                                                                  FRotator::ZeroRotator, SpawnParams);

        if (CurrentFPWeapon && Player && WeaponInfoDataTable) {
            CurrentFPWeapon->SetOwner(GetOwner());
            CurrentFPWeapon->AttachToComponent(Player->GetMeshFP(),
                                               FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                               WeaponAttachSocketName);

            const FString ContextString(TEXT("Weapon Change Context"));
            FWeaponInfo* WeaponInfo = WeaponInfoDataTable->FindRow<FWeaponInfo>(InventoryItem.ItemName, ContextString);
            if (WeaponInfo) {
                CurrentFPWeapon->Init(*WeaponInfo, InventoryItem);
            }
        }
    }
}

ACSNOCharacter* UCSNOInventoryComponent::GetCharacter() {
    return Cast<ACSNOCharacter>(GetOwner());
}

// Called every frame
void UCSNOInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

bool UCSNOInventoryComponent::ChangeInventorySlot(int SlotNr) {
    if (CurrentItemSlot != SlotNr) {

        if (!GetOwner()->HasAuthority()) {
            ServerChangeInventorySlot(SlotNr);
        }

        FInventoryItem SlotWeapon = InventoryArray[SlotNr];
        if (SlotWeapon.ItemName != "") {

            PreviousItemSlot = CurrentItemSlot;
            InventoryArray[PreviousItemSlot].ClipAmmo = CurrentFPWeapon->GetCurrentClipAmmo();
            InventoryArray[PreviousItemSlot].TotalAmmo = CurrentFPWeapon->GetCurrentTotalAmmo();
            CurrentItemSlot = SlotNr;

            if (WeaponInfoDataTable) {
                const FString ContextString(TEXT("Weapon Change Context"));
                FWeaponInfo* WeaponInfo = WeaponInfoDataTable->FindRow<FWeaponInfo>(SlotWeapon.ItemName, ContextString);

                if (WeaponInfo && CurrentFPWeapon->IsA(WeaponInfo->WeaponClass)) {
                    CurrentFPWeapon->Init(*WeaponInfo, SlotWeapon);
                }
                else {
                    CurrentFPWeapon->Destroy();
                    SpawnWeapon(WeaponInfo->WeaponClass, SlotWeapon);
                }
            }
            return true;
        }
        return false;
    }
    return true;
}

void UCSNOInventoryComponent::ChangeToPreviousInventorySlot() {
    if (!ChangeInventorySlot(PreviousItemSlot)) {
        for (int i = 0; i < InventoryArray.Num(); ++i) {
            if (InventoryArray[i].ItemName != "") {
                ChangeInventorySlot(i);
                return;
            }
        }
    }
}

void UCSNOInventoryComponent::ServerChangeInventorySlot_Implementation(int SlotNr) {
    ChangeInventorySlot(SlotNr);
}

bool UCSNOInventoryComponent::ServerChangeInventorySlot_Validate(int SlotNr) {
    return true;
}

void UCSNOInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCSNOInventoryComponent, CurrentFPWeapon)
    //DOREPLIFETIME(UCSNOInventoryComponent, InventoryArray)
}
