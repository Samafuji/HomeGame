// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/HUD/HUDRootWidget.h"
#include "Home/TaskViewModel.h"
#include "TaskVMConsumer.h" // IF
#include "HeartWidget.h"
#include "ResourceBarWidget.h"
#include "TodayTabWidget.h"
#include "CharacterInteraction/FirstPlayerController.h"
#include "Input/Events.h"

#include "Components/SizeBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"      // ★Homeページ自動生成用
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Blueprint/WidgetTree.h"

void UHUDRootWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!*ResourceBarClass) ResourceBarClass = UResourceBarWidget::StaticClass();
    if (!*TodayTabClass)    TodayTabClass = UTodayTabWidget::StaticClass();
    if (!*HeartClass)       HeartClass = UHeartWidget::StaticClass();

    if (BtnTasks)  BtnTasks->OnClicked.AddDynamic(this, &UHUDRootWidget::OnBtnTasks);
    if (BtnHome)   BtnHome ->OnClicked.AddDynamic(this, &UHUDRootWidget::OnBtnHome);
    if (BtnToday)  BtnToday->OnClicked.AddDynamic(this, &UHUDRootWidget::OnTodayClicked);
    if (BtnStats)  BtnStats->OnClicked.AddDynamic(this, &UHUDRootWidget::OnBtnStats);
    if (BtnShop)   BtnShop ->OnClicked.AddDynamic(this, &UHUDRootWidget::OnBtnShop);

    // VM通知でトップバー反映（コイン等）
    if (TaskViewModel)
    {
        TaskViewModel->OnStatsChanged.AddDynamic(this, &UHUDRootWidget::OnVMStatsChanged);
    }
}

void UHUDRootWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetupChildren();
    RefreshTopBar();

    if (TaskViewModel && !TaskViewModel->OnStatsChanged.IsAlreadyBound(this, &UHUDRootWidget::OnVMStatsChanged))
    {
        TaskViewModel->OnStatsChanged.AddDynamic(this, &UHUDRootWidget::OnVMStatsChanged);
    }
    OnVMStatsChanged(); // 初回即反映（TxtSelectedName もここで更新）

    // 日付は1秒おきで更新（時刻/曜日表示がある想定）
    if (UWorld* W = GetWorld())
    {
        W->GetTimerManager().SetTimer(DateTimer, [this]()
            {
                RefreshTopBar();
            }, 1.0f, true);
    }

    if (SwitcherMain && Page_Home)
    {
        SwitcherMain->SetActiveWidget(Page_Home);
    }
}

