// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInteraction/FirstPlayerController.h"
#include "CombatThirdPerson.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h" // デバッグ表示したい場合


#include "Character/HomeCharacter.h"
#include "Components/CapsuleComponent.h" // ← 忘れずに

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/SViewport.h"

#include "Blueprint/UserWidget.h"
#include "Home/HUD/HUDRootWidget.h"
#include "Home/TaskViewModel.h"
#include "Home/Components/CharacterStatusComponent.h"
#include "Home/DailyCycleSubsystem.h"

// クラス参照はBPから設定：TSubclassOf<UUserWidget> HUDClass; などをUPROPERTYで用意しておく
UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UUserWidget> HUDClass;
UUserWidget* HUDInstance = nullptr;

void AFirstPlayerController::CancelAllPointerStates()
{
    bPointerDown = false;
    bDragStarted = false;
    bDragging = false;
    bDraggingHead = false;
    bPressedOnHead = false;
}

UFUNCTION()
void AFirstPlayerController::SyncSelectedToVM()
{
    if (!TaskVM) return;

    if (AHomeCharacter* HC = CurrentTarget.Get())
    {
        if (UCharacterStatusComponent* S = HC->Status)
        {
            const FCharacterStatusSnapshot Snap = S->MakeSnapshot();

            // ★ ステ値ミラー（既存どおり）
            TaskVM->Affection = Snap.Affection;
            TaskVM->Mood = Snap.Mood;
            TaskVM->Focus = Snap.Focus;
            TaskVM->Coin = Snap.Coin;
            TaskVM->HeartValue = Snap.Heart;   // ← HeartWidget が読むプロパティ名に合わせる

            // ★ メタ情報（名前/ID）もセット
            const FName CharId = HC->GetFName(); // ID フィールドがあるなら HC->CharacterId を推奨
#if WITH_EDITOR
            const FText CharName = FText::FromString(HC->GetActorLabel()); // Editor 名
#else
            const FText CharName = FText::FromString(HC->GetName()); // 実行時は名前
#endif
            TaskVM->SetSelectedCharacterMeta(CharId, CharName);

            // 最後に通知（SetSelectedCharacterMeta 内で Broadcast 済みなら省略可）
            TaskVM->OnStatsChanged.Broadcast();
        }
    }
    else
    {
        // 未選択状態（任意）
        TaskVM->SelectedCharacterId = NAME_None;
        TaskVM->SelectedCharacterName = FText::FromString(TEXT("Not Selected"));
        TaskVM->OnStatsChanged.Broadcast();
    }
}

void AFirstPlayerController::BeginPlay()
{
	Super::BeginPlay();

    // FPSデフォルト：UI無効
    bUIInputMode = false;
    ApplyInputMode();

	/*bEnableMouseOverEvents = true;
	bEnableClickEvents = true;*/
    //bShowMouseCursor = true;
/*
	// UI Only（必要ならフォーカスするウィジェットを設定）
	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);*/

    InitCursorWidget();

    TaskVM = NewObject<UTaskViewModel>(this);

    // 既存セーブ優先で復元（無ければ DataTable から新規）
    const bool bLoaded = TaskVM->LoadFromSlot(TaskTable, TEXT("TaskVM"), 0, /*bAppend=*/false);

    if (TaskVM)
    {
        // ★ 起動瞬間に“04:00跨ぎ済みか”を自前チェックして即リセット
        TaskVM->EnsureFreshDailyWeeklyOnLaunch();

        // 以後は毎日04:00に自動発火（従来どおり購読）
        if (UWorld* W = GetWorld()) {
            if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>()) {
                Cycle->OnDailyReset.AddDynamic(TaskVM, &UTaskViewModel::HandleDailyReset);
            }
        }
        
        // その後に DataTable から「不足分だけ」追加入り
        if (TaskTable)
        {
            const int32 Added = TaskVM->MergeFromDataTable(TaskTable);
            if (Added > 0 && GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                    *FString::Printf(TEXT("New tasks added: %d"), Added));
        }

        if (DailyCheckTable)
        {
            // いつでも「不足分だけ」追加入り（空のときは全件入る）
            const int32 AddedChecks = TaskVM->MergeDailyChecksFromDataTable(DailyCheckTable);
            if (AddedChecks > 0 && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1, 5.f, FColor::Green,
                    *FString::Printf(TEXT("New daily-checks added: %d"), AddedChecks));
            }
        }
    }
    
    if (HUDClass)
    {
        HUD = CreateWidget<UHUDRootWidget>(this, HUDClass);
        HUD->TaskViewModel = TaskVM;     // ExposeOnSpawn なら SpawnParams利用でもOK
        HUD->AddToViewport();
    }

    if (AHomeCharacter* HC = Cast<AHomeCharacter>(GetCharacter()))
    {
        HC->TaskViewModelRef = TaskVM;  // ★ これでクリック→好感度反映の導線が完成
    }

    if (UWorld* W = GetWorld()) {
        if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>()) {
            Cycle->OnDailyReset.AddDynamic(TaskVM, &UTaskViewModel::HandleDailyReset);
        }
    }

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("PlayerController Init"));
}

