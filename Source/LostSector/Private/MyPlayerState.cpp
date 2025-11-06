// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"

void AMyPlayerState::UpgradeStat(EStatTypes Stat, int32 Amount)
{
	switch (Stat)
	{
	case EStatTypes::Hp:
		PlayerData.Hp += Amount;
		break;
	case EStatTypes::Stamina:
		PlayerData.Stamina += Amount;
		break;
	case EStatTypes::Hungry:
		PlayerData.hungry += Amount;
		break;
	case EStatTypes::Weight:
		PlayerData.weight += Amount;
		break;
	}
}
