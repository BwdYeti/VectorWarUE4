// Copyright 2020 BwdYeti.

#include "GameStateInterface.h"
#include "vectorwar.h"

int32 UGameStateInterface::ShipCount()
{
	return gs._num_ships;
}

int32 UGameStateInterface::BulletCount()
{
	return ShipCount() * MAX_BULLETS;
}

FTransform UGameStateInterface::ShipTransform(int32 Index)
{
	const Ship Ship = gs._ships[Index];
	FVector Position = FVector((float)Ship.position.x, (float)Ship.position.y, 0);
	FQuat Rotation = FRotator(0, Ship.heading - 90, 0).Quaternion();
	FTransform Result = FTransform(Rotation, Position);
	return Result;
}

bool UGameStateInterface::BulletEnabled(int32 Index)
{
	const Bullet Bullet = GetBullet(Index);
	return Bullet.active;
}

FTransform UGameStateInterface::BulletTransform(int32 Index)
{
	const Bullet Bullet = GetBullet(Index);
	FVector Position = FVector((float)Bullet.position.x, (float)Bullet.position.y, 0);
	FQuat Rotation = FQuat::Identity;
	FTransform Result = FTransform(Rotation, Position);
	return Result;
}

Bullet UGameStateInterface::GetBullet(int32 Index)
{
	const Ship Ship = gs._ships[Index / MAX_BULLETS];
	return Ship.bullets[Index % MAX_BULLETS];
}

