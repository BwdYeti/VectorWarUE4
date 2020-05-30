// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 ShipCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 BulletCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static FTransform ShipTransform(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static int32 ShipScore(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static bool BulletEnabled(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameState")
	static FTransform BulletTransform(int32 Index);

private:

	static Bullet GetBullet(int32 Index);
};