// （任意）終了時にオートセーブ
void AFirstPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (TaskVM)
    {
        TaskVM->SaveToSlot(SaveSlotName, SaveUserIndex);
    }
    Super::EndPlay(EndPlayReason);
}

void AFirstPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    // HUD
    if (TaskVM) { TaskVM->Tick(DeltaSeconds); }

    // ★ HUDモードのときだけホバー更新を行う
    if (IsHUDModeActive())
    {
        UpdateHoverCursor();

        // まだドラッグ開始していない → しきい値チェック
        if (bPointerDown && !bDragStarted)
        {
            if (!IsHUDModeActive())
            {
                // HUDモードでないのにドラッグ状態にならないように保険
                CancelAllPointerStates();
            }
            else
            {
                float mx, my;
                if (GetMousePosition(mx, my))
                {
                    const FVector2D Now(mx, my);
                    const float Dist2 = (Now - PointerDownScreenPos).SizeSquared();
                    if (Dist2 >= FMath::Square(DragDetectPixels))
                    {
                        // ★ここでドラッグ開始確定★
                        bDragStarted = true;

                        if (CurrentTarget.IsValid() && bPressedOnHead)
                        {
                            bDragging = true;
                            bDraggingHead = true;   // ← これが重要！

                            if (HUD) { HUD->ShowControlPage(false); }

                            // 頭なで基準の記録
                            DragStartScreenPos = PointerDownScreenPos;
                            DragStartActorLocation = CurrentTarget->GetActorLocation();
                            DragPlane = FPlane(DragStartActorLocation, FVector::UpVector);
                            ScreenPosToWorldOnPlane(DragStartScreenPos, DragPlane, DragStartWorldOnPlane);
                        }
                        else
                        {
                            // 頭以外でドラッグし始めたら何もしない（クリックも無効）
                            bDragging = false;
                            bDraggingHead = false;
                            CurrentTarget.Reset();
                        }

                    }
                }
            }
        }
    }
    else
    {
        // FPS中は保険で状態クリアのみ
        if (bPointerDown || bDragStarted || bDragging)
            CancelAllPointerStates();
    }
    // 既存：ドラッグ中の更新
    if (bDragging && CurrentTarget.IsValid())
    {
        UpdateDrag(DeltaSeconds);
    }
}

void AFirstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr) return;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (IsValid(MenuMappingContext))
		{
			//MappingContextを登録
			InputSystem->AddMappingContext(MenuMappingContext, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Completed, this, &ABlasterController::ShowReturnToMainMenu);
	}
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFirstPlayerController::OnLeftClick);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFirstPlayerController::OnLeftRelease); 

    // Tab で UI <=> FPS 切替
    InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AFirstPlayerController::OnTabPressed);
}

void AFirstPlayerController::InitCursorWidget()
{
    if (CursorClass)
    {
        CursorInst = CreateWidget<UUserWidget>(this, CursorClass);

        // カーソルとして登録（Defaultタイプに一つだけ）
        SetMouseCursorWidget(EMouseCursor::Default, CursorInst);
        CurrentMouseCursor = EMouseCursor::Default;
    }
}

