// FirstPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AHomeCharacter; // 前方宣言
class UHUDRootWidget;
class UTaskViewModel;

#include "FirstPlayerController.generated.h"

UCLASS()
class COMBATTHIRDPERSON_API AFirstPlayerController : public APlayerController
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputMappingContext* MenuMappingContext;

public:
    virtual void Tick(float DeltaSeconds) override;
    bool GetHoverHitResultUnderCursor(FHitResult& OutHit, float TraceDistance = 100000.f) const;

    // 追加プロトタイプ
    UFUNCTION() void OnTabPressed();

    // 状態保持（UI中かどうか）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    bool bUIInputMode = false;

    // HUDモード（UI操作モード）かどうか
    UFUNCTION(BlueprintPure, Category = "UI")
    FORCEINLINE bool IsHUDModeActive() const { return bUIInputMode; }

    // ポインタ状態を一括でリセット（安全終了用）
    void CancelAllPointerStates();

    UFUNCTION()
    void SyncSelectedToVM();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void SetupInputComponent() override;

    // ▼ 頭ホバー判定のパラメータ
    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    FName HeadBoneName = "head";

    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    float HeadHitTolerance = 10.f;

    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    TEnumAsByte<ECollisionChannel> HoverTraceChannel = ECC_Pawn;

    // カーソル用ウィジェット（UMG）
    UPROPERTY(EditDefaultsOnly, Category = "Cursor")
    TSubclassOf<UUserWidget> CursorClass;

    // UMG
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UUserWidget> HUDClass = nullptr;
    UPROPERTY() UHUDRootWidget* HUD = nullptr;
    UPROPERTY() UTaskViewModel* TaskVM = nullptr;

    // Save
    UPROPERTY(EditAnywhere, Category = "Life|Save")
    FString SaveSlotName = TEXT("TaskVM");

    UPROPERTY(EditAnywhere, Category = "Life|Save")
    int32 SaveUserIndex = 0;

    UPROPERTY(EditAnywhere, Category = "Life")
    UDataTable* TaskTable = nullptr;
    UPROPERTY(EditAnywhere, Category = "Life")
    UDataTable* DailyCheckTable = nullptr;

private:
    UPROPERTY() UUserWidget* CursorInst = nullptr;
    bool bCursorOverHead = false;

    void InitCursorWidget();
    void UpdateHoverCursor();
    void OnLeftClick();

    // ---- Dragging 共通 ----
    TWeakObjectPtr<AHomeCharacter> CurrentTarget;

    // ★ 頭なで用フラグ（重複しないよう1回だけ）
    bool bDragging = false;
    bool bDraggingHead = false;

    // ドラッグ始点
    FVector2D DragStartScreenPos = FVector2D::ZeroVector;
    FVector   DragStartActorLocation = FVector::ZeroVector;
    FVector   DragStartWorldOnPlane = FVector::ZeroVector;

    // ★ 感度（重複しないよう1回だけ）
    float HeadDragSensitivity = 0.10f; // px→deg

    // 平面ドラッグ用
    FPlane DragPlane = FPlane(FVector::ZeroVector, FVector::UpVector);
    float DragClampHalfWidth = 1000.f;

    // クリック/ドラッグ判定
    bool bPointerDown = false;
    bool bDragStarted = false;
    FVector2D PointerDownScreenPos = FVector2D::ZeroVector;
    float DragDetectPixels = 6.f;

    bool bPressedOnHead = false;

    void ApplyInputMode(); // 内部ヘルパ：フラグとInputModeをまとめて適用

    // 入力ハンドラ
    UFUNCTION() void OnLeftRelease();

    // 内部ヘルパ
    void UpdateDrag(float DeltaSeconds);
    bool ScreenPosToWorldOnPlane(const FVector2D& Screen, const FPlane& Plane, FVector& OutWorld) const;

    // （任意）HUD連携やサブシステム購読を仕込む口
    void SetCurrentTarget(AHomeCharacter* NewTarget);
};