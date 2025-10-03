#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataTable.h" // ← 追加
#include "TaskViewModel.generated.h"

class UTaskSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVMStatsChanged);

UENUM(BlueprintType)
enum class ETaskState : uint8 { Ready, Running, Paused, Finished, Done };
UENUM(BlueprintType)
enum class EStreakPeriod : uint8 { Daily, Weekly };

struct FStreakRuntime {
    int32   Count = 0, Best = 0, GraceLeft = 1;
    int64   LastKey = 0;
    EStreakPeriod Period = EStreakPeriod::Daily;
};

USTRUCT(BlueprintType)
struct FDailyCheckRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CheckId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Order = 0;

    // 報酬
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionDelta = 0;

    // 上下限（既存Taskと同じ運用）
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinMax = 999999;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusMax = 100;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodMax = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionMax = 99999;
};

// ▼ ランタイム表示用
USTRUCT(BlueprintType)
struct FDailyCheckItemVM
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CheckId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Order = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  bChecked = false;

    // 報酬と上下限
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMax = 999999;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMax = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMax = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMax = 99999;
};

// TaskViewModel.h（FTaskItemVM に追加）
USTRUCT(BlueprintType)
struct FTaskItemVM
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TaskId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BaseMinutes = 25;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText  CategoryText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETaskState State = ETaskState::Ready;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float   Progress01 = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float   RemainingSeconds = 0.f;

    // ▼ 追加：報酬（増減量）
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionDelta = 0;

    // ▼ 追加：上下限（Clamp 用）
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMax = 999999;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMax = 9999;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMax = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMax = 9999;

    // 並び順など既存があれば続く…
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Order = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName StreakKey;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EStreakPeriod StreakPeriod = EStreakPeriod::Daily;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GraceMax = 1;
};


// TaskViewModel.h

USTRUCT(BlueprintType)
struct FTaskDefRow : public FTableRowBase
{
    GENERATED_BODY()

    // --------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName TaskId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 BaseMinutes = 25;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText  CategoryText;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString DateISO;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 RepeatCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Order = 0;

    // 増減量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 AffectionDelta = 0;

    // 上下限
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 CoinMax = 999999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 FocusMax = 9999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") float MoodMax = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 AffectionMax = 9999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    FName StreakKey;                       // 例: "study", "walk"

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    EStreakPeriod StreakPeriod = EStreakPeriod::Daily;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    int32 GraceMax = 1;                    // 許容欠損回数（Dailyなら“1日サボりOK”）
};


UCLASS(BlueprintType, Blueprintable)
class COMBATTHIRDPERSON_API UTaskViewModel : public UObject
{
    GENERATED_BODY()
public:
    // 例：public: あたりに
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selected")
    FName SelectedCharacterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selected")
    FText SelectedCharacterName;

    UFUNCTION(BlueprintCallable, Category = "Selected")
    void SetSelectedCharacterMeta(FName InId, FText InName);

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FTaskItemVM> TodayTasks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FDailyCheckItemVM> TodayChecks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Coin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Focus = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Mood = 0.5f;   // 0..1
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Affection = 0;

    // --- Heart ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeartValue = 60.f;    // 0..100
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeartDayCap = 80.f;   // 昼の上限
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeartNightCap = 90.f; // 夜の上限

    UFUNCTION(BlueprintCallable) void UpdateHeartFromStats();               // 総合値(加重平均)
    void NaturalHeartTick(float DeltaSeconds);                              // 自然回復/微減
    float HeartAccumSec = 0.f;                                              // 内部積算


    UFUNCTION(BlueprintCallable) void InitMockData();
    UFUNCTION(BlueprintCallable) void StartTask(FName TaskId);
    UFUNCTION(BlueprintCallable) void PauseTask(FName TaskId);
    UFUNCTION(BlueprintCallable) void CompleteTask(FName TaskId);

    UFUNCTION(BlueprintCallable) void LoadFromDataTable(UDataTable* Table, bool bAppend = false);
    UFUNCTION(BlueprintCallable) void LoadDailyChecksFromDataTable(UDataTable* Table, bool bAppend = false);

    UFUNCTION(BlueprintCallable) void ClaimTask(FName TaskId);

