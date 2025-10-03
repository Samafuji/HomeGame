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

    // �\���Ώ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TaskId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText CategoryText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BaseMinutes = 0;

    // Bind or ����
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
    // �������FVM�ƃA�C�e�����𒍓�
    UFUNCTION(BlueprintCallable) void InitFromItem(const struct FTaskItemVM& Item, UTaskViewModel* InVM);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    void BuildWidgetTreeIfNeeded();
    void RefreshFromVM();
    bool FindMyItem(struct FTaskItemVM& Out) const;
    static FText FormatMMSS(float Seconds);

    // VM�ʒm
    UFUNCTION() void OnVMStatsChanged();

    // �{�^��
    UFUNCTION() void OnStartClicked();
    UFUNCTION() void OnPauseClicked();
    UFUNCTION() void OnCompleteClicked();
    UFUNCTION() void OnClaimClicked();
    void UpdateClaimUI(const FTaskItemVM& Item);  // ���x���E�L��/�����̏o������

public:
    // IF�����iTodayTab������VM������n���ꍇ�ɂ��Ή��j
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;
};
