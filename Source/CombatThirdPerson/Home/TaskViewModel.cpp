// Fill out your copyright notice in the Description page of Project Settings.

#include "Home/TaskViewModel.h"
#include "Kismet/GameplayStatics.h"
#include "Home/TaskSaveGame.h"

#include "Engine/DataTable.h"
#include "Containers/UnrealString.h"
#include "Misc/Char.h"
// TaskViewModel.cpp

void UTaskViewModel::LoadFromDataTable(UDataTable* Table, bool bAppend /*=false*/)
{
    if (!Table) return;
    if (!bAppend) TodayTasks.Empty();

    static const FString Ctx(TEXT("TaskVM Load"));
    TArray<FTaskDefRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    // 並び順を安定させたい場合は Order でソート
    Rows.Sort([](const FTaskDefRow& A, const FTaskDefRow& B) {
        return A.Order < B.Order;
        });

    const FString TodayISO = FDateTime::Now().ToString(TEXT("%Y-%m-%d"));

    for (const FTaskDefRow* R : Rows)
    {
        if (!R) continue;

        // 当日フィルタ（DateISOが空なら“常に有効”）
        const bool bIsToday = (R->DateISO.IsEmpty() || R->DateISO == TodayISO);
        if (!bIsToday) continue;

        const int32 N = FMath::Max(1, R->RepeatCount);

        for (int32 i = 0; i < N; ++i)
        {
            FTaskItemVM Item;
            // ① 一意なID（複数化に耐えるよう連番を付与）
            if (N == 1)
            {
                Item.TaskId = R->TaskId;                         // 1本ならそのまま
                Item.DisplayName = R->DisplayName;               // 表示名もそのまま
            }
            else
            {
                // TaskId: "study_01", "study_02", ...
                Item.TaskId = *FString::Printf(TEXT("%s_%02d"), *R->TaskId.ToString(), i + 1);

                // 表示名: "Study (1/2)" のように
                Item.DisplayName = FText::FromString(
                    FString::Printf(TEXT("%s (%d/%d)"), *R->DisplayName.ToString(), i + 1, N)
                );
            }

            // ② 時間やカテゴリなど
            Item.BaseMinutes = R->BaseMinutes;
            Item.RemainingSeconds = Item.BaseMinutes * 60.f;
            Item.State = ETaskState::Ready;
            Item.Progress01 = 0.f;
            Item.CategoryText = R->CategoryText;

            Item.CoinDelta = R->CoinDelta;
            Item.FocusDelta = R->FocusDelta;
            Item.MoodDelta = R->MoodDelta;
            Item.AffectionDelta = R->AffectionDelta;

            Item.CoinMin = R->CoinMin;
            Item.CoinMax = R->CoinMax;
            Item.FocusMin = R->FocusMin;
            Item.FocusMax = R->FocusMax;
            Item.MoodMin = R->MoodMin;
            Item.MoodMax = R->MoodMax;
            Item.AffectionMin = R->AffectionMin;
            Item.AffectionMax = R->AffectionMax;

            TodayTasks.Add(Item);
        }
    }

    // まとめてUIに通知
    OnStatsChanged.Broadcast();
}

void UTaskViewModel::LoadDailyChecksFromDataTable(UDataTable* Table, bool bAppend /*=false*/)
{
    if (!Table) return;
    if (!bAppend) TodayChecks.Empty();

    static const FString Ctx(TEXT("DailyChecks Load"));
    TArray<FDailyCheckRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    Rows.Sort([](const FDailyCheckRow& A, const FDailyCheckRow& B) { return A.Order < B.Order; });

    for (const FDailyCheckRow* R : Rows)
    {
        if (!R) continue;
        FDailyCheckItemVM It;
        It.CheckId = R->CheckId;
        It.DisplayName = R->DisplayName;
        It.Order = R->Order;
        It.bChecked = false;

        It.CoinDelta = R->CoinDelta;
        It.FocusDelta = R->FocusDelta;
        It.MoodDelta = R->MoodDelta;
        It.AffectionDelta = R->AffectionDelta;

        It.CoinMin = R->CoinMin;   It.CoinMax = R->CoinMax;
        It.FocusMin = R->FocusMin;  It.FocusMax = R->FocusMax;
        It.MoodMin = R->MoodMin;   It.MoodMax = R->MoodMax;
        It.AffectionMin = R->AffectionMin; It.AffectionMax = R->AffectionMax;

        TodayChecks.Add(It);
    }

    OnStatsChanged.Broadcast(); // UI反映
}


