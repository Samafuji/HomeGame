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
#include "Components/CanvasPanel.h"      // ��Home�y�[�W���������p
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

    // VM�ʒm�Ńg�b�v�o�[���f�i�R�C�����j
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
    OnVMStatsChanged(); // ���񑦔��f�iTxtSelectedName �������ōX�V�j

    // ���t��1�b�����ōX�V�i����/�j���\��������z��j
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

    // Heart�i�����p�l���j
    if (SizeBox_Heart && !HeartWidget)
    {
        HeartWidget = CreateWidget<UHeartWidget>(GetOwningPlayer(), HeartClass);

        ITaskVMConsumer::Execute_SetTaskViewModel(HeartWidget, TaskViewModel);
        if (UWidget* Old = SizeBox_Heart->GetChildAt(0)) SizeBox_Heart->RemoveChild(Old);
        SizeBox_Heart->AddChild(HeartWidget);
    }

    // Today/Tasks
    if (!SwitcherMain) return;

    // Home�y�[�W�̗p�ӁiUMG���Ɋ��Ɏq������΂����Home�Ƃ��Ďg���j
    if (!Page_Home)
    {
        if (SwitcherMain->GetChildrenCount() > 0)
        {
            Page_Home = SwitcherMain->GetChildAt(0); // �擪��Home����
        }
        else
        {
            // �����������CanvasPanel���쐬����Home�y�[�W�ɂ���
            UCanvasPanel* HomeCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Page_Home"));
            SwitcherMain->AddChild(HomeCanvas);
            Page_Home = HomeCanvas;
            SwitcherMain->SetActiveWidget(Page_Home);
        }
    }

    // �� Today(=Tasks)�y�[�W�̗p�Ӂi�������Ȃ琶���j
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
            // �t�H�[���o�b�N�F�R�[�h�Œ��ȈՃy�[�W�𐶐�
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
        TodayTabWidget->SetUserFocus(GetOwningPlayer());   // �� �������d�v
        // �����̃f�o�b�O�\���͂��̂܂�
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
void UHUDRootWidget::OnBtnStats() { /* ���͖������^�u����Œǉ� */ }
void UHUDRootWidget::OnBtnShop() { /* ���͖������^�u����Œǉ� */ }

void UHUDRootWidget::OnVMStatsChanged()
{
    RefreshTopBar();

    // �� �I�𒆃L�������̍X�V
    if (TxtSelectedName && TaskViewModel)
    {
        const FText Name = TaskViewModel->SelectedCharacterName.IsEmpty()
            ? FText::FromString(TEXT("not selected"))
            : TaskViewModel->SelectedCharacterName;

        TxtSelectedName->SetText(Name);
    }

    // Hint�͊ȈՃ��W�b�N �n�[�g�Ⴂ/�t�H�[�J�X�Ⴂ���ňꌾ
    if (TxtHint && TaskViewModel)
    {
        const FString H = (TaskViewModel->HeartValue < 30.f) ? TEXT("�P�A�D��F�Z��10������n�߂悤")
            : (TaskViewModel->Focus < 20) ? TEXT("�Z���ԃ^�X�N�ŏW�����񕜂��悤")
            : TEXT("25������y���X�^�[�g���Ă݂悤");
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
        const FDateTime Now = FDateTime::Now();  // �� �K���錾����

        // ���{��̗j���BFDateTime::GetDayOfWeek() �� 0=Sun ... 6=Sat
        static const TCHAR* WJP[] = {
            TEXT("Sun"),TEXT("Mon"),TEXT("Tue"),TEXT("Wed"),TEXT("Thu"),TEXT("Fri"),TEXT("Sat")
            //TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("��"), TEXT("�y")
        };
        const int32 W = static_cast<int32>(Now.GetDayOfWeek());

        // Printf��%s�́uconst TCHAR*�v��n���BFString�� * ��TCHAR* �����o��
        const FString S = FString::Printf(
            TEXT("[ %s (%s) ]"),
            *Now.ToString(TEXT("yyyy/MM/dd")), // �� FString �� %s �ɓn���Ƃ��� * ��t���� TCHAR* �ɂ���
            WJP[W]                              // �� ����͊��� const TCHAR* �Ȃ̂� * ��t���Ȃ�
        );

        TxtDate->SetText(FText::FromString(S));
    }

    if (TxtCoinTop && TaskViewModel)
    {
        TxtCoinTop->SetText(FText::FromString(FString::Printf(TEXT("Coin: %d"), TaskViewModel->Coin)));
    }
}


