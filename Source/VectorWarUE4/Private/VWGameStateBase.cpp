// Copyright 2020 BwdYeti.

#include "VWGameStateBase.h"
#include "VectorWarPlayerController.h"
#include "VectorWar/GameStateInterface.h"
#include "include/ggponet.h"
#include "GGPOGameInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#define ARRAYSIZE(a) sizeof(a) / sizeof(a[0])
// Because of how it's coded, the original VectorWar runs at 62 fps, not 60
#define FRAME_RATE 62
#define ONE_FRAME (1.0f / FRAME_RATE)

void AVWGameStateBase::BeginPlay()
{
    Super::BeginPlay();

    UGGPONetwork* NetworkAddresses = nullptr;
    int32 NumPlayers = 1;

    // If this is a GGPO game instance
    UGameInstance* GameInstance = GetGameInstance();
    UGGPOGameInstance* GgpoGameInstance = Cast<UGGPOGameInstance>(GameInstance);
    if (GgpoGameInstance != nullptr)
    {
        // Get the network addresses
        NetworkAddresses = GgpoGameInstance->NetworkAddresses;
        NumPlayers = NetworkAddresses->NumPlayers();
        // Reset the game instance network addresses
        GgpoGameInstance->NetworkAddresses = nullptr;
    }

    bSessionStarted = TryStartGGPOPlayerSession(NumPlayers, NetworkAddresses);

    if (bSessionStarted)
    {
        OnSessionStarted();

        NetworkGraphData.Empty();
        TArray<FGGPONetworkStats> Network = UGameStateInterface::NetworkStats();
        int32 Count = Network.Num();
        for (int32 i = 0; i < Count; i++)
        {
            NetworkGraphData.Add(FNetworkGraphPlayer{ });
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GGPO session"));
    }
}

void AVWGameStateBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MSG msg = { 0 };

    ElapsedTime += DeltaSeconds;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) { }
    }
    int32 IdleMs = (int32)(ONE_FRAME - (int32)(ElapsedTime * 1000));
    VectorWarHost::VectorWar_Idle(FMath::Max(0, IdleMs - 1));
    while (ElapsedTime >= ONE_FRAME) {
        RunFrame();

        ElapsedTime -= ONE_FRAME;
    }
}

void AVWGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (bSessionStarted)
    {
        VectorWarHost::VectorWar_Exit();

        bSessionStarted = false;
    }
}

void AVWGameStateBase::OnSessionStarted_Implementation() { }

int32 AVWGameStateBase::GetFrameRate()
{
    return FRAME_RATE;
}

TArray<FVector2D> AVWGameStateBase::GetNetworkGraphData(int32 Index, ENetworkGraphType Type, FVector2D GraphSize, int32 MinY, int32 MaxY) const
{
    TArray<FVector2D> Result = TArray<FVector2D>();

    // Return an empty array if there's no entry for this index
    if (Index >= NetworkGraphData.Num())
        return Result;

    TArray<FNetworkGraphData> PlayerData = NetworkGraphData[Index].PlayerData;
    for (int32 i = 0; i < PlayerData.Num(); i++)
    {
        int32 IntValue = 0;
        switch (Type)
        {
        case ENetworkGraphType::PING:
            IntValue = PlayerData[i].Ping;
            break;
        case ENetworkGraphType::SYNC:
            IntValue = PlayerData[i].Fairness;
            break;
        case ENetworkGraphType::REMOTE_SYNC:
            IntValue = PlayerData[i].RemoteFairness;
            break;
        }

        float Value = GraphValue(IntValue, GraphSize, MinY, MaxY);
        float X = (i * (GraphSize.X - 1)) / NETWORK_GRAPH_STEPS;
        Result.Add(FVector2D(X, Value));
    }

    return Result;
}

float AVWGameStateBase::GraphValue(int32 Value, FVector2D GraphSize, int32 MinY, int32 MaxY)
{
    float Result = 0.f;

    int32 DiffY = MaxY - MinY;
    if (DiffY > 0)
    {
        int32 IntValue = FMath::Clamp(Value - MinY, 0, DiffY);
        Result = IntValue / (float)DiffY;
        Result = (1.f - Result) * (GraphSize.Y - 1);
    }

    return Result;
}

