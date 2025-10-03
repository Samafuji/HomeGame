#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "ResourceBarWidget.generated.h"

class UTaskViewModel;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;

UCLASS()
class COMBATTHIRDPERSON_API UResourceBarWidget : public UUserWidget, public ITaskVMConsumer
{
    GENERATED_BODY()
public:
    // VM参照
    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    // デザイナに置く場合は BindWidgetOptional、完全C++でも後述のBuildで生成
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Coin = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Focus = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Affection = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* Bar_Mood = nullptr;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // VM通知ハンドラ
    UFUNCTION() void OnVMStatsChanged();

    // 表示更新
    void Refresh();

    // デザイナ無しでも動くように最低限のUIを生成
    void BuildWidgetTreeIfNeeded();

public:
    // IF実装
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;

private:
    FDelegateHandle StatsChangedHandle; // （RemoveDynamicを使うので未使用でもOK）
};
