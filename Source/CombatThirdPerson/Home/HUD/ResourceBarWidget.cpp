// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/HUD/ResourceBarWidget.h"
#include "Home/TaskViewModel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

void UResourceBarWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UResourceBarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeIfNeeded();
    Refresh();
}

void UResourceBarWidget::NativeDestruct()
{
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UResourceBarWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UResourceBarWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

void UResourceBarWidget::BuildWidgetTreeIfNeeded()
{
    // デザイナで配置済みなら何もしない
    if (Txt_Coin && Txt_Focus && Txt_Affection && Bar_Mood) return;

    // ない場合は簡易UIをコードで組み立て
    if (!WidgetTree) return;
    UHorizontalBox* Root = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootHB"));
    WidgetTree->RootWidget = Root;

    auto MakeText = [&](const TCHAR* Name)->UTextBlock*
        {
            UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(Name));
            T->SetText(FText::FromString(TEXT("0")));
            UHorizontalBoxSlot* S = Root->AddChildToHorizontalBox(T);
            S->SetPadding(FMargin(8.f, 0.f));
            return T;
        };

    Txt_Coin = MakeText(TEXT("Txt_Coin"));
    Txt_Focus = MakeText(TEXT("Txt_Focus"));
    Txt_Affection = MakeText(TEXT("Txt_Affection"));

    Bar_Mood = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Bar_Mood"));
    Root->AddChildToHorizontalBox(Bar_Mood);
}

void UResourceBarWidget::Refresh()
{
    if (!TaskVM) return;
    if (Txt_Coin)      Txt_Coin->SetText(FText::AsNumber(TaskVM->Coin));
    if (Txt_Focus)     Txt_Focus->SetText(FText::AsNumber(TaskVM->Focus));
    if (Txt_Affection) Txt_Affection->SetText(FText::AsNumber(TaskVM->Affection));
    if (Bar_Mood)      Bar_Mood->SetPercent(TaskVM->Mood); // 0..1
}

void UResourceBarWidget::OnVMStatsChanged()
{
    Refresh();
}

void UResourceBarWidget::SetTaskViewModel_Implementation(UTaskViewModel* InVM)
{
    if (!InVM) return;

    // 既存Bind解除
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UResourceBarWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UResourceBarWidget::OnVMStatsChanged);
    }

    TaskVM = InVM;
    TaskVM->OnStatsChanged.AddDynamic(this, &UResourceBarWidget::OnVMStatsChanged);

    Refresh();
}