void AVWGameStateBase::RunFrame()
{
    int32 Input = GetLocalInputs();
    VectorWarHost::VectorWar_RunFrame(Input);

    // Network data
    TArray<FGGPONetworkStats> Network = UGameStateInterface::NetworkStats();
    for (int32 i = 0; i < NetworkGraphData.Num(); i++)
    {
        TArray<FNetworkGraphData>* PlayerData = &NetworkGraphData[i].PlayerData;

        int32 Fairness;
        int32 LocalFairness = Network[i].timesync.local_frames_behind;
        int32 RemoteFairness = Network[i].timesync.remote_frames_behind;
        int32 Ping = Network[i].network.ping;

        if (LocalFairness < 0 && RemoteFairness < 0) {
            /*
             * Both think it's unfair (which, ironically, is fair).  Scale both and subtrace.
             */
            Fairness = abs(abs(LocalFairness) - abs(RemoteFairness));
        }
        else if (LocalFairness > 0 && RemoteFairness > 0) {
            /*
             * Impossible!  Unless the network has negative transmit time.  Odd....
             */
            Fairness = 0;
        }
        else {
            /*
             * They disagree.  Add.
             */
            Fairness = abs(LocalFairness) + abs(RemoteFairness);
        }

        FNetworkGraphData GraphData = FNetworkGraphData{ Fairness, RemoteFairness, Ping };
        PlayerData->Add(GraphData);

        while (PlayerData->Num() > NETWORK_GRAPH_STEPS)
        {
            PlayerData->RemoveAt(0);
        }
    }
}

bool AVWGameStateBase::TryStartGGPOPlayerSession(
    int32 NumPlayers,
    const UGGPONetwork* NetworkAddresses)
{
    int32 Offset = 0;
    GGPOPlayer Players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];
    int32 NumSpectators = 0;

    uint16 LocalPort;

    // If there are no 
    if (NetworkAddresses == nullptr)
    {
        Players[0].size = sizeof(Players[0]);
        Players[0].player_num = 1;
        Players[0].type = EGGPOPlayerType::LOCAL;

        LocalPort = 7000;
        NumPlayers = 1;
    }
    else
    {
        if (NumPlayers > NetworkAddresses->NumPlayers())
            return false;

        LocalPort = NetworkAddresses->GetLocalPort();

        int32 i;
        for (i = 0; i < NumPlayers; i++)
        {
            Offset++;

            Players[i].size = sizeof(Players[i]);
            Players[i].player_num = i + 1;
            // The local player
            if (i == NetworkAddresses->GetPlayerIndex()) {
                Players[i].type = EGGPOPlayerType::LOCAL;
                continue;
            }

            Players[i].type = EGGPOPlayerType::REMOTE;
            Players[i].u.remote.port = (uint16)NetworkAddresses->GetAddress(i)->GetPort();
            NetworkAddresses->GetAddress(i)->GetIpAddress(Players[i].u.remote.ip_address);
        }
        // these are spectators...
        while (Offset < NetworkAddresses->NumPlayers()) {
            Offset++;

            Players[i].type = EGGPOPlayerType::SPECTATOR;
            Players[i].u.remote.port = (uint16)NetworkAddresses->GetAddress(i)->GetPort();
            NetworkAddresses->GetAddress(i)->GetIpAddress(Players[i].u.remote.ip_address);

            i++;
            NumSpectators++;
        }
    }

    VectorWarHost::VectorWar_Init(LocalPort, NumPlayers, Players, NumSpectators);

    UE_LOG(LogTemp, Display, TEXT("GGPO session started"));

    return true;
}

bool AVWGameStateBase::TryStartGGPOSpectatorSession(
    const uint16 LocalPort,
    const int32 NumPlayers,
    wchar_t* HostParameter)
{
    int32 Offset = 0;
    wchar_t WideIpBuffer[128];
    uint32 WideIpBufferSize = (uint32)ARRAYSIZE(WideIpBuffer);

    char HostIp[128];
    uint16 HostPort;
    if (swscanf_s(HostParameter, L"%[^:]:%hu", WideIpBuffer, WideIpBufferSize, &HostPort) != 2) {
        return 1;
    }
    wcstombs_s(nullptr, HostIp, ARRAYSIZE(HostIp), WideIpBuffer, _TRUNCATE);

    VectorWarHost::VectorWar_InitSpectator(LocalPort, NumPlayers, HostIp, HostPort);

    UE_LOG(LogTemp, Display, TEXT("GGPO spectator session started"));

    return true;
}

int32 AVWGameStateBase::GetLocalInputs()
{
    const UObject* world = (UObject*)GetWorld();
    AVectorWarPlayerController* Controller = Cast<AVectorWarPlayerController>(UGameplayStatics::GetPlayerController(world, 0));
    if (Controller)
    {
        return Controller->GetVectorWarInput();
    }
    return 0;
}
