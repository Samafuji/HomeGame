// Fill out your copyright notice in the Description page of Project Settings.


#include "Home/HUD/HeartWidget.h"
#include "Components/Image.h"
#include "Home/TaskViewModel.h"

void UHeartWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // 初期反映
    ApplyVisuals(TaskVM ? TaskVM->HeartValue : 60.f);
}

void UHeartWidget::NativeDestruct()
{
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UHeartWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UHeartWidget::OnVMStatsChanged);
    }
    Super::NativeDestruct();
}

void UHeartWidget::SetTaskViewModel_Implementation(UTaskViewModel* InVM)
{
    if (!InVM) return;
    if (TaskVM && TaskVM->OnStatsChanged.IsAlreadyBound(this, &UHeartWidget::OnVMStatsChanged))
    {
        TaskVM->OnStatsChanged.RemoveDynamic(this, &UHeartWidget::OnVMStatsChanged);
    }
    TaskVM = InVM;
    TaskVM->OnStatsChanged.AddDynamic(this, &UHeartWidget::OnVMStatsChanged);
    ApplyVisuals(TaskVM->HeartValue);
}

void UHeartWidget::OnVMStatsChanged()
{
    if (TaskVM) ApplyVisuals(TaskVM->HeartValue);
}

void UHeartWidget::NativeTick(const FGeometry& MyGeo, float DeltaSeconds)
{
    Super::NativeTick(MyGeo, DeltaSeconds);
    TimeAcc += DeltaSeconds;

    const float hv = TaskVM ? TaskVM->HeartValue : 60.f;
    const float bpm = BaseBPM + FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(-10, +40), hv);
    const float s = 1.0f + Amp * FMath::Sin(2 * PI * TimeAcc * bpm / 60.f);

    if (ImgHeart)
    {
        ImgHeart->SetRenderScale(FVector2D(s, s));
    }

    // 30未満の“弱り”ジッタ（ごく小さく）
    if (hv < 30.f && ImgHeart)
    {
        const float jitter = (FMath::FRand() - 0.5f) * 0.01f;
        ImgHeart->SetRenderScale(FVector2D(s + jitter, s + jitter));
    }
}

void UHeartWidget::ApplyVisuals(float HeartValue)
{
    const float e = FMath::Lerp(EmissionMin, EmissionMax, HeartValue / 100.f);
    if (ImgHeart)
    {
        FLinearColor c = ImgHeart->GetColorAndOpacity();
        c.A = e; // 透明度で“発光感”を簡易表現（マテリアル化すればEmissiveでもOK）
        ImgHeart->SetColorAndOpacity(c);
    }
}

