// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
#include "nongamestate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameStateInterface.generated.h"

// Forward declarations
struct Bullet;

/**
 * 
 */
UCLASS()
class VECTORWAR_API UGameStateInterface : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Gets the number of ships in the game state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 ShipCount();

	/** Gets the total number of bullets allocated by active ships. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 BulletCount();

	/**
	 * Gets the Transform of a player's ship.
	 * @param Index						Index of the player
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static FTransform ShipTransform(int32 Index);

	/**
	 * Gets a player's score.
	 * @param Index						Index of the player
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 ShipScore(int32 Index);

	/**
	 * Gets whether a bullet is enabled.
	 * @param Index						Bullet index
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static bool BulletEnabled(int32 Index);

	/**
	 * Gets the Transform of a bullet.
	 * @param Index						Bullet index
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static FTransform BulletTransform(int32 Index);

private:

	/**
	 * Gets a Bullet.
	 * @param Index						Bullet index
	 */
	static Bullet GetBullet(int32 Index);

public:
	/**
	 * Gets a player's connection info.
	 * @param Index						Index of the player
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static FPlayerConnectionInfo ConnectionInfo(int32 Index);
	//static int32 ConnectionState(int32 Index);

	/** Gets the frame number of the game state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGPO")
	static int32 FrameNumber();

	/** Gets the checksum of the game state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGPO")
	static int32 Checksum();

	/** Gets the frame number of the game state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGPO")
		static int32 PeriodicFrame();

	/** Gets the checksum of the game state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGPO")
		static int32 PeriodicChecksum();
};
