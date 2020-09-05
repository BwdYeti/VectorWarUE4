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

    VectorWar_Init(LocalPort, NumPlayers, Players, NumSpectators);

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

    VectorWar_InitSpectator(LocalPort, NumPlayers, HostIp, HostPort);

    UE_LOG(LogTemp, Display, TEXT("GGPO spectator session started"));

    return true;
}

void AVWGameStateBase::VectorWar_Init(uint16 localport, int32 num_players, GGPOPlayer* players, int32 num_spectators)
{
    GGPOErrorCode result;

    // Initialize the game state
    gs.Init(num_players);
    ngs.num_players = num_players;

    // Fill in a ggpo callbacks structure to pass to start_session.
    GGPOSessionCallbacks cb = CreateCallbacks();

#if defined(SYNC_TEST)
    result = GGPONet::ggpo_start_synctest(&ggpo, &cb, "vectorwar", num_players, sizeof(int), 1);
#else
    result = GGPONet::ggpo_start_session(&ggpo, &cb, "vectorwar", num_players, sizeof(int), localport);
#endif

    // automatically disconnect clients after 3000 ms and start our count-down timer
    // for disconnects after 1000 ms.   To completely disable disconnects, simply use
    // a value of 0 for ggpo_set_disconnect_timeout.
    GGPONet::ggpo_set_disconnect_timeout(ggpo, 3000);
    GGPONet::ggpo_set_disconnect_notify_start(ggpo, 1000);

    int i;
    for (i = 0; i < num_players + num_spectators; i++) {
        GGPOPlayerHandle handle;
        result = GGPONet::ggpo_add_player(ggpo, players + i, &handle);
        ngs.players[i].handle = handle;
        ngs.players[i].type = players[i].type;
        if (players[i].type == EGGPOPlayerType::LOCAL) {
            ngs.players[i].connect_progress = 100;
            ngs.local_player_handle = handle;
            ngs.SetConnectState(handle, EPlayerConnectState::Connecting);
            GGPONet::ggpo_set_frame_delay(ggpo, handle, FRAME_DELAY);
        }
        else {
            ngs.players[i].connect_progress = 0;
        }
    }

    GGPONet::ggpo_try_synchronize_local(ggpo);
}
void AVWGameStateBase::VectorWar_InitSpectator(uint16 localport, int32 num_players, char* host_ip, uint16 host_port)
{
    GGPOErrorCode result;

    // Initialize the game state
    gs.Init(num_players);
    ngs.num_players = num_players;

    // Fill in a ggpo callbacks structure to pass to start_session.
    GGPOSessionCallbacks cb = CreateCallbacks();

    result = GGPONet::ggpo_start_spectating(&ggpo, &cb, "vectorwar", num_players, sizeof(int), localport, host_ip, host_port);
}

