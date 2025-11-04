// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataBase.generated.h"


UENUM(BlueprintType)
enum class EItemType :uint8 { Ammo, Med, Weapon, Key, Material, Quest };

UCLASS(BlueprintType)
class LOSTSECTOR_API UItemDataBase : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName ItemId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) UTexture2D* Icon = nullptr; // UI ³ªÁß¿ë
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float Weight = 0.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 MaxStack = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) EItemType Type = EItemType::Material;
};