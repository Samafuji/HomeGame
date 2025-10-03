// HUDRootWidget.h
// Blueprint�p��C++���HUD�iVM���󂯎����BP�Ńo�C���h���₷������j
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskVMConsumer.h"
#include "HUDRootWidget.generated.h"

class UTaskViewModel;// ...
class USizeBox;
class UWidgetSwitcher;
class UButton;
class UResourceBarWidget;
class UTodayTabWidget;
class UHeartWidget;
class UCheckBox;
class UTextBlock;

UCLASS(Abstract, BlueprintType)
class COMBATTHIRDPERSON_API UHUDRootWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
    UTaskViewModel* TaskViewModel = nullptr;

    // �q�N���X�i���ݒ�Ȃ�C++�N���X���g�p�j
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UResourceBarWidget> ResourceBarClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UTodayTabWidget> TodayTabClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UHeartWidget> HeartClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UUserWidget> ControlPageClass;

    // �f�U�C�i�ɒu�����g��h
    UPROPERTY(meta = (BindWidgetOptional)) USizeBox* SizeBox_ResourceBar = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USizeBox* SizeBox_Heart = nullptr;      // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UWidgetSwitcher* SwitcherMain = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnToday = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnHome = nullptr;             // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnTasks = nullptr;            // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnStats = nullptr;            // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnShop = nullptr;             // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UCheckBox* ToggleNotify = nullptr;      // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtHint = nullptr;          // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtDate = nullptr;          // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtCoinTop = nullptr;       // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnSettings = nullptr;         // ���ǉ�
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtSelectedName; // UMG�ɒu���Ă��Ȃ���Γ��I����

    UPROPERTY() int32 HomeIndex = INDEX_NONE;
    UPROPERTY() int32 TasksIndex = INDEX_NONE;
    UPROPERTY() UResourceBarWidget* ResourceBarWidget = nullptr;
    UPROPERTY() UTodayTabWidget* TodayTabWidget = nullptr;
    UPROPERTY() UHeartWidget* HeartWidget = nullptr;
    UPROPERTY() UWidget* Page_Control = nullptr;

    UPROPERTY() UWidget* Page_Home = nullptr;


    // �^�u�����ɉ񂷁iTab�L�[�p�j
    UFUNCTION(BlueprintCallable)
    void ToggleNextTab();

    // FirstPlayerController�p�y�[�W�փW�����v
    UFUNCTION(BlueprintCallable)
    void ShowControlPage(bool bFocusMouse = false);

    // Home�֖߂��i�C�ӂŎg����j
    UFUNCTION(BlueprintCallable)
    void ShowHome();

protected:


    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override; // �� �ǉ�
    virtual void NativeDestruct() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    void SetupChildren();
    UFUNCTION() void OnTodayClicked();
    UFUNCTION() void OnBtnHome();
    UFUNCTION() void OnBtnTasks();
    UFUNCTION() void OnBtnStats();
    UFUNCTION() void OnBtnShop();
    UFUNCTION() void OnVMStatsChanged();


    void RefreshTopBar();
    FTimerHandle DateTimer;
};
