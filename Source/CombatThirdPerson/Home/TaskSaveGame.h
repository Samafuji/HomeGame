// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Home/TaskViewModel.h"
#include "TaskSaveGame.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCharacterStatusSG
{
    GENERATED_BODY()
    UPROPERTY() int32 Affection = 50;
    UPROPERTY() int32 Mood = 60;
    UPROPERTY() int32 Energy = 60;
    UPROPERTY() int32 Focus = 40;
    UPROPERTY() int32 Coin = 0;
    UPROPERTY() int32 Heart = 50;
};

USTRUCT()
struct FStreakSG {
    GENERATED_BODY()
    UPROPERTY() int32   Count = 0;
    UPROPERTY() int32   Best = 0;
    UPROPERTY() int64   LastKey = 0;     // 日次: 04:00キー / 週次: 週キー
    UPROPERTY() int32   GraceLeft = 1;
    UPROPERTY() uint8   Period = (uint8)EStreakPeriod::Daily;
};

UCLASS()
class COMBATTHIRDPERSON_API UTaskSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    // 既存のタスク保存に加えて：
    UPROPERTY() TMap<FName, FCharacterStatusSG> CharacterStatuses;

    UPROPERTY() int32 Version = 1;

    // リソース
    UPROPERTY() int32 Coin = 0;
    UPROPERTY() int32 Focus = 0;
    UPROPERTY() float Mood = 0.f;
    UPROPERTY() int32 Affection = 0;

    // 進行中のタスク一覧（FTaskItemVM は USTRUCT + UPROPERTY なのでそのまま保存可能）
    UPROPERTY() TArray<FTaskItemVM> TodayTasks;
    UPROPERTY() TArray<FDailyCheckItemVM> TodayChecks; // ← VMと同型

    UPROPERTY() TMap<FName, FStreakSG> Streaks;

    UPROPERTY() int64 LastDailyKey = 0;
    UPROPERTY() int64 LastWeeklyKey = 0;
};