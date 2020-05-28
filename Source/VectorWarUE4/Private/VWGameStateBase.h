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

private:
	HWND StartSinglePlayerGGPOSession();
	HWND StartGGPOPlayerSession(const uint16 LocalPort, const int32 NumPlayers, TArray<wchar_t*> PlayerParameters);

	HWND Hwnd;

	float ElapsedTime;

};