    UFUNCTION(BlueprintCallable) void SetDailyCheck(FName CheckId, bool bChecked); // チェック／アンチェック操作（UIから呼ぶ）    
    void ClaimDailyChecksAtReset(); // ★ 04:00清算（HandleDailyReset内から呼ぶ）

    // UI更新用
    void Tick(float DeltaSeconds);

    // デリゲート（UIへ通知）
    UPROPERTY(BlueprintAssignable) FOnVMStatsChanged OnStatsChanged;

    // ▼ 追加：セーブ／ロードAPI（BPからも呼べるように）
    UFUNCTION(BlueprintCallable, Category = "Life|Save")
    bool SaveToSlot(const FString& SlotName = TEXT("TaskVM"), int32 UserIndex = 0) const;

    // DefaultTable は「セーブが無かった時の初期ロード元」
    UFUNCTION(BlueprintCallable, Category = "Life|Save")
    bool LoadFromSlot(UDataTable* DefaultTable, const FString& SlotName = TEXT("TaskVM"), int32 UserIndex = 0, bool bAppend = false);

    // これらも .cpp で定義（インラインにしない）
    void AddCoin(int32 Delta);
    void SetMood(float InMood);
    UFUNCTION(BlueprintCallable, Category="Life|Stats")
    void AddAffection(int32 Delta, int32 Min = 0, int32 Max = 9999);


    // DataTable 全体を見て「不足分だけ追加」する（追加数を返す）
    UFUNCTION(BlueprintCallable, Category = "Life|Task")
    int32 MergeFromDataTable(UDataTable* Table);

    // 指定 RowName だけを追加（Count<0 なら RepeatCount まで不足分を追加）
    UFUNCTION(BlueprintCallable, Category = "Life|Task")
    int32 AddTasksFromRow(UDataTable* Table, FName RowName, int32 Count = -1);

    UFUNCTION(BlueprintCallable, Category = "Life|Checks")
    int32 MergeDailyChecksFromDataTable(UDataTable* Table);

    // 起動時チェックで呼ぶ
    UFUNCTION() void EnsureFreshDailyWeeklyOnLaunch();

    // 04:00境界の当日キー（ローカル時刻基準：Asia/Taipei想定）
    int64 GetDailyKeyNow() const;
    // 月曜04:00起点の週キー
    int64 GetWeeklyKeyNow() const;

    // Save/Load時に詰め替えるだけのアクセサ
    FORCEINLINE int64 GetLastDailyKey()  const { return LastDailyKey; }
    FORCEINLINE int64 GetLastWeeklyKey() const { return LastWeeklyKey; }
    FORCEINLINE void  SetLastDailyKey(int64 V) { LastDailyKey = V; }
    FORCEINLINE void  SetLastWeeklyKey(int64 V) { LastWeeklyKey = V; }

    UFUNCTION() void HandleDailyReset();     // 既存を拡張済みでOK
    UFUNCTION() void HandleWeeklyReset();    // 週次が必要なら用意（なければスキップ可）

private:
    FTaskItemVM* FindTask(FName TaskId);

    // Broadcastを間引くための蓄積
    float Accum = 0.f;

    // 既存タスクのうち BaseId（= Row.TaskId）に一致する枚数と最大インデックスを調べる
    void GetExistingCountAndMaxIndex(const FName& BaseId, int32& OutCount, int32& OutMaxIndex) const;

    // 1枚分のVMアイテムを Row から生成（index と total で (1/3) のような表示に）
    FTaskItemVM MakeItemFromRow(const FTaskDefRow& R, int32 Index, int32 Total) const;

    /// 
    ///    /// streak
    /// 
    FStreakRuntime& GetStreakRef(const FTaskItemVM& Item);
    int32 TierFromCount(int32 Count) const;

    float RewardMul(int32 Tier) const { return 1.f + 0.1f * Tier; }    // 1.0 / 1.1 / 1.2 / 1.3
    float TimeEase(int32 Tier)  const { return FMath::Max(0.85f, 1.f - 0.05f * Tier); } // 1.00 / 0.95 / 0.90 / 0.85

    TMap<FName, FStreakRuntime> RuntimeStreaks;

    UPROPERTY() int64 LastDailyKey = 0;
    UPROPERTY() int64 LastWeeklyKey = 0;
};
