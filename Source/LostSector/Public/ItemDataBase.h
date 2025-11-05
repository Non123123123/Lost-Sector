#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon,         // 총기
    Ammo,           // 탄약
    Heal,           // 힐템
    Food,           // 음식
    Armor,          // 방탄복
    Junk,           // 잡동사니
    Valuable,       // 귀중품
    BossItem,       // 보스 전용 아이템
    Bag,            // 가방
};

/**
 * 아이템의 공통 속성 정의
 */
UCLASS(BlueprintType)
class LOSTSECTOR_API UItemDataBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 🔹 아이템 고유 ID (데이터 검색용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId;

    // 🔹 게임 내 표시 이름
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    // 🔹 아이콘 (UI용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon = nullptr;

    // 🔹 무게
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float Weight = 0.f;

    // 🔹 최대 스택 개수 (예: 총알 30, 힐템 1)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 1;

    // 🔹 아이템 종류 (총 / 힐템 / 음식 등)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Junk;

    // 🔹 아이템 설명 (선택)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;
};
