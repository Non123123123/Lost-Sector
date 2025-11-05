#pragma once
#include "CoreMinimal.h"
#include "ItemDataBase.h"
#include "ItemTypes.generated.h"   // ← 항상 마지막 include

USTRUCT(BlueprintType)
struct FItemStack          // ← API 매크로 넣지 마세요
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemDataBase* Item = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    bool  IsValid()     const { return Item && Count > 0; }
    bool  IsSame(const FItemStack& O) const { return Item && Item == O.Item; }
    int32 FreeSpace()   const { return Item ? FMath::Max(0, Item->MaxStack - Count) : 0; }
    float TotalWeight() const { return Item ? Item->Weight * Count : 0.f; }
};
