// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "HeartWidget.generated.h"

class UImage;
class UTaskViewModel;

/**
 * 
 */
UCLASS()
class COMBATTHIRDPERSON_API UHeartWidget : public UUserWidget, public ITaskVMConsumer
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidgetOptional)) UImage* ImgHeart = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseBPM = 60.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amp = 0.05f; // ägèkÅ}5%
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissionMin = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EmissionMax = 1.0f;

    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeo, float DeltaSeconds) override;

    UFUNCTION() void OnVMStatsChanged();

public:
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM);

private:
    float TimeAcc = 0.f;
    void ApplyVisuals(float HeartValue);
};