void UTaskViewModel::ClaimTask(FName TaskId)
{
    if (FTaskItemVM* T = FindTask(TaskId))
    {
        // 受け取り条件：Finished（受け取り待ち）のみ
        if (T->State != ETaskState::Finished) return;

        // ↓ ここからは既存 CompleteTask と同じ“報酬＋ストリーク”処理
        float mul = 1.f;
        if (T->StreakKey != NAME_None) {
            auto& S = GetStreakRef(*T);
            const int64 nowKey = (T->StreakPeriod == EStreakPeriod::Weekly) ? GetWeeklyKeyNow() : GetDailyKeyNow();
            if (S.LastKey == 0) { S.Count = 1; S.LastKey = nowKey; }
            else if (nowKey != S.LastKey) {
                const bool isConsecutive = (nowKey - S.LastKey) == ((S.Period == EStreakPeriod::Weekly) ? 7 * 24 * 3600 : 24 * 3600);
                if (isConsecutive) { S.Count += 1; S.LastKey = nowKey; S.GraceLeft = FMath::Max(S.GraceLeft, 1); }
                else {
                    if (S.GraceLeft > 0) { S.GraceLeft -= 1; S.Count += 1; S.LastKey = nowKey; }
                    else { S.Count = 1; S.LastKey = nowKey; }
                }
            }
            S.Best = FMath::Max(S.Best, S.Count);
            mul = RewardMul(TierFromCount(S.Count));
        }

        auto ClampAdd = [](auto Value, auto Delta, auto MinV, auto MaxV) {
            using V = decltype(Value); return FMath::Clamp<V>(Value + Delta, MinV, MaxV);
            };

        // 報酬付与
        Coin = ClampAdd(Coin, FMath::RoundToInt(T->CoinDelta * mul), T->CoinMin, T->CoinMax);
        Focus = ClampAdd(Focus, FMath::RoundToInt(T->FocusDelta * mul), T->FocusMin, T->FocusMax);
        Mood = FMath::Clamp(Mood + (T->MoodDelta * mul), T->MoodMin, T->MoodMax);
        Affection = ClampAdd(Affection, FMath::RoundToInt(T->AffectionDelta * mul), T->AffectionMin, T->AffectionMax);

        HeartValue = FMath::Clamp(HeartValue + 2.f, 0.f, 100.f);
        UpdateHeartFromStats();

        // 受け取り済み
        T->State = ETaskState::Done;

        OnStatsChanged.Broadcast();
    }
}

void UTaskViewModel::SetDailyCheck(FName CheckId, bool bChecked)
{
    for (auto& C : TodayChecks)
    {
        if (C.CheckId == CheckId)
        {
            C.bChecked = bChecked;
            OnStatsChanged.Broadcast(); // チェックUIだけの更新でもOK
            break;
        }
    }
}

static auto ClampAddInt = [](int32 Value, int32 Delta, int32 MinV, int32 MaxV)
    {
        return FMath::Clamp(Value + Delta, MinV, MaxV);
    };

void UTaskViewModel::ClaimDailyChecksAtReset()
{
    bool bAnyClaim = false;

    for (auto& C : TodayChecks)
    {
        if (!C.bChecked) continue; // 未チェックは0

        // 報酬付与（ストリーク倍率なし。必要なら将来拡張）
        Coin = ClampAddInt(Coin, C.CoinDelta, C.CoinMin, C.CoinMax);
        Focus = ClampAddInt(Focus, C.FocusDelta, C.FocusMin, C.FocusMax);
        Mood = FMath::Clamp(Mood + C.MoodDelta, C.MoodMin, C.MoodMax);
        Affection = ClampAddInt(Affection, C.AffectionDelta, C.AffectionMin, C.AffectionMax);

        bAnyClaim = true;
    }

    if (bAnyClaim)
    {
        UpdateHeartFromStats(); // 総合Heart更新
    }

    // 翌日のために全リセット
    for (auto& C : TodayChecks) { C.bChecked = false; }

    if (bAnyClaim)
    {
        OnStatsChanged.Broadcast(); // 受け取り反映＋チェック消去
    }
}




