// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameStateInterface.generated.h"

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
	static FTransform ShipTransform(int32 Index);
};
