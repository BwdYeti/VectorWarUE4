// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
#include "VectorWar/vectorwar.h"
#include "GameFramework/GameStateBase.h"
#include "VWGameStateBase.generated.h"

#define NETWORK_GRAPH_STEPS 720

UENUM(BlueprintType)
enum class ENetworkGraphType : uint8
{
	PING           UMETA(DisplayName = "Ping"),
	SYNC           UMETA(DisplayName = "Fairness"),
	REMOTE_SYNC    UMETA(DisplayName = "Remote Fairness"),
};

USTRUCT(BlueprintType)
struct FNetworkGraphData {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32   Fairness;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32   RemoteFairness;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32   Ping;
};

USTRUCT(BlueprintType)
struct FNetworkGraphPlayer {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FNetworkGraphData> PlayerData;
};

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

	/** Get the game state frame rate */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game State")
	static int32 GetFrameRate();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGPO")
	TArray<FVector2D> GetNetworkGraphData(int32 Index, ENetworkGraphType Type, FVector2D GraphSize, int32 MinY, int32 MaxY) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Graph")
	static float GraphValue(int32 Value, FVector2D GraphSize, int32 MinY, int32 MaxY);

private:
	void RunFrame();

	/** Starts a single player GGPO game session. */
	bool TryStartSinglePlayerGGPOSession();
	/** Starts a GGPO game session. */
	bool TryStartGGPOPlayerSession(const uint16 LocalPort, const int32 NumPlayers, TArray<wchar_t*> PlayerParameters);
	/** Starts a GGPO spectating game session. */
	bool TryStartGGPOSpectatorSession(const uint16 LocalPort, const int32 NumPlayers, wchar_t* HostParameter);
	/** Gets the inputs from the local player. */
	int32 GetLocalInputs();

	bool bSessionStarted;

	float ElapsedTime;

	TArray<FNetworkGraphPlayer> NetworkGraphData;

};
