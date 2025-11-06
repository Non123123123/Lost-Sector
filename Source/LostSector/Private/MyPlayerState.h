#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterDataStructs.h"
#include "Net/UnrealNetwork.h"
#include "MyPlayerState.generated.h"

UCLASS()
class AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FCharacterData PlayerData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(AMyPlayerState, PlayerData);
	}
	UFUNCTION(BlueprintCallable)
	void UpgradeStat(EStatTypes Stat, int32 Amount);

};