FTaskItemVM* UTaskViewModel::FindTask(FName TaskId)
{
    for (auto& T : TodayTasks)
    {
        if (T.TaskId == TaskId) return &T;
    }
    return nullptr;
}

void UTaskViewModel::SetSelectedCharacterMeta(FName InId, FText InName)
{
    SelectedCharacterId = InId;
    SelectedCharacterName = InName;
    OnStatsChanged.Broadcast(); // HUD 刷新
}

void UTaskViewModel::UpdateHeartFromStats()
{
    // 例：Mood(0..1)を40%、Focus/100を35%、Affection/100を25%
    const float mood = FMath::Clamp(Mood, 0.f, 1.f);
    const float focus01 = FMath::Clamp(Focus / 100.f, 0.f, 1.f);
    const float aff01 = FMath::Clamp(Affection / 100.f, 0.f, 1.f);

    const float composite01 = 0.40f * mood + 0.35f * focus01 + 0.25f * aff01;
    HeartValue = FMath::Clamp(composite01 * 100.f, 0.f, 100.f);
}

void UTaskViewModel::NaturalHeartTick(float DeltaSeconds)
{
    HeartAccumSec += DeltaSeconds;

    // 5分ごと +1（昼=上限80、夜=上限90）
    if (HeartAccumSec >= 300.f) // 300s = 5min
    {
        const int Hour = FDateTime::Now().GetHour(); // ローカル前提でOK
        const float Cap = (Hour >= 4 && Hour < 22) ? HeartDayCap : HeartNightCap;
        HeartValue = FMath::Min(Cap, HeartValue + 1.f);
        HeartAccumSec = 0.f;
        OnStatsChanged.Broadcast();
    }

    // 放置で微減（1分ごと -0.2）
    bool bAnyRunning = false;
    for (const auto& T : TodayTasks) { if (T.State == ETaskState::Running) { bAnyRunning = true; break; } }
    static float DecayAccum = 0.f;
    DecayAccum += DeltaSeconds;
    if (!bAnyRunning && DecayAccum >= 60.f) // 1分
    {
        HeartValue = FMath::Max(0.f, HeartValue - 0.2f);
        DecayAccum = 0.f;
        OnStatsChanged.Broadcast();
    }
}


void UTaskViewModel::InitMockData()
{
    TodayTasks.Empty();

    auto Add = [&](FName Id, const TCHAR* Name, int32 Min, const TCHAR* Cat)
        {
            FTaskItemVM T;
            T.TaskId = Id;
            T.DisplayName = FText::FromString(Name);
            T.BaseMinutes = Min;
            T.RemainingSeconds = Min * 60.f;
            T.State = ETaskState::Ready;
            T.CategoryText = FText::FromString(Cat);
            TodayTasks.Add(T);
        };

    Add("study_25", TEXT("Study（25 min）"), 25, TEXT("Study"));
    Add("clean_10", TEXT("Clean（10 min）"), 10, TEXT("Clean"));
    Add("workout_20", TEXT("Sport（20 min）"), 20, TEXT("Workout"));

    Coin = 120; Focus = 30; Mood = 0.6f; Affection = 120;

    OnStatsChanged.Broadcast(); // 初期表示用
}

void UTaskViewModel::StartTask(FName TaskId)
{
    if (auto* T = FindTask(TaskId)) {
        if (T->State == ETaskState::Ready || T->State == ETaskState::Paused) {
            // 追加: Streakに応じてRemainingSecondsを短縮
            if (T->StreakKey != NAME_None) {
                const auto& S = GetStreakRef(*T);
                const int32 tier = TierFromCount(S.Count);
                const float ease = TimeEase(tier);
                const float total = FMath::Max(60.f, T->BaseMinutes * 60.f);
                // Pausedからの再開時は今の残りを維持。Readyから開始時のみ適用。
                if (T->State == ETaskState::Ready) {
                    T->RemainingSeconds = total * ease;
                }
            }
            T->State = ETaskState::Running;
            OnStatsChanged.Broadcast();
        }
    }
}

