#include "InventoryComponent.h"
#include "ItemPickup.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Slots.Num() != SlotCount)
    {
        InitSlots();
    }
}

void UInventoryComponent::InitSlots()
{
    Slots.SetNum(SlotCount);

    if (GetOwnerRole() == ROLE_Authority)
    {
        for (int32 i = 0; i < SlotCount; ++i)
        {
            Slots[i] = FItemStack();
        }
    }

    BroadcastUpdated();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, Slots);
}

float UInventoryComponent::GetTotalWeight() const
{
    float Total = 0.f;
    for (const FItemStack& Stack : Slots)
    {
        Total += Stack.TotalWeight();
    }
    return Total;
}

bool UInventoryComponent::CanAddWeight(float AddW) const
{
    return (GetTotalWeight() + AddW) <= WeightLimit;
}

void UInventoryComponent::BroadcastUpdated()
{
    // 나중에 UI 새로고침용 (현재 비워둠)
}

void UInventoryComponent::OnRep_Slots()
{
    BroadcastUpdated();
}

bool UInventoryComponent::TryAddStack(const FItemStack& InStack, int32& OutAdded)
{
    OutAdded = 0;
    if (!InStack.IsValid()) return false;

    float AddWeight = InStack.Item->Weight * InStack.Count;
    if (!CanAddWeight(AddWeight)) return false;

    for (FItemStack& S : Slots)
    {
        if (S.IsSame(InStack))
        {
            int32 Free = S.FreeSpace();
            int32 ToAdd = FMath::Min(Free, InStack.Count - OutAdded);
            if (ToAdd > 0)
            {
                S.Count += ToAdd;
                OutAdded += ToAdd;
            }
            if (OutAdded >= InStack.Count)
                return true;
        }
    }

    for (FItemStack& S : Slots)
    {
        if (!S.IsValid())
        {
            int32 ToAdd = InStack.Count - OutAdded;
            S = InStack;
            S.Count = ToAdd;
            OutAdded += ToAdd;
            return true;
        }
    }
    return OutAdded > 0;
}

bool UInventoryComponent::TryMove(int32 FromIdx, int32 ToIdx)
{
    if (!ValidIndex(FromIdx) || !ValidIndex(ToIdx) || FromIdx == ToIdx) return false;

    FItemStack& From = Slots[FromIdx];
    FItemStack& To = Slots[ToIdx];
    if (!From.IsValid()) return false;

    if (!To.IsValid())
    {
        To = From;
        From = FItemStack();
        return true;
    }
    else if (To.IsSame(From))
    {
        int32 Free = To.FreeSpace();
        int32 Move = FMath::Min(Free, From.Count);
        To.Count += Move;
        From.Count -= Move;
        if (From.Count <= 0) From = FItemStack();
        return true;
    }

    return false;
}

bool UInventoryComponent::TrySplit(int32 FromIdx, int32 NumToSplit, int32 ToIdx)
{
    if (!ValidIndex(FromIdx) || !ValidIndex(ToIdx) || FromIdx == ToIdx) return false;

    FItemStack& From = Slots[FromIdx];
    FItemStack& To = Slots[ToIdx];

    if (!From.IsValid() || NumToSplit <= 0 || NumToSplit >= From.Count) return false;
    if (To.IsValid()) return false;

    To = From;
    To.Count = NumToSplit;
    From.Count -= NumToSplit;
    return true;
}

bool UInventoryComponent::RemoveAt(int32 Index, int32 Count)
{
    if (!ValidIndex(Index)) return false;
    FItemStack& S = Slots[Index];
    if (!S.IsValid() || Count <= 0) return false;

    S.Count -= Count;
    if (S.Count <= 0) S = FItemStack();
    return true;
}

bool UInventoryComponent::TransferFrom(UInventoryComponent* From, int32 FromIdx, int32 Count, int32& OutMoved)
{
    if (!From || !From->ValidIndex(FromIdx)) return false;

    const FItemStack& Src = From->Slots[FromIdx];
    if (!Src.IsValid()) return false;

    FItemStack Partial = Src;
    Partial.Count = Count;
    return TryAddStack(Partial, OutMoved);
}

bool UInventoryComponent::TransferAllFrom(UInventoryComponent* From, int32& OutTotalMoved)
{
    if (!From) return false;
    OutTotalMoved = 0;

    for (int32 i = 0; i < From->Slots.Num(); ++i)
    {
        FItemStack Stack = From->Slots[i];
        if (!Stack.IsValid()) continue;

        int32 Added = 0;
        if (TryAddStack(Stack, Added))
        {
            From->RemoveAt(i, Added);
            OutTotalMoved += Added;
        }
    }
    return OutTotalMoved > 0;
}

bool UInventoryComponent::DropAt(int32 FromIdx, int32 Count, const FTransform& WorldTransform, TSubclassOf<AItemPickup> PickupClass)
{
    if (!PickupClass || !ValidIndex(FromIdx)) return false;

    FItemStack& S = Slots[FromIdx];
    if (!S.IsValid() || Count <= 0 || Count > S.Count) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AItemPickup* Pickup = World->SpawnActor<AItemPickup>(PickupClass, WorldTransform, Params);
    if (!Pickup) return false;

    Pickup->Stack = S;
    Pickup->Stack.Count = Count;

    RemoveAt(FromIdx, Count);
    return true;
}

// ---------- ✅ 서버 RPC 구현부 ----------
void UInventoryComponent::Server_TryAddStack_Implementation(const FItemStack& InStack)
{
    int32 Added = 0;
    TryAddStack(InStack, Added);
}

void UInventoryComponent::Server_TryMove_Implementation(int32 FromIdx, int32 ToIdx)
{
    TryMove(FromIdx, ToIdx);
}

void UInventoryComponent::Server_TrySplit_Implementation(int32 FromIdx, int32 NumToSplit, int32 ToIdx)
{
    TrySplit(FromIdx, NumToSplit, ToIdx);
}

void UInventoryComponent::Server_TransferFrom_Implementation(UInventoryComponent* From, int32 FromIdx, int32 Count)
{
    int32 Moved = 0;
    TransferFrom(From, FromIdx, Count, Moved);
}

void UInventoryComponent::Server_TransferAllFrom_Implementation(UInventoryComponent* From)
{
    int32 TotalMoved = 0;
    TransferAllFrom(From, TotalMoved);
}

void UInventoryComponent::Server_DropAt_Implementation(int32 FromIdx, int32 Count, const FTransform& Xform, TSubclassOf<AItemPickup> PickupClass)
{
    DropAt(FromIdx, Count, Xform, PickupClass);
}
