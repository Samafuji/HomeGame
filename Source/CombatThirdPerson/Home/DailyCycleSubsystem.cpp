// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/DailyCycleSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDailyCycleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 初期キー（04:00区切り）
    LastDailyResetKey = ComputeCurrentResetKey();
    StartTimers();
}

void UDailyCycleSubsystem::Deinitialize()
{
    StopTimers();
    Super::Deinitialize();
}

void UDailyCycleSubsystem::StartTimers()
{
    if (!GetWorld()) return;

    // 5分ティック
    GetWorld()->GetTimerManager().SetTimer(
        RecoveryTimer, this, &UDailyCycleSubsystem::TickRecovery,
        FMath::Max(60, RecoveryIntervalSec), true);

    // 1分ごとに日次チェック
    GetWorld()->GetTimerManager().SetTimer(
        MinuteTimer, this, &UDailyCycleSubsystem::TickMinute,
        60.f, true);
}

void UDailyCycleSubsystem::StopTimers()
{
    if (!GetWorld()) return;
    GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
    GetWorld()->GetTimerManager().ClearTimer(MinuteTimer);
}

void UDailyCycleSubsystem::TickRecovery()
{
    OnRecoveryTick.Broadcast(CurrentHeartCap());
}

void UDailyCycleSubsystem::TickMinute()
{
    const FDateTime KeyNow = ComputeCurrentResetKey();
    if (KeyNow > LastDailyResetKey)
    {
        LastDailyResetKey = KeyNow;
        OnDailyReset.Broadcast();
    }
}

FDateTime UDailyCycleSubsystem::ComputeCurrentResetKey() const
{
    const FDateTime Now = FDateTime::Now(); // ローカル
    FDateTime Key(Now.GetYear(), Now.GetMonth(), Now.GetDay(), DailyResetHourLocal, 0, 0);
    if (Now.GetHour() < DailyResetHourLocal)
    {
        Key -= FTimespan(1, 0, 0, 0); // 前日04:00キー
    }
    return Key;
}

bool UDailyCycleSubsystem::IsNightNowLocal() const
{
    const int32 H = FDateTime::Now().GetHour();
    return (H >= 22 || H < 4); // 必要なら調整
}

int32 UDailyCycleSubsystem::CurrentHeartCap() const
{
    return IsNightNowLocal() ? NightHeartCap : DaytimeHeartCap;
}