void AFirstPlayerController::UpdateHoverCursor()
{
    if (!IsHUDModeActive()) return; // 念のための二重ガード

    FHitResult Hit;
    const bool bHit = GetHoverHitResultUnderCursor(Hit); // ← ここを差し替え

    bool bOverHeadNow = false;

    if (bHit)
    {
        if (UPrimitiveComponent* Comp = Hit.GetComponent())
        {
            const auto Resp = Comp->GetCollisionResponseToChannel(ECC_CursorCollision);
            UE_LOG(LogTemp, Warning, TEXT("Hit Comp %s HoverTrace Response = %d"), *GetNameSafe(Comp), (int32)Resp);
        }

        if (ACharacter* Ch = Cast<ACharacter>(Hit.GetActor()))
        {
            if (USkeletalMeshComponent* Sk = Ch->GetMesh())
            {
                if (Hit.BoneName == HeadBoneName)
                {
                    bOverHeadNow = true;
                }
                else
                {
                    const FVector Head = Sk->GetSocketLocation(HeadBoneName);
                    bOverHeadNow = FVector::DistSquared(Head, Hit.ImpactPoint) <= FMath::Square(HeadHitTolerance);
                }
            }
        }
    }

    if (bOverHeadNow != bCursorOverHead)
    {
        bCursorOverHead = bOverHeadNow;
    }
}


bool AFirstPlayerController::GetHoverHitResultUnderCursor(FHitResult& OutHit, float TraceDistance) const
{
    FVector WorldOrigin, WorldDir;
    // 画面上のマウス座標 → ワールドのレイに変換
    if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDir))
    {
        return false;
    }

    const FVector Start = WorldOrigin;
    const FVector End = Start + WorldDir * TraceDistance;

    // ★ 自分自身を無視する
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HoverTrace), /*bTraceComplex=*/true);
    Params.bReturnPhysicalMaterial = false;

    if (const APawn* MyPawn = GetPawn())
    {
        Params.AddIgnoredActor(MyPawn);

        // （任意）Pawnに付いている子アクターも無視したい場合
        TArray<AActor*> AttachedActors;
        MyPawn->GetAttachedActors(AttachedActors);
        for (AActor* A : AttachedActors)
        {
            Params.AddIgnoredActor(A);
        }
    }

    // 実トレース
    const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_CursorCollision, Params);

    // （任意）デバッグ可視化
     DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.02f, 0, 0.5f);

    return bHit;
}



void AFirstPlayerController::OnTabPressed()
{
    if (bDragging) { OnLeftRelease(); } // 既に追加済みならOK

    // UIへ入るとき
    if (!bUIInputMode)
    {
        bUIInputMode = true;
        ApplyInputMode();
        if (HUD) { HUD->ShowHome(); }
        // 念のためポインタ状態をクリア
        CancelAllPointerStates();
        return;
    }

    // UI→FPSへ戻すとき
    CancelAllPointerStates();
    bUIInputMode = false;
    if (HUD) { HUD->ShowHome(); }
    ApplyInputMode();
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("2FPS"));
}

// FirstPlayerController.cpp
void AFirstPlayerController::OnLeftClick()
{
    // HUDモード以外では一切の“クリック検出”をしない
    if (!IsHUDModeActive()) return;

    FHitResult Hit;
    if (!GetHoverHitResultUnderCursor(Hit)) {
        CancelAllPointerStates();
        return;
    }

    // 押下状態を開始（クリックはまだ確定させない）
    bPointerDown = true;
    bDragStarted = false;

    float mx, my;
    if (GetMousePosition(mx, my)) {
        PointerDownScreenPos = FVector2D(mx, my);
    }

    // 押下時に HomeCharacter を覚えておく（クリック時に使う）
    if (AHomeCharacter* HC = Cast<AHomeCharacter>(Hit.GetActor()))
    {
        SetCurrentTarget(HC);
        bPressedOnHead = (Hit.GetComponent() == HC->Head) || (Hit.BoneName == HeadBoneName);
    }
    else
    {
        SetCurrentTarget(nullptr);
        bPressedOnHead = false;
    }

}

void AFirstPlayerController::ApplyInputMode()
{
    if (bUIInputMode)
    {
        // ★ UI操作モード（Home等を操作）
        bEnableMouseOverEvents = true;
        bEnableClickEvents = true;
        bShowMouseCursor = true;

        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);

        // ↓ UIのどこにフォーカスを置くか明示（無指定だと適当なWidgetがTabを飲み込む）
        if (HUD)
        {
            Mode.SetWidgetToFocus(HUD->TakeWidget());
        }
        SetInputMode(Mode);

        SetIgnoreLookInput(true);
        SetIgnoreMoveInput(true);
    }
    else
    {
        // ★ FPSモード（UIは触らない）
        bEnableMouseOverEvents = false;
        bEnableClickEvents = false;
        bShowMouseCursor = false;

        FInputModeGameOnly Mode;
        SetInputMode(Mode);

        SetIgnoreLookInput(false);
        SetIgnoreMoveInput(false);
    }

    // 必要に応じてIMC切替（Enhanced Input使用時・任意）
    /*
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (auto* Subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            if (bUIInputMode)
            {
                // Subsys->AddMappingContext(UI_IMC, 10);
            }
            else
            {
                // Subsys->RemoveMappingContext(UI_IMC);
            }
        }
    }
    */
}