void UTaskViewModel::PauseTask(FName TaskId)
{
    if (auto* T = FindTask(TaskId))
    {
        if (T->State == ETaskState::Running)
        {
            T->State = ETaskState::Paused;
            OnStatsChanged.Broadcast();

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Paused"));
            }
        }
    }
}

// TaskViewModel.cpp

void UTaskViewModel::CompleteTask(FName TaskId)
{
    if (FTaskItemVM* T = FindTask(TaskId)) {
        if (T->State == ETaskState::Done) return;
        T->State = ETaskState::Done;

        float mul = 1.f;
        if (T->StreakKey != NAME_None) {
            auto& S = GetStreakRef(*T);
            const int64 nowKey = (T->StreakPeriod == EStreakPeriod::Weekly) ? GetWeeklyKeyNow() : GetDailyKeyNow();

            if (S.LastKey == 0) {
                S.Count = 1; S.LastKey = nowKey;
            }
            else if (nowKey == S.LastKey) {
                // 同日(同週)の複数回はカウント据え置き（好みで+0.2等も可）
            }
            else {
                // 連続/スキップ判定
                const bool isConsecutive = (nowKey - S.LastKey) == ((S.Period == EStreakPeriod::Weekly) ? 7 * 24 * 3600 : 24 * 3600);
                if (isConsecutive) {
                    S.Count += 1; S.LastKey = nowKey; S.GraceLeft = FMath::Max(S.GraceLeft, 1); // 進めたら最低1に復活も可
                }
                else {
                    // 欠損あり：Grace消費
                    if (S.GraceLeft > 0) {
                        S.GraceLeft -= 1;
                        S.Count += 1; S.LastKey = nowKey;
                    }
                    else {
                        S.Count = 1; S.LastKey = nowKey; // リセット
                    }
                }
            }
            S.Best = FMath::Max(S.Best, S.Count);
            mul = RewardMul(TierFromCount(S.Count));
        }

        auto ClampAdd = [](auto Value, auto Delta, auto MinV, auto MaxV) {
            using V = decltype(Value); return FMath::Clamp<V>(Value + Delta, MinV, MaxV);
            };

        // ★ ここに倍率を掛けて反映 ★
        Coin = ClampAdd(Coin, FMath::RoundToInt(T->CoinDelta * mul), T->CoinMin, T->CoinMax);
        Focus = ClampAdd(Focus, FMath::RoundToInt(T->FocusDelta * mul), T->FocusMin, T->FocusMax);
        Mood = FMath::Clamp(Mood + (T->MoodDelta * mul), T->MoodMin, T->MoodMax);
        Affection = ClampAdd(Affection, FMath::RoundToInt(T->AffectionDelta * mul), T->AffectionMin, T->AffectionMax);

        HeartValue = FMath::Clamp(HeartValue + 2.f, 0.f, 100.f);
        UpdateHeartFromStats();
        OnStatsChanged.Broadcast();
    }
}


void UTaskViewModel::AddCoin(int32 Delta)
{
    Coin += Delta;
    OnStatsChanged.Broadcast();
}

void UTaskViewModel::SetMood(float InMood)
{
    Mood = FMath::Clamp(InMood, 0.f, 1.f);
    OnStatsChanged.Broadcast();
}

void UTaskViewModel::AddAffection(int32 Delta, int32 Min, int32 Max)
{
    Affection = FMath::Clamp(Affection + Delta, Min, Max);
    UpdateHeartFromStats();          // 好感度が総合Heartに反映される
    OnStatsChanged.Broadcast();      // UIへ通知（ResourceBar/Heartが更新）
}


