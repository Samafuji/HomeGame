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
    // VM�Q��
    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    // �f�U�C�i�ɒu���ꍇ�� BindWidgetOptional�A���SC++�ł���q��Build�Ő���
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Coin = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Focus = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Txt_Affection = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* Bar_Mood = nullptr;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // VM�ʒm�n���h��
    UFUNCTION() void OnVMStatsChanged();

    // �\���X�V
    void Refresh();

    // �f�U�C�i�����ł������悤�ɍŒ����UI�𐶐�
    void BuildWidgetTreeIfNeeded();

public:
    // IF����
    virtual void SetTaskViewModel_Implementation(UTaskViewModel* InVM) override;

private:
    FDelegateHandle StatsChangedHandle; // �iRemoveDynamic���g���̂Ŗ��g�p�ł�OK�j
};
