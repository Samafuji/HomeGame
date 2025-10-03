// Fill out your copyright notice in the Description page of Project Settings.

//#include "Home/HUD/TaskVMConsumer.h"
#include "Home/HUD/DailyCheckCardWidget.h"
#include "Home/HUD/TodayTabWidget.h"
#include "Home/TaskViewModel.h"
#include "TaskCardWidget.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"        // ← 追加
#include "Components/VerticalBox.h"      // ← 追加
#include "Components/HorizontalBox.h"    // ← 追加
#include "Blueprint/WidgetTree.h"

#include "CharacterInteraction/FirstPlayerController.h"
#include "Input/Events.h" // FKeyEvent
#include "Framework/Application/SlateApplication.h"

#include "DrawDebugHelpers.h" // デバッグ表示したい場合

void UTodayTabWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    //BuildWidgetTreeIfNeeded();
    BuildList();
    if (TaskVM) { BuildChecks(); }  // 既にVMが注入済みで開かれた場合に備える
    RefreshSummary(); // 初期表示
    if (TaskVM) { OnVMStatsChanged(); }
}

void UTodayTabWidget::NativeDestruct()
{
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UTodayTabWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UTodayTabWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

FReply UTodayTabWidget::NativeOnPreviewKeyDown(const FGeometry& Geo, const FKeyEvent& KeyEvent)
{
    if (KeyEvent.GetKey() == EKeys::Tab)
    {
        if (AFirstPlayerController* PC = GetOwningPlayer<AFirstPlayerController>())
        {
            PC->OnTabPressed();              // ← プレイヤーコントローラへ委譲
            return FReply::Handled();        // このTabはUIで処理済みとする
        }
    }
    return Super::NativeOnPreviewKeyDown(Geo, KeyEvent);
}


void UTodayTabWidget::BuildWidgetTreeIfNeeded()
{
    // すでに全部あるなら何もしない
    if (Txt_Total && Txt_Running && Txt_Done && ListBox) return;
    if (!WidgetTree) return;

    // ルート：縦並び
    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootVB"));
    WidgetTree->RootWidget = Root;

    // ヘッダー行
    UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderHB"));
    Root->AddChildToVerticalBox(Header);

    auto MakeText = [&](const TCHAR* Name, const TCHAR* Init)->UTextBlock*
        {
            UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(Name));
            T->SetText(FText::FromString(Init));
            Header->AddChildToHorizontalBox(T);
            return T;
        };

    Txt_Total = Txt_Total ? Txt_Total : MakeText(TEXT("Txt_Total"), TEXT("Total: 0"));
    Txt_Running = Txt_Running ? Txt_Running : MakeText(TEXT("Txt_Running"), TEXT("Running: 0"));
    Txt_Done = Txt_Done ? Txt_Done : MakeText(TEXT("Txt_Done"), TEXT("Done: 0"));

    // リスト
    ListBox = ListBox ? ListBox : WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ListBox"));
    Root->AddChildToVerticalBox(ListBox);

    ChecksBox = ChecksBox ? ChecksBox
        : WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ChecksBox"));
    Root->AddChildToVerticalBox(ChecksBox);
}


void UTodayTabWidget::BuildList()
{
    if (!TaskVM || !ListBox) return;

    ListBox->ClearChildren();

    TSubclassOf<UTaskCardWidget> CardCls = TaskCardClass;
    if (!*CardCls) CardCls = UTaskCardWidget::StaticClass();

    for (const auto& Item : TaskVM->TodayTasks)
    {
        UTaskCardWidget* Card = CreateWidget<UTaskCardWidget>(GetOwningPlayer(), CardCls);
        if (!Card) continue;

        // VM注入（※インターフェイス経由を厳守）
        ITaskVMConsumer::Execute_SetTaskViewModel(Card, TaskVM);

        // そのアイテムの内容で初期化（タイトル・残り時間・ボタン状態など）
        Card->InitFromItem(Item, TaskVM);

        ListBox->AddChild(Card);
    }

    RefreshSummary();
}

