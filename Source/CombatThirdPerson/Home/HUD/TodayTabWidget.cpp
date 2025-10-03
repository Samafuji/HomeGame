// Fill out your copyright notice in the Description page of Project Settings.

//#include "Home/HUD/TaskVMConsumer.h"
#include "Home/HUD/DailyCheckCardWidget.h"
#include "Home/HUD/TodayTabWidget.h"
#include "Home/TaskViewModel.h"
#include "TaskCardWidget.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"        // �� �ǉ�
#include "Components/VerticalBox.h"      // �� �ǉ�
#include "Components/HorizontalBox.h"    // �� �ǉ�
#include "Blueprint/WidgetTree.h"

#include "CharacterInteraction/FirstPlayerController.h"
#include "Input/Events.h" // FKeyEvent
#include "Framework/Application/SlateApplication.h"

#include "DrawDebugHelpers.h" // �f�o�b�O�\���������ꍇ

void UTodayTabWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    //BuildWidgetTreeIfNeeded();
    BuildList();
    if (TaskVM) { BuildChecks(); }  // ����VM�������ς݂ŊJ���ꂽ�ꍇ�ɔ�����
    RefreshSummary(); // �����\��
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
            PC->OnTabPressed();              // �� �v���C���[�R���g���[���ֈϏ�
            return FReply::Handled();        // ����Tab��UI�ŏ����ς݂Ƃ���
        }
    }
    return Super::NativeOnPreviewKeyDown(Geo, KeyEvent);
}


void UTodayTabWidget::BuildWidgetTreeIfNeeded()
{
    // ���łɑS������Ȃ牽�����Ȃ�
    if (Txt_Total && Txt_Running && Txt_Done && ListBox) return;
    if (!WidgetTree) return;

    // ���[�g�F�c����
    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootVB"));
    WidgetTree->RootWidget = Root;

    // �w�b�_�[�s
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

    // ���X�g
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

        // VM�����i���C���^�[�t�F�C�X�o�R������j
        ITaskVMConsumer::Execute_SetTaskViewModel(Card, TaskVM);

        // ���̃A�C�e���̓��e�ŏ������i�^�C�g���E�c�莞�ԁE�{�^����ԂȂǁj
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

    if (!DailyCheckCardClass) return; // BP�������Ȃ�I��

    // ���я�
    TArray<FDailyCheckItemVM> Src = TaskVM->TodayChecks;
    Src.Sort([](const auto& A, const auto& B) { return A.Order < B.Order; });

    for (const auto& C : Src)
    {
        /*if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("CheckId"));*/

        // �� �^�� UDailyCheckCardWidget* �𖾎�
        UDailyCheckCardWidget* Row =
            CreateWidget<UDailyCheckCardWidget>(GetOwningPlayer(), DailyCheckCardClass);
        if (!Row) continue;

        /*if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                *FString::Printf(TEXT("CheckId: %s"), *C.CheckId.ToString()));*/

        // �� �e�s�ɌŗL��ID���Z�b�g�i���ꂪ�����ƑS�s�������\���ɂȂ�)
        Row->CheckId = C.CheckId;

        // VM���� �� �s���g�� RefreshFromVM() ����
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
            // VM���Ē������s���������� CheckId �� TodayChecks �������čX�V
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
        // �� �N������̃f�[�^�����ɂ������ł���悤�w��
        TaskVM->OnStatsChanged.AddDynamic(this, &UTodayTabWidget::OnVMStatsChanged);
    }

    /*BuildList();
    BuildChecks();*/
    RefreshSummary();
}