void UTaskViewModel::Tick(float DeltaSeconds)
{
    bool bAnyProgress = false;   // 進捗（割合更新など）
    bool bAnyFinishedNow = false;   // ← このフレームで Finished へ遷移したか

    for (auto& T : TodayTasks)
    {
        if (T.State == ETaskState::Running)
        {
            T.RemainingSeconds = FMath::Max(0.f, T.RemainingSeconds - DeltaSeconds);
            const float Total = FMath::Max(1.f, T.BaseMinutes * 60.f);
            T.Progress01 = 1.f - (T.RemainingSeconds / Total);
            bAnyProgress = true;

            if (T.RemainingSeconds <= 0.01f)
            {
                // ★ 完了：受け取り待ちへ
                T.State = ETaskState::Finished;
                T.Progress01 = 1.f;
                T.RemainingSeconds = 0.f;

                // ★ このフレームで Finished になったことを記録
                bAnyFinishedNow = true;
            }
        }
    }

    NaturalHeartTick(DeltaSeconds);

    // ★ 優先度高：Finished が出たフレームは即時Broadcast（UIを瞬時に「Claim」へ）
    if (bAnyFinishedNow)
    {
        OnStatsChanged.Broadcast();
        Accum = 0.f;               // スロットルの連続発火を防ぐ
        return;                    // ここで返してもOK。続けたい場合は return を外す
    }

    // 通常の進捗は0.2s間引き
    Accum += DeltaSeconds;
    if (bAnyProgress && Accum > 0.2f)
    {
        OnStatsChanged.Broadcast();
        Accum = 0.f;
    }
}


// --- Save ---
bool UTaskViewModel::SaveToSlot(const FString& SlotName, int32 UserIndex) const
{
    UTaskSaveGame* SaveObj = Cast<UTaskSaveGame>(UGameplayStatics::CreateSaveGameObject(UTaskSaveGame::StaticClass()));
    if (!SaveObj) return false;

    SaveObj->Version = 1;
    SaveObj->Coin = Coin;
    SaveObj->Focus = Focus;
    SaveObj->Mood = Mood;
    SaveObj->Affection = Affection;
    SaveObj->TodayTasks = TodayTasks; // 進行状態も保存したい場合
    SaveObj->TodayChecks = TodayChecks; // 追加

    SaveObj->LastDailyKey = LastDailyKey;
    SaveObj->LastWeeklyKey = LastWeeklyKey;

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

// --- Load (存在すれば読み込み／無ければDefaultTableから初期化) ---
bool UTaskViewModel::LoadFromSlot(UDataTable* DefaultTable, const FString& SlotName, int32 UserIndex, bool bAppend)
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex))
        {
            if (UTaskSaveGame* SaveObj = Cast<UTaskSaveGame>(Base))
            {
                Coin = SaveObj->Coin;
                Focus = SaveObj->Focus;
                Mood = SaveObj->Mood;
                Affection = SaveObj->Affection;

                if (!bAppend) TodayTasks.Empty();
                TodayTasks = SaveObj->TodayTasks;
                TodayChecks = SaveObj->TodayChecks;

                // ★ ここが追加点：前回キーを復元
                LastDailyKey = SaveObj->LastDailyKey;
                LastWeeklyKey = SaveObj->LastWeeklyKey;

                OnStatsChanged.Broadcast();
                return true;    // ← セーブから復元できた
            }
        }
    }

    // セーブが無い／壊れている → 既存の仕組みで初期生成
    LoadFromDataTable(DefaultTable, bAppend);

    // ★ 新規作成時はキーを“現在”に合わせておく（起動直後の判定が安定）
    LastDailyKey = GetDailyKeyNow();
    LastWeeklyKey = GetWeeklyKeyNow();

    OnStatsChanged.Broadcast();
    return false; // ← 既定値で生成した（=新規）
}


