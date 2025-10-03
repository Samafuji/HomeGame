#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataTable.h" // �� �ǉ�
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

    // ��V
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionDelta = 0;

    // �㉺���i����Task�Ɠ����^�p�j
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CoinMax = 999999;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FocusMax = 100;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MoodMax = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AffectionMax = 99999;
};

// �� �����^�C���\���p
USTRUCT(BlueprintType)
struct FDailyCheckItemVM
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CheckId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Order = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  bChecked = false;

    // ��V�Ə㉺��
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

// TaskViewModel.h�iFTaskItemVM �ɒǉ��j
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

    // �� �ǉ��F��V�i�����ʁj
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionDelta = 0;

    // �� �ǉ��F�㉺���iClamp �p�j
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CoinMax = 999999;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FocusMax = 9999;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoodMax = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AffectionMax = 9999;

    // ���я��ȂǊ���������Α����c
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

    // ������
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 CoinDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 FocusDelta = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") float MoodDelta = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Reward") int32 AffectionDelta = 0;

    // �㉺��
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 CoinMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 CoinMax = 999999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 FocusMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 FocusMax = 9999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") float MoodMin = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") float MoodMax = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 AffectionMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Limit") int32 AffectionMax = 9999;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    FName StreakKey;                       // ��: "study", "walk"

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    EStreakPeriod StreakPeriod = EStreakPeriod::Daily;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Life|Streak")
    int32 GraceMax = 1;                    // ���e�����񐔁iDaily�Ȃ�g1���T�{��OK�h�j
};


UCLASS(BlueprintType, Blueprintable)
class COMBATTHIRDPERSON_API UTaskViewModel : public UObject
{
    GENERATED_BODY()
public:
    // ��Fpublic: �������
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeartDayCap = 80.f;   // ���̏��
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeartNightCap = 90.f; // ��̏��

    UFUNCTION(BlueprintCallable) void UpdateHeartFromStats();               // �����l(���d����)
    void NaturalHeartTick(float DeltaSeconds);                              // ���R��/����
    float HeartAccumSec = 0.f;                                              // �����ώZ


    UFUNCTION(BlueprintCallable) void InitMockData();
    UFUNCTION(BlueprintCallable) void StartTask(FName TaskId);
    UFUNCTION(BlueprintCallable) void PauseTask(FName TaskId);
    UFUNCTION(BlueprintCallable) void CompleteTask(FName TaskId);

    UFUNCTION(BlueprintCallable) void LoadFromDataTable(UDataTable* Table, bool bAppend = false);
    UFUNCTION(BlueprintCallable) void LoadDailyChecksFromDataTable(UDataTable* Table, bool bAppend = false);

    UFUNCTION(BlueprintCallable) void ClaimTask(FName TaskId);

    UFUNCTION(BlueprintCallable) void SetDailyCheck(FName CheckId, bool bChecked); // �`�F�b�N�^�A���`�F�b�N����iUI����Ăԁj    
    void ClaimDailyChecksAtReset(); // �� 04:00���Z�iHandleDailyReset������Ăԁj

    // UI�X�V�p
    void Tick(float DeltaSeconds);

    // �f���Q�[�g�iUI�֒ʒm�j
    UPROPERTY(BlueprintAssignable) FOnVMStatsChanged OnStatsChanged;

    // �� �ǉ��F�Z�[�u�^���[�hAPI�iBP������Ăׂ�悤�Ɂj
    UFUNCTION(BlueprintCallable, Category = "Life|Save")
    bool SaveToSlot(const FString& SlotName = TEXT("TaskVM"), int32 UserIndex = 0) const;

    // DefaultTable �́u�Z�[�u�������������̏������[�h���v
    UFUNCTION(BlueprintCallable, Category = "Life|Save")
    bool LoadFromSlot(UDataTable* DefaultTable, const FString& SlotName = TEXT("TaskVM"), int32 UserIndex = 0, bool bAppend = false);

    // ������ .cpp �Œ�`�i�C�����C���ɂ��Ȃ��j
    void AddCoin(int32 Delta);
    void SetMood(float InMood);
    UFUNCTION(BlueprintCallable, Category="Life|Stats")
    void AddAffection(int32 Delta, int32 Min = 0, int32 Max = 9999);


    // DataTable �S�̂����āu�s���������ǉ��v����i�ǉ�����Ԃ��j
    UFUNCTION(BlueprintCallable, Category = "Life|Task")
    int32 MergeFromDataTable(UDataTable* Table);

    // �w�� RowName ������ǉ��iCount<0 �Ȃ� RepeatCount �܂ŕs������ǉ��j
    UFUNCTION(BlueprintCallable, Category = "Life|Task")
    int32 AddTasksFromRow(UDataTable* Table, FName RowName, int32 Count = -1);

    UFUNCTION(BlueprintCallable, Category = "Life|Checks")
    int32 MergeDailyChecksFromDataTable(UDataTable* Table);

    // �N�����`�F�b�N�ŌĂ�
    UFUNCTION() void EnsureFreshDailyWeeklyOnLaunch();

    // 04:00���E�̓����L�[�i���[�J��������FAsia/Taipei�z��j
    int64 GetDailyKeyNow() const;
    // ���j04:00�N�_�̏T�L�[
    int64 GetWeeklyKeyNow() const;

    // Save/Load���ɋl�ߑւ��邾���̃A�N�Z�T
    FORCEINLINE int64 GetLastDailyKey()  const { return LastDailyKey; }
    FORCEINLINE int64 GetLastWeeklyKey() const { return LastWeeklyKey; }
    FORCEINLINE void  SetLastDailyKey(int64 V) { LastDailyKey = V; }
    FORCEINLINE void  SetLastWeeklyKey(int64 V) { LastWeeklyKey = V; }

    UFUNCTION() void HandleDailyReset();     // �������g���ς݂�OK
    UFUNCTION() void HandleWeeklyReset();    // �T�����K�v�Ȃ�p�Ӂi�Ȃ���΃X�L�b�v�j

private:
    FTaskItemVM* FindTask(FName TaskId);

    // Broadcast���Ԉ������߂̒~��
    float Accum = 0.f;

    // �����^�X�N�̂��� BaseId�i= Row.TaskId�j�Ɉ�v���閇���ƍő�C���f�b�N�X�𒲂ׂ�
    void GetExistingCountAndMaxIndex(const FName& BaseId, int32& OutCount, int32& OutMaxIndex) const;

    // 1������VM�A�C�e���� Row ���琶���iindex �� total �� (1/3) �̂悤�ȕ\���Ɂj
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
