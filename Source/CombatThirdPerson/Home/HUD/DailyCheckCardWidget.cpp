// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/HUD/DailyCheckCardWidget.h"
#include "Home/TaskViewModel.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

void UDailyCheckCardWidget::SetTaskViewModel_Implementation(UTaskViewModel* InVM)
{
    // ‚·‚Å‚Éw“Ç‚µ‚Ä‚¢‚½‚ç‰ðœ
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UDailyCheckCardWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UDailyCheckCardWidget::OnVMStatsChanged);
    }

    TaskVM = InVM;

    if (TaskVM)
    {
        TaskVM->OnStatsChanged.AddDynamic(this, &UDailyCheckCardWidget::OnVMStatsChanged);
    }
    RefreshFromVM();
}

void UDailyCheckCardWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Check)
    {
        Check->OnCheckStateChanged.RemoveAll(this);
        Check->OnCheckStateChanged.AddDynamic(this, &UDailyCheckCardWidget::OnToggled);
    }
}

void UDailyCheckCardWidget::NativeDestruct()
{
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UDailyCheckCardWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UDailyCheckCardWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

void UDailyCheckCardWidget::OnVMStatsChanged()
{
    RefreshFromVM();
}

void UDailyCheckCardWidget::OnToggled(bool bIsChecked)
{
    if (TaskVM && CheckId != NAME_None)
    {
        TaskVM->SetDailyCheck(CheckId, bIsChecked);
    }
}


void UDailyCheckCardWidget::RefreshFromVM()
{
    if (!TaskVM) return;

    for (const auto& C : TaskVM->TodayChecks)
    {
        if (C.CheckId == CheckId) {
            if (Txt)   Txt->SetText(C.DisplayName);
            if (Check) Check->SetIsChecked(C.bChecked);
            break;
        }
    }
}