void UTaskViewModel::GetExistingCountAndMaxIndex(const FName& BaseId, int32& OutCount, int32& OutMaxIndex) const
{
    const FString Base = BaseId.ToString();
    OutCount = 0;
    OutMaxIndex = 0;

    for (const FTaskItemVM& It : TodayTasks)
    {
        const FString Id = It.TaskId.ToString();

        // 既存命名規則：
        //  1枚目が「Base」だけの可能性（旧データ）もケア
        if (Id == Base)
        {
            OutCount++;
            OutMaxIndex = FMath::Max(OutMaxIndex, 1);
            continue;
        }

        // 「Base_02」などの形式
        const FString Prefix = Base + TEXT("_");
        if (Id.StartsWith(Prefix))
        {
            const FString Suffix = Id.Mid(Prefix.Len());
            // 数値として解釈（Atoi は数字以外だと 0 を返す）
            const int32 Num = FCString::Atoi(*Suffix);
            if (Num > 0)
            {
                OutCount++;
                OutMaxIndex = FMath::Max(OutMaxIndex, Num);
            }
        }
    }
}


FTaskItemVM UTaskViewModel::MakeItemFromRow(const FTaskDefRow& R, int32 Index, int32 Total) const
{
    FTaskItemVM Item;
    // TaskId 命名（2枚以上ある場合は _NN を付与）
    if (Total <= 1 && Index == 1)
    {
        Item.TaskId = R.TaskId; // 1枚だけならそのまま
        Item.DisplayName = R.DisplayName;
    }
    else
    {
        Item.TaskId = *FString::Printf(TEXT("%s_%02d"), *R.TaskId.ToString(), Index);
        Item.DisplayName = FText::FromString(
            FString::Printf(TEXT("%s (%d/%d)"), *R.DisplayName.ToString(), Index, Total));
    }

    Item.BaseMinutes = R.BaseMinutes;
    Item.RemainingSeconds = R.BaseMinutes * 60.f;
    Item.State = ETaskState::Ready;
    Item.Progress01 = 0.f;
    Item.CategoryText = R.CategoryText;
    Item.Order = R.Order;

    // 報酬・上下限も転記（DataTable 駆動）
    Item.CoinDelta = R.CoinDelta;
    Item.FocusDelta = R.FocusDelta;
    Item.MoodDelta = R.MoodDelta;
    Item.AffectionDelta = R.AffectionDelta;

    Item.CoinMin = R.CoinMin;
    Item.CoinMax = R.CoinMax;
    Item.FocusMin = R.FocusMin;
    Item.FocusMax = R.FocusMax;
    Item.MoodMin = R.MoodMin;
    Item.MoodMax = R.MoodMax;
    Item.AffectionMin = R.AffectionMin;
    Item.AffectionMax = R.AffectionMax;

    return Item;
}

void UTaskViewModel::EnsureFreshDailyWeeklyOnLaunch()
{
    const int64 CurD = GetDailyKeyNow();
    if (LastDailyKey == 0) {
        // 初回は“今日”に合わせる
        LastDailyKey = CurD;
    }
    else if (LastDailyKey != CurD) {
        HandleDailyReset();       // ← 即、今日分へ切り替え
        LastDailyKey = CurD;
    }

    const int64 CurW = GetWeeklyKeyNow();
    if (LastWeeklyKey == 0) {
        LastWeeklyKey = CurW;
    }
    else if (LastWeeklyKey != CurW) {
        // 週次の整理が必要ならここで
        HandleWeeklyReset();      // （週別の上限/カード更新などが無ければ省略可）
        LastWeeklyKey = CurW;
    }

    // 変更をUIへ
    OnStatsChanged.Broadcast();

    // 任意：ここで即セーブしてもよい（キーの同期ずれ防止）
    // SaveToSlot(TEXT("TaskVM"), 0);
}

int64 UTaskViewModel::GetDailyKeyNow() const
{
    const FDateTime Now = FDateTime::Now(); // ローカル
    const int32 R = 4; // 04:00
    FDateTime Day4(Now.GetYear(), Now.GetMonth(), Now.GetDay(), R, 0, 0);
    if (Now.GetHour() < R) { Day4 -= FTimespan(1, 0, 0, 0); }
    return Day4.ToUnixTimestamp();
}

