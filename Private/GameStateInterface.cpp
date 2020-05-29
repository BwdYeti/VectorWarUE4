// Copyright 2020 BwdYeti.

#include "GameStateInterface.h"
#include "vectorwar.h"

int32 UGameStateInterface::ShipCount()
{
	return gs._num_ships;
}

FTransform UGameStateInterface::ShipTransform(int32 Index)
{
	const Ship Ship = gs._ships[Index];
	FVector Position = FVector((float)Ship.position.x, (float)Ship.position.y, 0);
	FQuat Rotation = FRotator(0, Ship.heading - 90, 0).Quaternion();
	FTransform Result = FTransform(Rotation, Position);
	return Result;
}

