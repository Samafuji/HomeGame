#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "TodayTabWidget.generated.h"

class UTaskViewModel;
class UTaskCardWidget;
class UDailyCheckCardWidget;

class UScrollBox;
class UTextBlock;
class UVerticalBox;

UCLASS()
class COMBATTHIRDPERSON_API UTodayTabWidget : public UUserWidget, public ITaskVMConsumer
{
    GENERATED_BODY()
public:
    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    // �X�N���[���̈�
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Total = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Running = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Done = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UScrollBox* ListBox = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UVerticalBox* ChecksBox = nullptr;

    // �����Ɏg���J�[�h�̃N���X�i���ݒ�Ȃ� C++�N���X���g�p�j
    UPROPERTY(EditAnywhere, Category = "Life")
    TSubclassOf<UTaskCardWidget> TaskCardClass;
    UPROPERTY(EditDefaultsOnly, Category = "DailyChecks")
    TSubclassOf<UDailyCheckCardWidget> DailyCheckCardClass;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    // �ǉ�
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    void BuildWidgetTreeIfNeeded();
    void BuildList();
    void RefreshAllCards();
    void RefreshSummary();

    void BuildChecks();
    void RefreshAllCheckCards();

    UFUNCTION() void OnVMStatsChanged();

public:
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;
};