int64 UTaskViewModel::GetWeeklyKeyNow() const
{
    const FDateTime Now = FDateTime::Now();
    const int32 R = 4;
    FDateTime Day4(Now.GetYear(), Now.GetMonth(), Now.GetDay(), R, 0, 0);
    if (Now.GetHour() < R) { Day4 -= FTimespan(1, 0, 0, 0); }
    const int32 W = (int32)Day4.GetDayOfWeek(); // 0=Sun..6=Sat
    const int32 DaysFromMonday = (W == 0) ? 6 : (W - 1);
    const FDateTime Monday4 = Day4 - FTimespan(DaysFromMonday, 0, 0, 0);
    return Monday4.ToUnixTimestamp();
}
FStreakRuntime& UTaskViewModel::GetStreakRef(const FTaskItemVM& Item)
{
    const FName Key = Item.StreakKey;
    FStreakRuntime* Found = RuntimeStreaks.Find(Key);
    if (!Found) {
        FStreakRuntime NewS; NewS.GraceLeft = Item.GraceMax; NewS.Period = Item.StreakPeriod;
        Found = &RuntimeStreaks.Add(Key, NewS);
    }
    return *Found;
}

int32 UTaskViewModel::TierFromCount(int32 Count) const
{
    if (Count >= 30) return 3;
    if (Count >= 14) return 2;
    if (Count >= 7)  return 1;
    return 0;
}

void UTaskViewModel::HandleDailyReset() {
    const int64 todayKey = GetDailyKeyNow();
    ClaimDailyChecksAtReset();

    // 1) ストリークの欠損処理（既存のまま）
    for (auto& KV : RuntimeStreaks) {
        auto& S = KV.Value;
        if (S.Period != EStreakPeriod::Daily) continue;
        if (S.LastKey == 0) continue;

        const int64 missed = (todayKey - S.LastKey) / (24 * 3600);
        if (missed >= 1) {
            if (S.GraceLeft > 0) { S.GraceLeft -= 1; S.LastKey = todayKey - (24 * 3600); }
            else { S.Count = 0; }
        }
    }

    // 2) ★ 未受取の Finished を自動受け取り
    for (auto& T : TodayTasks)
    {
        if (T.State == ETaskState::Finished)
        {
            ClaimTask(T.TaskId);
        }
    }

    // 3) ★ 全タスクを翌日の“再挑戦可”へ
    for (auto& T : TodayTasks)
    {
        T.State = ETaskState::Ready;
        T.Progress01 = 0.f;
        T.RemainingSeconds = FMath::Max(60.f, T.BaseMinutes * 60.f);
    }

    OnStatsChanged.Broadcast();
}

void UTaskViewModel::HandleWeeklyReset()
{
    //const int64 weekKeyNow = GetWeeklyKeyNow();
    //const int64 kWeekSec = 7 * 24 * 3600;

    //// 1) 週ストリークの欠損/グレース処理
    //for (auto& KV : RuntimeStreaks)
    //{
    //    auto& S = KV.Value;
    //    if (S.Period != EStreakPeriod::Weekly) continue;
    //    if (S.LastKey == 0) continue;

    //    const int64 diff = weekKeyNow - S.LastKey;
    //    if (diff >= kWeekSec) {
    //        const int32 missedWeeks = static_cast<int32>(diff / kWeekSec);
    //        if (missedWeeks >= 1) {
    //            if (S.GraceLeft > 0) {
    //                S.GraceLeft -= 1;
    //                // “先週まではOK”として継続余地を残す
    //                S.LastKey = weekKeyNow - kWeekSec;
    //            }
    //            else {
    //                S.Count = 0; // 断絶
    //            }
    //        }
    //    }
    //}

    //// 2) 週次リセット対象タスクをReadyへ（Weeklyのみ再挑戦化）
    //for (auto& T : TodayTasks)
    //{
    //    if (T.StreakPeriod == EStreakPeriod::Weekly) {
    //        T.State = ETaskState::Ready;
    //        T.Progress01 = 0.f;
    //        T.RemainingSeconds = FMath::Max(60.f, T.BaseMinutes * 60.f);
    //    }
    //}

    //OnStatsChanged.Broadcast();
}

