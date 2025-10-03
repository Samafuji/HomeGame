// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/HUD/TaskCardWidget.h"
#include "Home/TaskViewModel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UTaskCardWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeIfNeeded();

    // �{�^���n���h��
    if (Btn_Start)   Btn_Start->OnClicked.AddDynamic(this, &UTaskCardWidget::OnStartClicked);
    if (Btn_Pause)   Btn_Pause->OnClicked.AddDynamic(this, &UTaskCardWidget::OnPauseClicked);
    if (Btn_Claim)
    {
        Btn_Claim->OnClicked.RemoveAll(this);
        Btn_Claim->OnClicked.AddDynamic(this, &UTaskCardWidget::OnClaimClicked);
    }

    // �����\��
    if (Txt_Title)    Txt_Title->SetText(DisplayName);
    if (Txt_Category) Txt_Category->SetText(CategoryText);
    RefreshFromVM();
}

void UTaskCardWidget::NativeDestruct()
{
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UTaskCardWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UTaskCardWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

void UTaskCardWidget::BuildWidgetTreeIfNeeded()
{
    if (Txt_Title && Bar_Progress && Txt_Remain && Txt_State && Btn_Start && Btn_Pause) return;
    if (!WidgetTree) return;

    UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CardBorder"));
    WidgetTree->RootWidget = Root;

    UVerticalBox* VB = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VB"));
    Root->SetContent(VB);

    // ��i �^�C�g���{�J�e�S���i�ȗ��j
    UHorizontalBox* HBTop = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HBTop"));
    VB->AddChildToVerticalBox(HBTop);
    Txt_Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_Title"));
    HBTop->AddChildToHorizontalBox(Txt_Title);
    Txt_Category = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_Category"));
    HBTop->AddChildToHorizontalBox(Txt_Category);

    // ���i �i��
    Bar_Progress = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Bar_Progress"));
    VB->AddChildToVerticalBox(Bar_Progress);

    // ���i �c��/���/�{�^���Q
    UHorizontalBox* HBBottom = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HBBottom"));
    VB->AddChildToVerticalBox(HBBottom);

    Txt_Remain = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_Remain"));
    HBBottom->AddChildToHorizontalBox(Txt_Remain);

    Txt_State = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_State"));
    HBBottom->AddChildToHorizontalBox(Txt_State);

    Btn_Start = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Btn_Start"));
    HBBottom->AddChildToHorizontalBox(Btn_Start);
    Btn_Pause = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Btn_Pause"));
    HBBottom->AddChildToHorizontalBox(Btn_Pause);
}

void UTaskCardWidget::InitFromItem(const FTaskItemVM& Item, UTaskViewModel* InVM)
{
    TaskId = Item.TaskId;
    DisplayName = Item.DisplayName;
    CategoryText = Item.CategoryText;
    BaseMinutes = Item.BaseMinutes;

    if (Txt_Title)    Txt_Title->SetText(DisplayName);
    if (Txt_Category) Txt_Category->SetText(CategoryText);

    ITaskVMConsumer::Execute_SetTaskViewModel(this, InVM);
    RefreshFromVM();
}

void UTaskCardWidget::SetTaskViewModel_Implementation(UTaskViewModel* InVM)
{
    if (!InVM) return;

    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UTaskCardWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UTaskCardWidget::OnVMStatsChanged);
    }

    TaskVM = InVM;
    TaskVM->OnStatsChanged.AddDynamic(this, &UTaskCardWidget::OnVMStatsChanged);
    RefreshFromVM();
}

bool UTaskCardWidget::FindMyItem(FTaskItemVM& Out) const
{
    if (!TaskVM) return false;
    for (const auto& T : TaskVM->TodayTasks)
    {
        if (T.TaskId == TaskId) { Out = T; return true; }
    }
    return false;
}

