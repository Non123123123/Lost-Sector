#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "Interactable.h"
#include "ItemPickup.generated.h"

UCLASS()
class LOSTSECTOR_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    
    AItemPickup();

    UPROPERTY(EditAnywhere, Replicated) FItemStack Stack;
    UPROPERTY(EditAnywhere) float MaxUseDistance = 220.f;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void Interact(class ACharacter* ByWho) override;
};
