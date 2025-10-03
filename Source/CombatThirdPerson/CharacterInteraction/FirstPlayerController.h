// FirstPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AHomeCharacter; // �O���錾
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

    // �ǉ��v���g�^�C�v
    UFUNCTION() void OnTabPressed();

    // ��ԕێ��iUI�����ǂ����j
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    bool bUIInputMode = false;

    // HUD���[�h�iUI���샂�[�h�j���ǂ���
    UFUNCTION(BlueprintPure, Category = "UI")
    FORCEINLINE bool IsHUDModeActive() const { return bUIInputMode; }

    // �|�C���^��Ԃ��ꊇ�Ń��Z�b�g�i���S�I���p�j
    void CancelAllPointerStates();

    UFUNCTION()
    void SyncSelectedToVM();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void SetupInputComponent() override;

    // �� ���z�o�[����̃p�����[�^
    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    FName HeadBoneName = "head";

    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    float HeadHitTolerance = 10.f;

    UPROPERTY(EditAnywhere, Category = "HoverCursor")
    TEnumAsByte<ECollisionChannel> HoverTraceChannel = ECC_Pawn;

    // �J�[�\���p�E�B�W�F�b�g�iUMG�j
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

    // ---- Dragging ���� ----
    TWeakObjectPtr<AHomeCharacter> CurrentTarget;

    // �� ���Ȃŗp�t���O�i�d�����Ȃ��悤1�񂾂��j
    bool bDragging = false;
    bool bDraggingHead = false;

    // �h���b�O�n�_
    FVector2D DragStartScreenPos = FVector2D::ZeroVector;
    FVector   DragStartActorLocation = FVector::ZeroVector;
    FVector   DragStartWorldOnPlane = FVector::ZeroVector;

    // �� ���x�i�d�����Ȃ��悤1�񂾂��j
    float HeadDragSensitivity = 0.10f; // px��deg

    // ���ʃh���b�O�p
    FPlane DragPlane = FPlane(FVector::ZeroVector, FVector::UpVector);
    float DragClampHalfWidth = 1000.f;

    // �N���b�N/�h���b�O����
    bool bPointerDown = false;
    bool bDragStarted = false;
    FVector2D PointerDownScreenPos = FVector2D::ZeroVector;
    float DragDetectPixels = 6.f;

    bool bPressedOnHead = false;

    void ApplyInputMode(); // �����w���p�F�t���O��InputMode���܂Ƃ߂ēK�p

    // ���̓n���h��
    UFUNCTION() void OnLeftRelease();

    // �����w���p
    void UpdateDrag(float DeltaSeconds);
    bool ScreenPosToWorldOnPlane(const FVector2D& Screen, const FPlane& Plane, FVector& OutWorld) const;

    // �i�C�ӁjHUD�A�g��T�u�V�X�e���w�ǂ��d���ތ�
    void SetCurrentTarget(AHomeCharacter* NewTarget);
};