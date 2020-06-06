// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
#include "vectorwar.h"
#include "GameFramework/GameStateBase.h"
#include "VWGameStateBase.generated.h"

/**
 *
 */
UCLASS()
class AVWGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called from BeginPlay() after creating the game state.
	 * Can be overridden by a blueprint to create actors that represent the game state.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="GGPO")
	void OnSessionStarted();
	virtual void OnSessionStarted_Implementation();

private:
	/** Starts a single player GGPO game session. */
	HWND StartSinglePlayerGGPOSession();
	/** Starts a GGPO game session. */
	HWND StartGGPOPlayerSession(const uint16 LocalPort, const int32 NumPlayers, TArray<wchar_t*> PlayerParameters);

	HWND Hwnd;

	float ElapsedTime;

};