GGPOSessionCallbacks AVWGameStateBase::CreateCallbacks()
{
    GGPOSessionCallbacks cb = { 0 };

    cb.begin_game = std::bind(&AVWGameStateBase::vw_begin_game_callback, this, std::placeholders::_1);
    cb.save_game_state = std::bind(&AVWGameStateBase::vw_save_game_state_callback, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    cb.load_game_state = std::bind(&AVWGameStateBase::vw_load_game_state_callback, this,
        std::placeholders::_1, std::placeholders::_2);
    cb.log_game_state = std::bind(&AVWGameStateBase::vw_log_game_state, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    cb.free_buffer = std::bind(&AVWGameStateBase::vw_free_buffer, this, std::placeholders::_1);
    cb.advance_frame = std::bind(&AVWGameStateBase::vw_advance_frame_callback, this, std::placeholders::_1);
    cb.on_event = std::bind(&AVWGameStateBase::vw_on_event_callback, this, std::placeholders::_1);

    return cb;
}


bool AVWGameStateBase::vw_begin_game_callback(const char*)
{
    return true;
}
bool AVWGameStateBase::vw_save_game_state_callback(unsigned char** buffer, int32* len, int32* checksum, int32)
{
    *len = sizeof(gs);
    *buffer = (unsigned char*)malloc(*len);
    if (!*buffer) {
        return false;
    }
    memcpy(*buffer, &gs, *len);
    *checksum = fletcher32_checksum((short*)*buffer, *len / 2);
    return true;
}
bool AVWGameStateBase::vw_load_game_state_callback(unsigned char* buffer, int32 len)
{
    memcpy(&gs, buffer, len);
    return true;
}
bool AVWGameStateBase::vw_log_game_state(char* filename, unsigned char* buffer, int32)
{
    FILE* fp = nullptr;
    fopen_s(&fp, filename, "w");
    if (fp) {
        GameState* gamestate = (GameState*)buffer;
        fprintf(fp, "GameState object.\n");
        fprintf(fp, "  bounds: %ld,%ld x %ld,%ld.\n", gamestate->_bounds.left, gamestate->_bounds.top,
            gamestate->_bounds.right, gamestate->_bounds.bottom);
        fprintf(fp, "  num_ships: %d.\n", gamestate->_num_ships);
        for (int i = 0; i < gamestate->_num_ships; i++) {
            Ship* ship = gamestate->_ships + i;
            fprintf(fp, "  ship %d position:  %.4f, %.4f\n", i, ship->position.x, ship->position.y);
            fprintf(fp, "  ship %d velocity:  %.4f, %.4f\n", i, ship->velocity.dx, ship->velocity.dy);
            fprintf(fp, "  ship %d radius:    %d.\n", i, ship->radius);
            fprintf(fp, "  ship %d heading:   %d.\n", i, ship->heading);
            fprintf(fp, "  ship %d health:    %d.\n", i, ship->health);
            fprintf(fp, "  ship %d speed:     %d.\n", i, ship->speed);
            fprintf(fp, "  ship %d cooldown:  %d.\n", i, ship->cooldown);
            fprintf(fp, "  ship %d score:     %d.\n", i, ship->score);
            for (int j = 0; j < MAX_BULLETS; j++) {
                Bullet* bullet = ship->bullets + j;
                fprintf(fp, "  ship %d bullet %d: %.2f %.2f -> %.2f %.2f.\n", i, j,
                    bullet->position.x, bullet->position.y,
                    bullet->velocity.dx, bullet->velocity.dy);
            }
        }
        fclose(fp);
    }
    return true;
}
void AVWGameStateBase::vw_free_buffer(void* buffer)
{
    free(buffer);
}
bool AVWGameStateBase::vw_advance_frame_callback(int32)
{
    int inputs[MAX_SHIPS] = { 0 };
    int disconnect_flags;

    // Make sure we fetch new inputs from GGPO and use those to update
    // the game state instead of reading from the keyboard.
    GGPONet::ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_SHIPS, &disconnect_flags);
    VectorWar_AdvanceFrame(inputs, disconnect_flags);
    return true;
}
bool AVWGameStateBase::vw_on_event_callback(GGPOEvent* info)
{
    int progress;
    switch (info->code) {
    case GGPO_EVENTCODE_CONNECTED_TO_PEER:
        ngs.SetConnectState(info->u.connected.player, EPlayerConnectState::Synchronizing);
        break;
    case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
        progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
        ngs.UpdateConnectProgress(info->u.synchronizing.player, progress);
        break;
    case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
        ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
        break;
    case GGPO_EVENTCODE_RUNNING:
        ngs.SetConnectState(EPlayerConnectState::Running);
        break;
    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        ngs.SetDisconnectTimeout(info->u.connection_interrupted.player,
            get_time(),
            info->u.connection_interrupted.disconnect_timeout);
        break;
    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        ngs.SetConnectState(info->u.connection_resumed.player, EPlayerConnectState::Running);
        break;
    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        ngs.SetConnectState(info->u.disconnected.player, EPlayerConnectState::Disconnected);
        break;
    case GGPO_EVENTCODE_TIMESYNC:
        Sleep(1000 * info->u.timesync.frames_ahead / 60);
        break;
    }
    return true;
}

