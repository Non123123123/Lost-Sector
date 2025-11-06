#include "ItemPickup.h"
#include "InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UPROPERTY(VisibleAnywhere)UStaticMeshComponent* Mesh;

AItemPickup::AItemPickup()
{
    bReplicates = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemPickup, Stack);
}
void AItemPickup::Interact(ACharacter* ByWho)
{
    if (!ByWho || GetLocalRole() != ROLE_Authority || !Stack.IsValid()) return;
    if (FVector::Dist(ByWho->GetActorLocation(), GetActorLocation()) > MaxUseDistance) return;

    if (UInventoryComponent* Inv = ByWho->FindComponentByClass<UInventoryComponent>())
    {
        int32 Added = 0;
        Inv->TryAddStack(Stack, Added);
        if (Added > 0)
        {
            Stack.Count -= Added;
            if (Stack.Count <= 0) Destroy();
        }
    }
}