int32 UTaskViewModel::AddTasksFromRow(UDataTable* Table, FName RowName, int32 Count /*= -1*/)
{
    if (!Table) return 0;

    static const FString Ctx(TEXT("TaskVM AddFromRow"));
    if (const FTaskDefRow* R = Table->FindRow<FTaskDefRow>(RowName, Ctx))
    {
        // 既存枚数と最大インデックスを確認
        int32 Have = 0, MaxIdx = 0;
        GetExistingCountAndMaxIndex(R->TaskId, Have, MaxIdx);

        const int32 Target = (Count < 0) ? FMath::Max(1, R->RepeatCount) : Count;
        int32 Added = 0;

        // 既存が 0 で Target==1 のときはそのまま 1 枚追加
        if (Have == 0 && Target == 1)
        {
            TodayTasks.Add(MakeItemFromRow(*R, 1, 1));
            Added++;
        }
        else
        {
            // 不足分を _NN で追加（既存が Base 単体でも、2枚目以降は _02 から）
            for (int32 idx = Have + 1; idx <= Target; ++idx)
            {
                const int32 UseIndex = (MaxIdx == 0 ? idx : MaxIdx + (idx - Have));
                TodayTasks.Add(MakeItemFromRow(*R, UseIndex, Target));
                Added++;
            }
        }

        if (Added > 0)
        {
            OnStatsChanged.Broadcast();
        }
        return Added;
    }

    return 0; // 行が無い
}

int32 UTaskViewModel::MergeDailyChecksFromDataTable(UDataTable* Table)
{
    if (!Table) return 0;

    static const FString Ctx(TEXT("TaskVM MergeDailyChecks"));
    TArray<FDailyCheckRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    // 既に持っている CheckId を集合化
    TSet<FName> Existing;
    for (const auto& C : TodayChecks)
    {
        Existing.Add(C.CheckId);
    }

    // 並び順が欲しければ Order で安定化
    Rows.Sort([](const FDailyCheckRow& A, const FDailyCheckRow& B) {
        return A.Order < B.Order;
        });

    int32 Added = 0;
    for (const FDailyCheckRow* R : Rows)
    {
        if (!R) continue;
        if (R->CheckId.IsNone()) continue;
        if (Existing.Contains(R->CheckId)) continue; // 既にある → スキップ

        FDailyCheckItemVM NewItem;
        NewItem.CheckId = R->CheckId;
        NewItem.DisplayName = R->DisplayName; // フィールド名はプロジェクト定義に合わせてください
        NewItem.Order = R->Order;
        NewItem.bChecked = false;          // 新規は未チェック
        // 必要なら報酬やカテゴリ等も転記:
        // NewItem.Coin = R->Coin; ... など

        TodayChecks.Add(NewItem);
        Existing.Add(NewItem.CheckId);
        ++Added;
    }

    if (Added > 0)
    {
        OnStatsChanged.Broadcast(); // UIに追加を通知（件数差分で再構築されます）
    }
    return Added;
}



int32 UTaskViewModel::MergeFromDataTable(UDataTable* Table)
{
    if (!Table) return 0;

    static const FString Ctx(TEXT("TaskVM Merge"));
    TArray<FTaskDefRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    // 表示順を安定させたいなら Order ソート
    Rows.Sort([](const FTaskDefRow& A, const FTaskDefRow& B) { return A.Order < B.Order; });

    int32 TotalAdded = 0;
    for (const FTaskDefRow* R : Rows)
    {
        if (!R) continue;

        int32 Have = 0, MaxIdx = 0;
        GetExistingCountAndMaxIndex(R->TaskId, Have, MaxIdx);

        const int32 Target = FMath::Max(1, R->RepeatCount);
        if (Have >= Target) continue; // もう足りている

        // 不足分を追加
        if (Have == 0 && Target == 1)
        {
            TodayTasks.Add(MakeItemFromRow(*R, 1, 1));
            TotalAdded++;
        }
        else
        {
            for (int32 idx = Have + 1; idx <= Target; ++idx)
            {
                const int32 UseIndex = (MaxIdx == 0 ? idx : MaxIdx + (idx - Have));
                TodayTasks.Add(MakeItemFromRow(*R, UseIndex, Target));
                TotalAdded++;
            }
        }
    }

    if (TotalAdded > 0)
    {
        OnStatsChanged.Broadcast();
    }
    return TotalAdded;
}