// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DailyCycleSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecoveryTick, int32, HeartCap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyReset);

/**
 * 
 */
UCLASS()
class COMBATTHIRDPERSON_API UDailyCycleSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    // �S�̋K���i���f�֐S�j
    UPROPERTY(EditAnywhere, Category = "Cycle") int32 RecoveryIntervalSec = 300; // 5��
    UPROPERTY(EditAnywhere, Category = "Cycle", meta = (ClampMin = 0, ClampMax = 23)) int32 DailyResetHourLocal = 4; // 04:00
    UPROPERTY(EditAnywhere, Category = "Cycle", meta = (ClampMin = 0, ClampMax = 100)) int32 DaytimeHeartCap = 80;
    UPROPERTY(EditAnywhere, Category = "Cycle", meta = (ClampMin = 0, ClampMax = 100)) int32 NightHeartCap = 90;

    UPROPERTY(BlueprintAssignable) FOnRecoveryTick OnRecoveryTick;
    UPROPERTY(BlueprintAssignable) FOnDailyReset   OnDailyReset;

    // �֋X�֐�
    UFUNCTION(BlueprintPure, Category = "Cycle") bool IsNightNowLocal() const;

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RecoveryTimer;
    FTimerHandle MinuteTimer;
    FDateTime    LastDailyResetKey;

    void StartTimers();
    void StopTimers();

    void TickRecovery();     // 5������
    void TickMinute();       // 1�����Ƃɓ������Ď�
    FDateTime ComputeCurrentResetKey() const;
    int32     CurrentHeartCap() const;
};