void UTodayTabWidget::RefreshAllCards()
{
    if (!ListBox) return;
    const int32 N = ListBox->GetChildrenCount();
    for (int32 i = 0; i < N; ++i)
    {
        if (auto* Card = Cast<UTaskCardWidget>(ListBox->GetChildAt(i)))
        {
            ITaskVMConsumer::Execute_SetTaskViewModel(Card, TaskVM);
        }
    }
}

void UTodayTabWidget::RefreshSummary()
{
    if (!TaskVM) return;

    const int32 Total = TaskVM->TodayTasks.Num();
    int32 Running = 0, Done = 0;
    for (const auto& T : TaskVM->TodayTasks)
    {
        if (T.State == ETaskState::Running) ++Running;
        if (T.State == ETaskState::Done)    ++Done;
    }

    if (Txt_Total) { Txt_Total->SetText(FText::FromString(FString::Printf(TEXT("Total: %d"), Total))); }
    if (Txt_Running) { Txt_Running->SetText(FText::FromString(FString::Printf(TEXT("Running: %d"), Running))); }
    if (Txt_Done) { Txt_Done->SetText(FText::FromString(FString::Printf(TEXT("Done: %d"), Done))); }
}

void UTodayTabWidget::BuildChecks()
{
    if (!TaskVM || !ChecksBox) return;
    if (ChecksBox->GetChildrenCount() == TaskVM->TodayChecks.Num()) { RefreshAllCheckCards(); return; }

    ChecksBox->ClearChildren();

    if (!DailyCheckCardClass) return; // BP未割当なら終了

    // 並び順
    TArray<FDailyCheckItemVM> Src = TaskVM->TodayChecks;
    Src.Sort([](const auto& A, const auto& B) { return A.Order < B.Order; });

    for (const auto& C : Src)
    {
        /*if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("CheckId"));*/

        // ★ 型は UDailyCheckCardWidget* を明示
        UDailyCheckCardWidget* Row =
            CreateWidget<UDailyCheckCardWidget>(GetOwningPlayer(), DailyCheckCardClass);
        if (!Row) continue;

        /*if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                *FString::Printf(TEXT("CheckId: %s"), *C.CheckId.ToString()));*/

        // ★ 各行に固有のIDをセット（これが無いと全行が同じ表示になる)
        Row->CheckId = C.CheckId;

        // VM注入 → 行自身が RefreshFromVM() する
        ITaskVMConsumer::Execute_SetTaskViewModel(Row, TaskVM);

        ChecksBox->AddChild(Row);
    }

    ChecksBox->SetVisibility(ESlateVisibility::Visible);
}

void UTodayTabWidget::RefreshAllCheckCards()
{
    if (!TaskVM || !ChecksBox) return;

    const int32 N = ChecksBox->GetChildrenCount();
    for (int32 i = 0; i < N; ++i)
    {
        if (auto* Row = Cast<UDailyCheckCardWidget>(ChecksBox->GetChildAt(i)))
        {
            // VMを再注入→行側が自分の CheckId で TodayChecks を引いて更新
            ITaskVMConsumer::Execute_SetTaskViewModel(Row, TaskVM);
        }
    }
}

void UTodayTabWidget::OnVMStatsChanged()
{
    if (!TaskVM) return;

    if (TaskVM && ListBox)
    {
        if (ListBox->GetChildrenCount() == TaskVM->TodayTasks.Num())
            RefreshAllCards();
        else BuildList();
    }

    if (TaskVM && ChecksBox)
    {
        const int32 Childs = ChecksBox->GetChildrenCount();
        const int32 Items = TaskVM->TodayChecks.Num();
        if (Childs == Items) RefreshAllCheckCards();
        else                 BuildChecks();
    }

    RefreshSummary();
}

void UTodayTabWidget::SetTaskViewModel_Implementation(UTaskViewModel* InVM)
{
    if (!InVM) return;

    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UTodayTabWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UTodayTabWidget::OnVMStatsChanged);
    }

    TaskVM = InVM;
    if (TaskVM)
    {
        // ★ 起動直後のデータ投入にも反応できるよう購読
        TaskVM->OnStatsChanged.AddDynamic(this, &UTodayTabWidget::OnVMStatsChanged);
    }

    /*BuildList();
    BuildChecks();*/
    RefreshSummary();
}
