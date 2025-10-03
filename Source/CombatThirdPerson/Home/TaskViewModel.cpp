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

    // ���я������肳�������ꍇ�� Order �Ń\�[�g
    Rows.Sort([](const FTaskDefRow& A, const FTaskDefRow& B) {
        return A.Order < B.Order;
        });

    const FString TodayISO = FDateTime::Now().ToString(TEXT("%Y-%m-%d"));

    for (const FTaskDefRow* R : Rows)
    {
        if (!R) continue;

        // �����t�B���^�iDateISO����Ȃ�g��ɗL���h�j
        const bool bIsToday = (R->DateISO.IsEmpty() || R->DateISO == TodayISO);
        if (!bIsToday) continue;

        const int32 N = FMath::Max(1, R->RepeatCount);

        for (int32 i = 0; i < N; ++i)
        {
            FTaskItemVM Item;
            // �@ ��ӂ�ID�i�������ɑς���悤�A�Ԃ�t�^�j
            if (N == 1)
            {
                Item.TaskId = R->TaskId;                         // 1�{�Ȃ炻�̂܂�
                Item.DisplayName = R->DisplayName;               // �\���������̂܂�
            }
            else
            {
                // TaskId: "study_01", "study_02", ...
                Item.TaskId = *FString::Printf(TEXT("%s_%02d"), *R->TaskId.ToString(), i + 1);

                // �\����: "Study (1/2)" �̂悤��
                Item.DisplayName = FText::FromString(
                    FString::Printf(TEXT("%s (%d/%d)"), *R->DisplayName.ToString(), i + 1, N)
                );
            }

            // �A ���Ԃ�J�e�S���Ȃ�
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

    // �܂Ƃ߂�UI�ɒʒm
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

    OnStatsChanged.Broadcast(); // UI���f
}


void UTaskViewModel::ClaimTask(FName TaskId)
{
    if (FTaskItemVM* T = FindTask(TaskId))
    {
        // �󂯎������FFinished�i�󂯎��҂��j�̂�
        if (T->State != ETaskState::Finished) return;

        // �� ��������͊��� CompleteTask �Ɠ����g��V�{�X�g���[�N�h����
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

        // ��V�t�^
        Coin = ClampAdd(Coin, FMath::RoundToInt(T->CoinDelta * mul), T->CoinMin, T->CoinMax);
        Focus = ClampAdd(Focus, FMath::RoundToInt(T->FocusDelta * mul), T->FocusMin, T->FocusMax);
        Mood = FMath::Clamp(Mood + (T->MoodDelta * mul), T->MoodMin, T->MoodMax);
        Affection = ClampAdd(Affection, FMath::RoundToInt(T->AffectionDelta * mul), T->AffectionMin, T->AffectionMax);

        HeartValue = FMath::Clamp(HeartValue + 2.f, 0.f, 100.f);
        UpdateHeartFromStats();

        // �󂯎��ς�
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
            OnStatsChanged.Broadcast(); // �`�F�b�NUI�����̍X�V�ł�OK
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
        if (!C.bChecked) continue; // ���`�F�b�N��0

        // ��V�t�^�i�X�g���[�N�{���Ȃ��B�K�v�Ȃ珫���g���j
        Coin = ClampAddInt(Coin, C.CoinDelta, C.CoinMin, C.CoinMax);
        Focus = ClampAddInt(Focus, C.FocusDelta, C.FocusMin, C.FocusMax);
        Mood = FMath::Clamp(Mood + C.MoodDelta, C.MoodMin, C.MoodMax);
        Affection = ClampAddInt(Affection, C.AffectionDelta, C.AffectionMin, C.AffectionMax);

        bAnyClaim = true;
    }

    if (bAnyClaim)
    {
        UpdateHeartFromStats(); // ����Heart�X�V
    }

    // �����̂��߂ɑS���Z�b�g
    for (auto& C : TodayChecks) { C.bChecked = false; }

    if (bAnyClaim)
    {
        OnStatsChanged.Broadcast(); // �󂯎�蔽�f�{�`�F�b�N����
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
    OnStatsChanged.Broadcast(); // HUD ���V
}

void UTaskViewModel::UpdateHeartFromStats()
{
    // ��FMood(0..1)��40%�AFocus/100��35%�AAffection/100��25%
    const float mood = FMath::Clamp(Mood, 0.f, 1.f);
    const float focus01 = FMath::Clamp(Focus / 100.f, 0.f, 1.f);
    const float aff01 = FMath::Clamp(Affection / 100.f, 0.f, 1.f);

    const float composite01 = 0.40f * mood + 0.35f * focus01 + 0.25f * aff01;
    HeartValue = FMath::Clamp(composite01 * 100.f, 0.f, 100.f);
}

void UTaskViewModel::NaturalHeartTick(float DeltaSeconds)
{
    HeartAccumSec += DeltaSeconds;

    // 5������ +1�i��=���80�A��=���90�j
    if (HeartAccumSec >= 300.f) // 300s = 5min
    {
        const int Hour = FDateTime::Now().GetHour(); // ���[�J���O���OK
        const float Cap = (Hour >= 4 && Hour < 22) ? HeartDayCap : HeartNightCap;
        HeartValue = FMath::Min(Cap, HeartValue + 1.f);
        HeartAccumSec = 0.f;
        OnStatsChanged.Broadcast();
    }

    // ���u�Ŕ����i1������ -0.2�j
    bool bAnyRunning = false;
    for (const auto& T : TodayTasks) { if (T.State == ETaskState::Running) { bAnyRunning = true; break; } }
    static float DecayAccum = 0.f;
    DecayAccum += DeltaSeconds;
    if (!bAnyRunning && DecayAccum >= 60.f) // 1��
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

    Add("study_25", TEXT("Study�i25 min�j"), 25, TEXT("Study"));
    Add("clean_10", TEXT("Clean�i10 min�j"), 10, TEXT("Clean"));
    Add("workout_20", TEXT("Sport�i20 min�j"), 20, TEXT("Workout"));

    Coin = 120; Focus = 30; Mood = 0.6f; Affection = 120;

    OnStatsChanged.Broadcast(); // �����\���p
}

void UTaskViewModel::StartTask(FName TaskId)
{
    if (auto* T = FindTask(TaskId)) {
        if (T->State == ETaskState::Ready || T->State == ETaskState::Paused) {
            // �ǉ�: Streak�ɉ�����RemainingSeconds��Z�k
            if (T->StreakKey != NAME_None) {
                const auto& S = GetStreakRef(*T);
                const int32 tier = TierFromCount(S.Count);
                const float ease = TimeEase(tier);
                const float total = FMath::Max(60.f, T->BaseMinutes * 60.f);
                // Paused����̍ĊJ���͍��̎c����ێ��BReady����J�n���̂ݓK�p�B
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
                // ����(���T)�̕�����̓J�E���g�����u���i�D�݂�+0.2�����j
            }
            else {
                // �A��/�X�L�b�v����
                const bool isConsecutive = (nowKey - S.LastKey) == ((S.Period == EStreakPeriod::Weekly) ? 7 * 24 * 3600 : 24 * 3600);
                if (isConsecutive) {
                    S.Count += 1; S.LastKey = nowKey; S.GraceLeft = FMath::Max(S.GraceLeft, 1); // �i�߂���Œ�1�ɕ�������
                }
                else {
                    // ��������FGrace����
                    if (S.GraceLeft > 0) {
                        S.GraceLeft -= 1;
                        S.Count += 1; S.LastKey = nowKey;
                    }
                    else {
                        S.Count = 1; S.LastKey = nowKey; // ���Z�b�g
                    }
                }
            }
            S.Best = FMath::Max(S.Best, S.Count);
            mul = RewardMul(TierFromCount(S.Count));
        }

        auto ClampAdd = [](auto Value, auto Delta, auto MinV, auto MaxV) {
            using V = decltype(Value); return FMath::Clamp<V>(Value + Delta, MinV, MaxV);
            };

        // �� �����ɔ{�����|���Ĕ��f ��
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
    UpdateHeartFromStats();          // �D���x������Heart�ɔ��f�����
    OnStatsChanged.Broadcast();      // UI�֒ʒm�iResourceBar/Heart���X�V�j
}


void UTaskViewModel::Tick(float DeltaSeconds)
{
    bool bAnyProgress = false;   // �i���i�����X�V�Ȃǁj
    bool bAnyFinishedNow = false;   // �� ���̃t���[���� Finished �֑J�ڂ�����

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
                // �� �����F�󂯎��҂���
                T.State = ETaskState::Finished;
                T.Progress01 = 1.f;
                T.RemainingSeconds = 0.f;

                // �� ���̃t���[���� Finished �ɂȂ������Ƃ��L�^
                bAnyFinishedNow = true;
            }
        }
    }

    NaturalHeartTick(DeltaSeconds);

    // �� �D��x���FFinished ���o���t���[���͑���Broadcast�iUI���u���ɁuClaim�v�ցj
    if (bAnyFinishedNow)
    {
        OnStatsChanged.Broadcast();
        Accum = 0.f;               // �X���b�g���̘A�����΂�h��
        return;                    // �����ŕԂ��Ă�OK�B���������ꍇ�� return ���O��
    }

    // �ʏ�̐i����0.2s�Ԉ���
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
    SaveObj->TodayTasks = TodayTasks; // �i�s��Ԃ��ۑ��������ꍇ
    SaveObj->TodayChecks = TodayChecks; // �ǉ�

    SaveObj->LastDailyKey = LastDailyKey;
    SaveObj->LastWeeklyKey = LastWeeklyKey;

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

// --- Load (���݂���Γǂݍ��݁^�������DefaultTable���珉����) ---
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

                // �� �������ǉ��_�F�O��L�[�𕜌�
                LastDailyKey = SaveObj->LastDailyKey;
                LastWeeklyKey = SaveObj->LastWeeklyKey;

                OnStatsChanged.Broadcast();
                return true;    // �� �Z�[�u���畜���ł���
            }
        }
    }

    // �Z�[�u�������^���Ă��� �� �����̎d�g�݂ŏ�������
    LoadFromDataTable(DefaultTable, bAppend);

    // �� �V�K�쐬���̓L�[���g���݁h�ɍ��킹�Ă����i�N������̔��肪����j
    LastDailyKey = GetDailyKeyNow();
    LastWeeklyKey = GetWeeklyKeyNow();

    OnStatsChanged.Broadcast();
    return false; // �� ����l�Ő��������i=�V�K�j
}


