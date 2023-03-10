// Copyright (c) 2022-present Barrelhouse
// Written by Tim Verberne
// This source code is part of the project Frostbite

#include "RoomVolume.h"
#include "Nightstalker.h"
#include "PlayerCharacter.h"
#include "LogCategories.h"

void ARoomVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if(APlayerCharacter* PlayerCharacter {Cast<APlayerCharacter>(OtherActor)})
	{
		OnPlayerEnter.Broadcast(PlayerCharacter);
		EventOnPlayerEnter(PlayerCharacter);

		UE_LOG(LogRoomVolume, Verbose, TEXT("Player has entered room %s."), *this->GetName())
		
		return;
	}
	if(ANightstalker* Nightstalker {Cast<ANightstalker>(OtherActor)})
	{
		OnNightstalkerEnter.Broadcast(Nightstalker);
		EventOnNightstalkerEnter(Nightstalker);

		UE_LOG(LogRoomVolume, Verbose, TEXT("Nightstalker has entered room %s."), *this->GetName())
	}
}

void ARoomVolume::SetLightStatus(const bool Value)
{
	if(IsLit == Value) {return; }
	IsLit = Value;
	OnLuminosityChanged.Broadcast(Value);
}

void ARoomVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	if(APlayerCharacter* PlayerCharacter {Cast<APlayerCharacter>(OtherActor)})
	{
		OnPlayerLeave.Broadcast(PlayerCharacter);
		EventOnPlayerLeave(PlayerCharacter);

		UE_LOG(LogRoomVolume, Verbose, TEXT("Player has left room %s."), *this->GetName())
		
		return;
	}
	if(ANightstalker* Nightstalker {Cast<ANightstalker>(OtherActor)})
	{
		OnNightstalkerLeave.Broadcast(Nightstalker);
		EventOnNightstalkerLeave(Nightstalker);

		UE_LOG(LogRoomVolume, Verbose, TEXT("Nightstalker has left room %s."), *this->GetName())
	}
}

// BLUEPRINT NATIVE EVENTS
void ARoomVolume::EventOnPlayerEnter_Implementation(APlayerCharacter* PlayerCharacter)
{
}
void ARoomVolume::EventOnPlayerLeave_Implementation(APlayerCharacter* PlayerCharacter)
{
}
void ARoomVolume::EventOnNightstalkerEnter_Implementation(ANightstalker* Nightstalker)
{
}
void ARoomVolume::EventOnNightstalkerLeave_Implementation(ANightstalker* Nightstalker)
{
}


