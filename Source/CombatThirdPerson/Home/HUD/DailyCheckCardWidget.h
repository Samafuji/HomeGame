// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "DailyCheckCardWidget.generated.h"

class UTaskViewModel;
class UCheckBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class COMBATTHIRDPERSON_API UDailyCheckCardWidget : public UUserWidget, public ITaskVMConsumer
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget)) UCheckBox* Check = nullptr;
    UPROPERTY(meta = (BindWidget)) UTextBlock* Txt = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CheckId;

    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION() void OnToggled(bool bIsChecked);

    // ITaskVMConsumer
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;

    void RefreshFromVM();

    UFUNCTION()
    void OnVMStatsChanged();
};