FText UTaskCardWidget::FormatMMSS(float Seconds)
{
    int32 S = FMath::Max(0, (int32)FMath::RoundHalfFromZero(Seconds));
    int32 M = S / 60;
    S = S % 60;
    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), M, S));
}

void UTaskCardWidget::RefreshFromVM()
{
    if (!TaskVM) return;

    FTaskItemVM Item;
    if (!FindMyItem(Item)) return;

    if (Bar_Progress) Bar_Progress->SetPercent(Item.Progress01);
    if (Txt_State)
    {
        const TCHAR* S = TEXT("Ready");
        switch (Item.State)
        {
        case ETaskState::Running: S = TEXT("Running"); break;
        case ETaskState::Paused:  S = TEXT("Paused");  break;
        case ETaskState::Finished: S = TEXT("Finished"); break; // �󂯎��҂�
        case ETaskState::Done:    S = TEXT("Done");    break;
        default: break;
        }
        Txt_State->SetText(FText::FromString(S));
    }
    if (Txt_Remain)
    {
        const bool bZero =
            (Item.State == ETaskState::Finished || Item.State == ETaskState::Done);
        const float Remain = bZero ? 0.f : Item.RemainingSeconds;
        Txt_Remain->SetText(FormatMMSS(Remain));
    }

    // �{�^������
    const bool bStartable =
        (Item.State == ETaskState::Ready || Item.State == ETaskState::Paused);
    if (Btn_Start)  Btn_Start->SetIsEnabled(bStartable);
    if (Btn_Pause)  Btn_Pause->SetIsEnabled(Item.State == ETaskState::Running);

    UpdateClaimUI(Item);
}

void UTaskCardWidget::OnVMStatsChanged()
{
    RefreshFromVM();
}

void UTaskCardWidget::OnStartClicked()
{
    if (TaskVM && TaskId != NAME_None) TaskVM->StartTask(TaskId);
}

void UTaskCardWidget::OnPauseClicked()
{
    if (TaskVM && TaskId != NAME_None) TaskVM->PauseTask(TaskId);
}

void UTaskCardWidget::OnCompleteClicked()
{
    if (TaskVM && TaskId != NAME_None) TaskVM->CompleteTask(TaskId);
}

void UTaskCardWidget::OnClaimClicked()
{
    if (TaskVM && TaskId != NAME_None)
    {
        TaskVM->ClaimTask(TaskId);

        // �y�ύX�V�i�C�Ӂj�F�����uClaimed�v�Ɍ�����
        if (Btn_Claim) Btn_Claim->SetIsEnabled(false);
        if (Txt_Claim) Txt_Claim->SetText(NSLOCTEXT("Task", "Claimed", "Claimed"));
    }
}

void UTaskCardWidget::UpdateClaimUI(const FTaskItemVM& Item)
{

    if (Txt_Claim)
    {
        // ��Ԃɉ����ă��x���ؑ�
        FText Label;
        switch (Item.State)
        {
        case ETaskState::Finished: // �������󂯎��҂�
            Label = NSLOCTEXT("Task", "Claim", "Claim");
            break;
        case ETaskState::Done:     // �󂯎��ς�
            Label = NSLOCTEXT("Task", "Claimed", "Claimed");
            break;
        default:                   // �i�s��/Ready/Paused
            Label = NSLOCTEXT("Task", "InProgress", "InProgress");
            break;
        }
        Txt_Claim->SetText(Label);
    }

    if (Btn_Claim)
    {
        const bool bCanClaim = (Item.State == ETaskState::Finished);
        Btn_Claim->SetIsEnabled(bCanClaim);
        Btn_Claim->SetVisibility(ESlateVisibility::Visible);

        // �i�C�Ӂj�c�[���`�b�v�ŏ�ԕ⑫
        const FText Tip = bCanClaim
            ? FText::FromString(TEXT("Complete�IREward can be got"))
            : FText::FromString(TEXT("task progressing�cclaim after completing"));
        Btn_Claim->SetToolTipText(Tip);
    }
}

