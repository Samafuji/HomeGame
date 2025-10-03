// HUDRootWidget.h
// Blueprint用のC++基底HUD（VMを受け取ってBPでバインドしやすくする）
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

    // 子クラス（未設定ならC++クラスを使用）
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UResourceBarWidget> ResourceBarClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UTodayTabWidget> TodayTabClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UHeartWidget> HeartClass;
    UPROPERTY(EditAnywhere, Category = "Life|Classes")
    TSubclassOf<UUserWidget> ControlPageClass;

    // デザイナに置いた“器”
    UPROPERTY(meta = (BindWidgetOptional)) USizeBox* SizeBox_ResourceBar = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USizeBox* SizeBox_Heart = nullptr;      // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UWidgetSwitcher* SwitcherMain = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnToday = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnHome = nullptr;             // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnTasks = nullptr;            // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnStats = nullptr;            // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnShop = nullptr;             // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UCheckBox* ToggleNotify = nullptr;      // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtHint = nullptr;          // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtDate = nullptr;          // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtCoinTop = nullptr;       // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnSettings = nullptr;         // ★追加
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtSelectedName; // UMGに置いていなければ動的生成

    UPROPERTY() int32 HomeIndex = INDEX_NONE;
    UPROPERTY() int32 TasksIndex = INDEX_NONE;
    UPROPERTY() UResourceBarWidget* ResourceBarWidget = nullptr;
    UPROPERTY() UTodayTabWidget* TodayTabWidget = nullptr;
    UPROPERTY() UHeartWidget* HeartWidget = nullptr;
    UPROPERTY() UWidget* Page_Control = nullptr;

    UPROPERTY() UWidget* Page_Home = nullptr;


    // タブを次に回す（Tabキー用）
    UFUNCTION(BlueprintCallable)
    void ToggleNextTab();

    // FirstPlayerController用ページへジャンプ
    UFUNCTION(BlueprintCallable)
    void ShowControlPage(bool bFocusMouse = false);

    // Homeへ戻す（任意で使える）
    UFUNCTION(BlueprintCallable)
    void ShowHome();

protected:


    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override; // ← 追加
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
