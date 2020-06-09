// Copyright 2020 BwdYeti.

#include "VWGameStateBase.h"
#include "VectorWarPlayerController.h"
#include "include/ggponet.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#define ARRAYSIZE(a) sizeof(a) / sizeof(a[0])
#define FRAME_RATE 60
#define ONE_FRAME (1.0f / FRAME_RATE)

void AVWGameStateBase::BeginPlay()
{
    Super::BeginPlay();

    bSessionStarted = TryStartSinglePlayerGGPOSession();
    // xxx: bSessionStarted = TryStartGGPOPlayerSession(7000, 4,
    //    { L"local", L"127.0.0.1:7001", L"127.0.0.1:7002", L"127.0.0.1:7003" });

    if (bSessionStarted)
    {
        OnSessionStarted();
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
        int32 Input = GetLocalInputs();
        VectorWarHost::VectorWar_RunFrame(Input);
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

bool AVWGameStateBase::TryStartSinglePlayerGGPOSession()
{
    // Get port
    uint16 LocalPort = 7000;
    // Get number of players
    int32 NumPlayers = 1;

    return TryStartGGPOPlayerSession(LocalPort, NumPlayers, { L"local" });
}

bool AVWGameStateBase::TryStartGGPOPlayerSession(
    const uint16 LocalPort,
    const int32 NumPlayers,
    TArray<wchar_t*> PlayerParameters)
{
    int32 Offset = 0;
    wchar_t WideIpBuffer[128];
    uint32 WideIpBufferSize = (uint32)ARRAYSIZE(WideIpBuffer);

    GGPOPlayer Players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];

    if (NumPlayers > PlayerParameters.Num())
        return false;

    int32 i;
    for (i = 0; i < NumPlayers; i++) {
        const wchar_t* Arg = PlayerParameters[Offset++];

        Players[i].size = sizeof(Players[i]);
        Players[i].player_num = i + 1;
        if (!_wcsicmp(Arg, L"local")) {
            Players[i].type = EGGPOPlayerType::LOCAL;
            continue;
        }

        Players[i].type = EGGPOPlayerType::REMOTE;
        if (swscanf_s(Arg, L"%[^:]:%hd", WideIpBuffer, WideIpBufferSize, &Players[i].u.remote.port) != 2) {
            return false;
        }
        wcstombs_s(nullptr, Players[i].u.remote.ip_address, ARRAYSIZE(Players[i].u.remote.ip_address), WideIpBuffer, _TRUNCATE);
    }
    // these are spectators...
    int32 NumSpectators = 0;
    while (Offset < PlayerParameters.Num()) {
        Players[i].type = EGGPOPlayerType::SPECTATOR;
        if (swscanf_s(PlayerParameters[Offset++], L"%[^:]:%hd", WideIpBuffer, WideIpBufferSize, &Players[i].u.remote.port) != 2) {
            return false;
        }
        wcstombs_s(nullptr, Players[i].u.remote.ip_address, ARRAYSIZE(Players[i].u.remote.ip_address), WideIpBuffer, _TRUNCATE);
        i++;
        NumSpectators++;
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
    AVectorWarPlayerController* Controller = Cast<AVectorWarPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (Controller)
    {
        return Controller->GetVectorWarInput();
    }
    return 0;
}