void AFirstPlayerController::OnLeftRelease()
{
    // FPS中に離された場合：何も始まっていない想定だが、保険で全リセット
    if (!IsHUDModeActive())
    {
        CancelAllPointerStates();
        return;
    }

    // ドラッグ中だった → 終了のみ（クリックは発火しない）
    if (bDragStarted)
    {
        // ドラッグ終了。クリックは発火しない
        if (bDraggingHead && CurrentTarget.IsValid())
        { 
            CurrentTarget->ResetHeadPat(); 
        }
        bDragging = false;
    }
    else
    {
        // ドラッグしなかった → ここで初めてクリック判定を行う
        FHitResult Hit;
        if (GetHoverHitResultUnderCursor(Hit))
        {
            if (AHomeCharacter* HC = Cast<AHomeCharacter>(Hit.GetActor()))
            {
                SetCurrentTarget(HC); // ← クリックで選択も更新（HUD連携用）

                if (Hit.GetComponent() == HC->Head) { HC->TriggerHeadClick(); }
                else if (Hit.GetComponent() == HC->Body) { HC->TriggerBodyClick(); }
                else if (Hit.GetComponent() == HC->Breast_R) { HC->TriggerBreastRClick(); }
                else if (Hit.GetComponent() == HC->Breast_L) { HC->TriggerBreastLClick(); }
                else if (Hit.BoneName == HeadBoneName) { HC->TriggerHeadClick(); }
                else { HC->TriggerNothingClick(); }
            }
        }
    }

    CancelAllPointerStates();
}

void AFirstPlayerController::UpdateDrag(float /*DeltaSeconds*/)
{
    if (!bDraggingHead || !CurrentTarget.IsValid()) return;

    float mx, my; if (!GetMousePosition(mx, my)) return;
    const FVector2D Delta = FVector2D(mx, my) - DragStartScreenPos;

    const float RollDeg = Delta.Y * HeadDragSensitivity; // y => RollDeg
    const float PitchDeg = -Delta.X * HeadDragSensitivity; // x => PitchDeg

    if (AHomeCharacter* HC = CurrentTarget.Get())
    {
        HC->SetHeadPatAngles(RollDeg, PitchDeg, 1.f);
    }
}

bool AFirstPlayerController::ScreenPosToWorldOnPlane(const FVector2D& Screen, const FPlane& Plane, FVector& OutWorld) const
{
    FVector WorldOrigin, WorldDir;
    if (!DeprojectScreenPositionToWorld(Screen.X, Screen.Y, WorldOrigin, WorldDir))
        return false;

    // レイ（原点→遠点）と平面の交点
    const FVector End = WorldOrigin + WorldDir * 100000.f;
    const FVector Hit = FMath::LinePlaneIntersection(WorldOrigin, End, Plane);
    if (!FMath::IsFinite(Hit.X) || !FMath::IsFinite(Hit.Y) || !FMath::IsFinite(Hit.Z))
        return false;

    OutWorld = Hit;
    return true;
}

void AFirstPlayerController::SetCurrentTarget(AHomeCharacter* NewTarget)
{
    // 旧ターゲットの購読解除
    if (CurrentTarget.IsValid())
    {
        if (auto* OldS = CurrentTarget->Status)
        {
            if (OldS->OnStatusChanged.IsAlreadyBound(this, &AFirstPlayerController::SyncSelectedToVM))
            {
                OldS->OnStatusChanged.RemoveDynamic(this, &AFirstPlayerController::SyncSelectedToVM);
            }
        }
    }
    CurrentTarget = NewTarget;

    // 新ターゲットへ購読追加（重複防止ガード）＆即時同期
    if (CurrentTarget.IsValid())
    {
        if (auto* NewS = CurrentTarget->Status)
        {
            if (!NewS->OnStatusChanged.IsAlreadyBound(this, &AFirstPlayerController::SyncSelectedToVM))
            {
                NewS->OnStatusChanged.AddDynamic(this, &AFirstPlayerController::SyncSelectedToVM);
            }
        }
        SyncSelectedToVM();
    }
}