void UTaskViewModel::GetExistingCountAndMaxIndex(const FName& BaseId, int32& OutCount, int32& OutMaxIndex) const
{
    const FString Base = BaseId.ToString();
    OutCount = 0;
    OutMaxIndex = 0;

    for (const FTaskItemVM& It : TodayTasks)
    {
        const FString Id = It.TaskId.ToString();

        // ���������K���F
        //  1���ڂ��uBase�v�����̉\���i���f�[�^�j���P�A
        if (Id == Base)
        {
            OutCount++;
            OutMaxIndex = FMath::Max(OutMaxIndex, 1);
            continue;
        }

        // �uBase_02�v�Ȃǂ̌`��
        const FString Prefix = Base + TEXT("_");
        if (Id.StartsWith(Prefix))
        {
            const FString Suffix = Id.Mid(Prefix.Len());
            // ���l�Ƃ��ĉ��߁iAtoi �͐����ȊO���� 0 ��Ԃ��j
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
    // TaskId �����i2���ȏ゠��ꍇ�� _NN ��t�^�j
    if (Total <= 1 && Index == 1)
    {
        Item.TaskId = R.TaskId; // 1�������Ȃ炻�̂܂�
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

    // ��V�E�㉺�����]�L�iDataTable �쓮�j
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
        // ����́g�����h�ɍ��킹��
        LastDailyKey = CurD;
    }
    else if (LastDailyKey != CurD) {
        HandleDailyReset();       // �� ���A�������֐؂�ւ�
        LastDailyKey = CurD;
    }

    const int64 CurW = GetWeeklyKeyNow();
    if (LastWeeklyKey == 0) {
        LastWeeklyKey = CurW;
    }
    else if (LastWeeklyKey != CurW) {
        // �T���̐������K�v�Ȃ炱����
        HandleWeeklyReset();      // �i�T�ʂ̏��/�J�[�h�X�V�Ȃǂ�������Ώȗ��j
        LastWeeklyKey = CurW;
    }

    // �ύX��UI��
    OnStatsChanged.Broadcast();

    // �C�ӁF�����ő��Z�[�u���Ă��悢�i�L�[�̓�������h�~�j
    // SaveToSlot(TEXT("TaskVM"), 0);
}

int64 UTaskViewModel::GetDailyKeyNow() const
{
    const FDateTime Now = FDateTime::Now(); // ���[�J��
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

    // 1) �X�g���[�N�̌��������i�����̂܂܁j
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

    // 2) �� ������ Finished �������󂯎��
    for (auto& T : TodayTasks)
    {
        if (T.State == ETaskState::Finished)
        {
            ClaimTask(T.TaskId);
        }
    }

    // 3) �� �S�^�X�N�𗂓��́g�Ē���h��
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

    //// 1) �T�X�g���[�N�̌���/�O���[�X����
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
    //                // �g��T�܂ł�OK�h�Ƃ��Čp���]�n���c��
    //                S.LastKey = weekKeyNow - kWeekSec;
    //            }
    //            else {
    //                S.Count = 0; // �f��
    //            }
    //        }
    //    }
    //}

    //// 2) �T�����Z�b�g�Ώۃ^�X�N��Ready�ցiWeekly�̂ݍĒ��퉻�j
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
        // ���������ƍő�C���f�b�N�X���m�F
        int32 Have = 0, MaxIdx = 0;
        GetExistingCountAndMaxIndex(R->TaskId, Have, MaxIdx);

        const int32 Target = (Count < 0) ? FMath::Max(1, R->RepeatCount) : Count;
        int32 Added = 0;

        // ������ 0 �� Target==1 �̂Ƃ��͂��̂܂� 1 ���ǉ�
        if (Have == 0 && Target == 1)
        {
            TodayTasks.Add(MakeItemFromRow(*R, 1, 1));
            Added++;
        }
        else
        {
            // �s������ _NN �Œǉ��i������ Base �P�̂ł��A2���ڈȍ~�� _02 ����j
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

    return 0; // �s������
}

int32 UTaskViewModel::MergeDailyChecksFromDataTable(UDataTable* Table)
{
    if (!Table) return 0;

    static const FString Ctx(TEXT("TaskVM MergeDailyChecks"));
    TArray<FDailyCheckRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    // ���Ɏ����Ă��� CheckId ���W����
    TSet<FName> Existing;
    for (const auto& C : TodayChecks)
    {
        Existing.Add(C.CheckId);
    }

    // ���я����~������� Order �ň��艻
    Rows.Sort([](const FDailyCheckRow& A, const FDailyCheckRow& B) {
        return A.Order < B.Order;
        });

    int32 Added = 0;
    for (const FDailyCheckRow* R : Rows)
    {
        if (!R) continue;
        if (R->CheckId.IsNone()) continue;
        if (Existing.Contains(R->CheckId)) continue; // ���ɂ��� �� �X�L�b�v

        FDailyCheckItemVM NewItem;
        NewItem.CheckId = R->CheckId;
        NewItem.DisplayName = R->DisplayName; // �t�B�[���h���̓v���W�F�N�g��`�ɍ��킹�Ă�������
        NewItem.Order = R->Order;
        NewItem.bChecked = false;          // �V�K�͖��`�F�b�N
        // �K�v�Ȃ��V��J�e�S�������]�L:
        // NewItem.Coin = R->Coin; ... �Ȃ�

        TodayChecks.Add(NewItem);
        Existing.Add(NewItem.CheckId);
        ++Added;
    }

    if (Added > 0)
    {
        OnStatsChanged.Broadcast(); // UI�ɒǉ���ʒm�i���������ōč\�z����܂��j
    }
    return Added;
}



int32 UTaskViewModel::MergeFromDataTable(UDataTable* Table)
{
    if (!Table) return 0;

    static const FString Ctx(TEXT("TaskVM Merge"));
    TArray<FTaskDefRow*> Rows;
    Table->GetAllRows(Ctx, Rows);

    // �\���������肳�������Ȃ� Order �\�[�g
    Rows.Sort([](const FTaskDefRow& A, const FTaskDefRow& B) { return A.Order < B.Order; });

    int32 TotalAdded = 0;
    for (const FTaskDefRow* R : Rows)
    {
        if (!R) continue;

        int32 Have = 0, MaxIdx = 0;
        GetExistingCountAndMaxIndex(R->TaskId, Have, MaxIdx);

        const int32 Target = FMath::Max(1, R->RepeatCount);
        if (Have >= Target) continue; // ��������Ă���

        // �s������ǉ�
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