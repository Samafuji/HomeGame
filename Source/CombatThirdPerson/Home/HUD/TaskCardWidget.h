#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "TaskCardWidget.generated.h"

class UTaskViewModel;
class UTextBlock;
class UProgressBar;
class UButton;

UCLASS()
class COMBATTHIRDPERSON_API UTaskCardWidget : public UUserWidget, public ITaskVMConsumer
{
    GENERATED_BODY()
public:
    // VM
    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    // 表示対象
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TaskId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText CategoryText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BaseMinutes = 0;

    // Bind or 生成
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Title = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Category = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Remain = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_State = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* Bar_Progress = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Start = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Pause = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Claim = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Claim = nullptr;

public:
    // 初期化：VMとアイテム情報を注入
    UFUNCTION(BlueprintCallable) void InitFromItem(const struct FTaskItemVM& Item, UTaskViewModel* InVM);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    void BuildWidgetTreeIfNeeded();
    void RefreshFromVM();
    bool FindMyItem(struct FTaskItemVM& Out) const;
    static FText FormatMMSS(float Seconds);

    // VM通知
    UFUNCTION() void OnVMStatsChanged();

    // ボタン
    UFUNCTION() void OnStartClicked();
    UFUNCTION() void OnPauseClicked();
    UFUNCTION() void OnCompleteClicked();
    UFUNCTION() void OnClaimClicked();
    void UpdateClaimUI(const FTaskItemVM& Item);  // ラベル・有効/無効の出し分け

public:
    // IF実装（TodayTabから先にVMだけを渡す場合にも対応）
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;
};