void UHUDRootWidget::NativeDestruct()
{
    if (TaskViewModel && TaskViewModel->OnStatsChanged.IsAlreadyBound(this, &UHUDRootWidget::OnVMStatsChanged))
    {
        TaskViewModel->OnStatsChanged.RemoveDynamic(this, &UHUDRootWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

FReply UHUDRootWidget::NativeOnPreviewKeyDown(const FGeometry& Geo, const FKeyEvent& KeyEvent)
{
    if (KeyEvent.GetKey() == EKeys::Tab)
    {
        if (AFirstPlayerController* PC = GetOwningPlayer<AFirstPlayerController>())
        {
            PC->OnTabPressed();
            return FReply::Handled();
        }
    }
    return Super::NativeOnPreviewKeyDown(Geo, KeyEvent);
}

void UHUDRootWidget::SetupChildren()
{
    if (!TaskViewModel) return;

    // ResourceBar
    if (SizeBox_ResourceBar && !ResourceBarWidget)
    {
        ResourceBarWidget = CreateWidget<UResourceBarWidget>(GetOwningPlayer(), ResourceBarClass);

        auto LogImplements = [](UObject* O, const TCHAR* Where) {
            UE_LOG(LogTemp, Warning, TEXT("%s -> %s"), Where, O ? *O->GetClass()->GetName() : TEXT("nullptr"));
            };

        ITaskVMConsumer::Execute_SetTaskViewModel(ResourceBarWidget, TaskViewModel);
        if (UWidget* Old = SizeBox_ResourceBar->GetChildAt(0)) SizeBox_ResourceBar->RemoveChild(Old);
        SizeBox_ResourceBar->AddChild(ResourceBarWidget);
    }

    // Heart（中央パネル）
    if (SizeBox_Heart && !HeartWidget)
    {
        HeartWidget = CreateWidget<UHeartWidget>(GetOwningPlayer(), HeartClass);

        ITaskVMConsumer::Execute_SetTaskViewModel(HeartWidget, TaskViewModel);
        if (UWidget* Old = SizeBox_Heart->GetChildAt(0)) SizeBox_Heart->RemoveChild(Old);
        SizeBox_Heart->AddChild(HeartWidget);
    }

    // Today/Tasks
    if (!SwitcherMain) return;

    // Homeページの用意（UMG側に既に子があればそれをHomeとして使う）
    if (!Page_Home)
    {
        if (SwitcherMain->GetChildrenCount() > 0)
        {
            Page_Home = SwitcherMain->GetChildAt(0); // 先頭をHome扱い
        }
        else
        {
            // 何も無ければCanvasPanelを作成してHomeページにする
            UCanvasPanel* HomeCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Page_Home"));
            SwitcherMain->AddChild(HomeCanvas);
            Page_Home = HomeCanvas;
            SwitcherMain->SetActiveWidget(Page_Home);
        }
    }

    // ▼ Today(=Tasks)ページの用意（未生成なら生成）
    if (!TodayTabWidget)
    {
        TodayTabWidget = CreateWidget<UTodayTabWidget>(GetOwningPlayer(), TodayTabClass);
        if (TaskViewModel)
        {
            ITaskVMConsumer::Execute_SetTaskViewModel(TodayTabWidget, TaskViewModel);
        }
        SwitcherMain->AddChild(TodayTabWidget);
    }

    if(!Page_Control)
    {
        if (*ControlPageClass)
        {
            UUserWidget* W = CreateWidget<UUserWidget>(GetOwningPlayer(), ControlPageClass);
            Page_Control = W;
        }
        else
        {
            // フォールバック：コードで超簡易ページを生成
            UCanvasPanel* CtrlCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Page_Control"));
            UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_ControlHint"));
            Txt->SetText(FText::FromString(TEXT("Tab to Page")));
            CtrlCanvas->AddChild(Txt);
            Page_Control = CtrlCanvas;
        }
        SwitcherMain->AddChild(Page_Control);
    }
}

void UHUDRootWidget::OnTodayClicked()
{
    OnBtnTasks();
}
void UHUDRootWidget::OnBtnTasks()
{
    if (SwitcherMain && TodayTabWidget)
    {
        SwitcherMain->SetActiveWidget(TodayTabWidget);
        TodayTabWidget->SetIsFocusable(true);
        TodayTabWidget->SetUserFocus(GetOwningPlayer());   // ← ここが重要
        // 既存のデバッグ表示はそのまま
    }
}

void UHUDRootWidget::OnBtnHome()
{
    if (SwitcherMain && Page_Home)
    {
        SwitcherMain->SetActiveWidget(Page_Home);

        UE_LOG(LogTemp, Warning, TEXT("Page_Home Open %d"),
            SwitcherMain ? SwitcherMain->GetChildrenCount() : -1);
    }
}
void UHUDRootWidget::OnBtnStats() { /* 今は未実装タブを後で追加 */ }
void UHUDRootWidget::OnBtnShop() { /* 今は未実装タブを後で追加 */ }

void UHUDRootWidget::OnVMStatsChanged()
{
    RefreshTopBar();

    // ★ 選択中キャラ名の更新
    if (TxtSelectedName && TaskViewModel)
    {
        const FText Name = TaskViewModel->SelectedCharacterName.IsEmpty()
            ? FText::FromString(TEXT("not selected"))
            : TaskViewModel->SelectedCharacterName;

        TxtSelectedName->SetText(Name);
    }

    // Hintは簡易ロジック ハート低い/フォーカス低い等で一言
    if (TxtHint && TaskViewModel)
    {
        const FString H = (TaskViewModel->HeartValue < 30.f) ? TEXT("ケア優先：短い10分から始めよう")
            : (TaskViewModel->Focus < 20) ? TEXT("短時間タスクで集中を回復しよう")
            : TEXT("25分から軽くスタートしてみよう");
        TxtHint->SetText(FText::FromString(H));
    }
}

void UHUDRootWidget::ToggleNextTab()
{
    if (!SwitcherMain) return;
    const int32 N = SwitcherMain->GetChildrenCount();
    if (N <= 0) return;

    int32 Cur = SwitcherMain->GetActiveWidgetIndex();
    SwitcherMain->SetActiveWidgetIndex((Cur + 1) % N);
}

void UHUDRootWidget::ShowControlPage(bool /*bFocusMouse*/)
{
    if (SwitcherMain && Page_Control)
    {
        SwitcherMain->SetActiveWidget(Page_Control);
    }
}

void UHUDRootWidget::ShowHome()
{
    if (SwitcherMain && Page_Home)
    {
        SwitcherMain->SetActiveWidget(Page_Home);
    }
}

void UHUDRootWidget::RefreshTopBar()
{
    if (TxtDate)
    {
        const FDateTime Now = FDateTime::Now();  // ← 必ず宣言する

        // 日本語の曜日。FDateTime::GetDayOfWeek() は 0=Sun ... 6=Sat
        static const TCHAR* WJP[] = {
            TEXT("Sun"),TEXT("Mon"),TEXT("Tue"),TEXT("Wed"),TEXT("Thu"),TEXT("Fri"),TEXT("Sat")
            //TEXT("日"), TEXT("月"), TEXT("火"), TEXT("水"), TEXT("木"), TEXT("金"), TEXT("土")
        };
        const int32 W = static_cast<int32>(Now.GetDayOfWeek());

        // Printfの%sは「const TCHAR*」を渡す。FStringは * でTCHAR* を取り出す
        const FString S = FString::Printf(
            TEXT("[ %s (%s) ]"),
            *Now.ToString(TEXT("yyyy/MM/dd")), // ← FString を %s に渡すときは * を付けて TCHAR* にする
            WJP[W]                              // ← これは既に const TCHAR* なので * を付けない
        );

        TxtDate->SetText(FText::FromString(S));
    }

    if (TxtCoinTop && TaskViewModel)
    {
        TxtCoinTop->SetText(FText::FromString(FString::Printf(TEXT("Coin: %d"), TaskViewModel->Coin)));
    }
}


