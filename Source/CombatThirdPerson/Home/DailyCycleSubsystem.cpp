// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/DailyCycleSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDailyCycleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // �����L�[�i04:00��؂�j
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

    // 5���e�B�b�N
    GetWorld()->GetTimerManager().SetTimer(
        RecoveryTimer, this, &UDailyCycleSubsystem::TickRecovery,
        FMath::Max(60, RecoveryIntervalSec), true);

    // 1�����Ƃɓ����`�F�b�N
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
    const FDateTime Now = FDateTime::Now(); // ���[�J��
    FDateTime Key(Now.GetYear(), Now.GetMonth(), Now.GetDay(), DailyResetHourLocal, 0, 0);
    if (Now.GetHour() < DailyResetHourLocal)
    {
        Key -= FTimespan(1, 0, 0, 0); // �O��04:00�L�[
    }
    return Key;
}

bool UDailyCycleSubsystem::IsNightNowLocal() const
{
    const int32 H = FDateTime::Now().GetHour();
    return (H >= 22 || H < 4); // �K�v�Ȃ璲��
}

int32 UDailyCycleSubsystem::CurrentHeartCap() const
{
    return IsNightNowLocal() ? NightHeartCap : DaytimeHeartCap;
}