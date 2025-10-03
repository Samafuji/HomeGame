// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInteraction/FirstPlayerController.h"
#include "CombatThirdPerson.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h" // �f�o�b�O�\���������ꍇ


#include "Character/HomeCharacter.h"
#include "Components/CapsuleComponent.h" // �� �Y�ꂸ��

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

// �N���X�Q�Ƃ�BP����ݒ�FTSubclassOf<UUserWidget> HUDClass; �Ȃǂ�UPROPERTY�ŗp�ӂ��Ă���
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

            // �� �X�e�l�~���[�i�����ǂ���j
            TaskVM->Affection = Snap.Affection;
            TaskVM->Mood = Snap.Mood;
            TaskVM->Focus = Snap.Focus;
            TaskVM->Coin = Snap.Coin;
            TaskVM->HeartValue = Snap.Heart;   // �� HeartWidget ���ǂރv���p�e�B���ɍ��킹��

            // �� ���^���i���O/ID�j���Z�b�g
            const FName CharId = HC->GetFName(); // ID �t�B�[���h������Ȃ� HC->CharacterId �𐄏�
#if WITH_EDITOR
            const FText CharName = FText::FromString(HC->GetActorLabel()); // Editor ��
#else
            const FText CharName = FText::FromString(HC->GetName()); // ���s���͖��O
#endif
            TaskVM->SetSelectedCharacterMeta(CharId, CharName);

            // �Ō�ɒʒm�iSetSelectedCharacterMeta ���� Broadcast �ς݂Ȃ�ȗ��j
            TaskVM->OnStatsChanged.Broadcast();
        }
    }
    else
    {
        // ���I����ԁi�C�Ӂj
        TaskVM->SelectedCharacterId = NAME_None;
        TaskVM->SelectedCharacterName = FText::FromString(TEXT("Not Selected"));
        TaskVM->OnStatsChanged.Broadcast();
    }
}

void AFirstPlayerController::BeginPlay()
{
	Super::BeginPlay();

    // FPS�f�t�H���g�FUI����
    bUIInputMode = false;
    ApplyInputMode();

	/*bEnableMouseOverEvents = true;
	bEnableClickEvents = true;*/
    //bShowMouseCursor = true;
/*
	// UI Only�i�K�v�Ȃ�t�H�[�J�X����E�B�W�F�b�g��ݒ�j
	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);*/

    InitCursorWidget();

    TaskVM = NewObject<UTaskViewModel>(this);

    // �����Z�[�u�D��ŕ����i������� DataTable ����V�K�j
    const bool bLoaded = TaskVM->LoadFromSlot(TaskTable, TEXT("TaskVM"), 0, /*bAppend=*/false);

    if (TaskVM)
    {
        // �� �N���u�ԂɁg04:00�ׂ��ς݂��h�����O�`�F�b�N���đ����Z�b�g
        TaskVM->EnsureFreshDailyWeeklyOnLaunch();

        // �Ȍ�͖���04:00�Ɏ������΁i�]���ǂ���w�ǁj
        if (UWorld* W = GetWorld()) {
            if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>()) {
                Cycle->OnDailyReset.AddDynamic(TaskVM, &UTaskViewModel::HandleDailyReset);
            }
        }
        
        // ���̌�� DataTable ����u�s���������v�ǉ�����
        if (TaskTable)
        {
            const int32 Added = TaskVM->MergeFromDataTable(TaskTable);
            if (Added > 0 && GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                    *FString::Printf(TEXT("New tasks added: %d"), Added));
        }

        if (DailyCheckTable)
        {
            // ���ł��u�s���������v�ǉ�����i��̂Ƃ��͑S������j
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
        HUD->TaskViewModel = TaskVM;     // ExposeOnSpawn �Ȃ� SpawnParams���p�ł�OK
        HUD->AddToViewport();
    }

    if (AHomeCharacter* HC = Cast<AHomeCharacter>(GetCharacter()))
    {
        HC->TaskViewModelRef = TaskVM;  // �� ����ŃN���b�N���D���x���f�̓���������
    }

    if (UWorld* W = GetWorld()) {
        if (auto* Cycle = W->GetSubsystem<UDailyCycleSubsystem>()) {
            Cycle->OnDailyReset.AddDynamic(TaskVM, &UTaskViewModel::HandleDailyReset);
        }
    }

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("PlayerController Init"));
}

// �i�C�Ӂj�I�����ɃI�[�g�Z�[�u
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

    // �� HUD���[�h�̂Ƃ������z�o�[�X�V���s��
    if (IsHUDModeActive())
    {
        UpdateHoverCursor();

        // �܂��h���b�O�J�n���Ă��Ȃ� �� �������l�`�F�b�N
        if (bPointerDown && !bDragStarted)
        {
            if (!IsHUDModeActive())
            {
                // HUD���[�h�łȂ��̂Ƀh���b�O��ԂɂȂ�Ȃ��悤�ɕی�
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
                        // �������Ńh���b�O�J�n�m�聚
                        bDragStarted = true;

                        if (CurrentTarget.IsValid() && bPressedOnHead)
                        {
                            bDragging = true;
                            bDraggingHead = true;   // �� ���ꂪ�d�v�I

                            if (HUD) { HUD->ShowControlPage(false); }

                            // ���ȂŊ�̋L�^
                            DragStartScreenPos = PointerDownScreenPos;
                            DragStartActorLocation = CurrentTarget->GetActorLocation();
                            DragPlane = FPlane(DragStartActorLocation, FVector::UpVector);
                            ScreenPosToWorldOnPlane(DragStartScreenPos, DragPlane, DragStartWorldOnPlane);
                        }
                        else
                        {
                            // ���ȊO�Ńh���b�O���n�߂��牽�����Ȃ��i�N���b�N�������j
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
        // FPS���͕ی��ŏ�ԃN���A�̂�
        if (bPointerDown || bDragStarted || bDragging)
            CancelAllPointerStates();
    }
    // �����F�h���b�O���̍X�V
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
			//MappingContext��o�^
			InputSystem->AddMappingContext(MenuMappingContext, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Completed, this, &ABlasterController::ShowReturnToMainMenu);
	}
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFirstPlayerController::OnLeftClick);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFirstPlayerController::OnLeftRelease); 

    // Tab �� UI <=> FPS �ؑ�
    InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AFirstPlayerController::OnTabPressed);
}

void AFirstPlayerController::InitCursorWidget()
{
    if (CursorClass)
    {
        CursorInst = CreateWidget<UUserWidget>(this, CursorClass);

        // �J�[�\���Ƃ��ēo�^�iDefault�^�C�v�Ɉ�����j
        SetMouseCursorWidget(EMouseCursor::Default, CursorInst);
        CurrentMouseCursor = EMouseCursor::Default;
    }
}

void AFirstPlayerController::UpdateHoverCursor()
{
    if (!IsHUDModeActive()) return; // �O�̂��߂̓�d�K�[�h

    FHitResult Hit;
    const bool bHit = GetHoverHitResultUnderCursor(Hit); // �� �����������ւ�

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
    // ��ʏ�̃}�E�X���W �� ���[���h�̃��C�ɕϊ�
    if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDir))
    {
        return false;
    }

    const FVector Start = WorldOrigin;
    const FVector End = Start + WorldDir * TraceDistance;

    // �� �������g�𖳎�����
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HoverTrace), /*bTraceComplex=*/true);
    Params.bReturnPhysicalMaterial = false;

    if (const APawn* MyPawn = GetPawn())
    {
        Params.AddIgnoredActor(MyPawn);

        // �i�C�ӁjPawn�ɕt���Ă���q�A�N�^�[�������������ꍇ
        TArray<AActor*> AttachedActors;
        MyPawn->GetAttachedActors(AttachedActors);
        for (AActor* A : AttachedActors)
        {
            Params.AddIgnoredActor(A);
        }
    }

    // ���g���[�X
    const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_CursorCollision, Params);

    // �i�C�Ӂj�f�o�b�O����
     DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.02f, 0, 0.5f);

    return bHit;
}



void AFirstPlayerController::OnTabPressed()
{
    if (bDragging) { OnLeftRelease(); } // ���ɒǉ��ς݂Ȃ�OK

    // UI�֓���Ƃ�
    if (!bUIInputMode)
    {
        bUIInputMode = true;
        ApplyInputMode();
        if (HUD) { HUD->ShowHome(); }
        // �O�̂��߃|�C���^��Ԃ��N���A
        CancelAllPointerStates();
        return;
    }

    // UI��FPS�֖߂��Ƃ�
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
    // HUD���[�h�ȊO�ł͈�؂́g�N���b�N���o�h�����Ȃ�
    if (!IsHUDModeActive()) return;

    FHitResult Hit;
    if (!GetHoverHitResultUnderCursor(Hit)) {
        CancelAllPointerStates();
        return;
    }

    // ������Ԃ��J�n�i�N���b�N�͂܂��m�肳���Ȃ��j
    bPointerDown = true;
    bDragStarted = false;

    float mx, my;
    if (GetMousePosition(mx, my)) {
        PointerDownScreenPos = FVector2D(mx, my);
    }

    // �������� HomeCharacter ���o���Ă����i�N���b�N���Ɏg���j
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
        // �� UI���샂�[�h�iHome���𑀍�j
        bEnableMouseOverEvents = true;
        bEnableClickEvents = true;
        bShowMouseCursor = true;

        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);

        // �� UI�̂ǂ��Ƀt�H�[�J�X��u���������i���w�肾�ƓK����Widget��Tab�����ݍ��ށj
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
        // �� FPS���[�h�iUI�͐G��Ȃ��j
        bEnableMouseOverEvents = false;
        bEnableClickEvents = false;
        bShowMouseCursor = false;

        FInputModeGameOnly Mode;
        SetInputMode(Mode);

        SetIgnoreLookInput(false);
        SetIgnoreMoveInput(false);
    }

    // �K�v�ɉ�����IMC�ؑցiEnhanced Input�g�p���E�C�Ӂj
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
    // FPS���ɗ����ꂽ�ꍇ�F�����n�܂��Ă��Ȃ��z�肾���A�ی��őS���Z�b�g
    if (!IsHUDModeActive())
    {
        CancelAllPointerStates();
        return;
    }

    // �h���b�O�������� �� �I���̂݁i�N���b�N�͔��΂��Ȃ��j
    if (bDragStarted)
    {
        // �h���b�O�I���B�N���b�N�͔��΂��Ȃ�
        if (bDraggingHead && CurrentTarget.IsValid())
        { 
            CurrentTarget->ResetHeadPat(); 
        }
        bDragging = false;
    }
    else
    {
        // �h���b�O���Ȃ����� �� �����ŏ��߂ăN���b�N������s��
        FHitResult Hit;
        if (GetHoverHitResultUnderCursor(Hit))
        {
            if (AHomeCharacter* HC = Cast<AHomeCharacter>(Hit.GetActor()))
            {
                SetCurrentTarget(HC); // �� �N���b�N�őI�����X�V�iHUD�A�g�p�j

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

    // ���C�i���_�����_�j�ƕ��ʂ̌�_
    const FVector End = WorldOrigin + WorldDir * 100000.f;
    const FVector Hit = FMath::LinePlaneIntersection(WorldOrigin, End, Plane);
    if (!FMath::IsFinite(Hit.X) || !FMath::IsFinite(Hit.Y) || !FMath::IsFinite(Hit.Z))
        return false;

    OutWorld = Hit;
    return true;
}

void AFirstPlayerController::SetCurrentTarget(AHomeCharacter* NewTarget)
{
    // ���^�[�Q�b�g�̍w�ǉ���
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

    // �V�^�[�Q�b�g�֍w�ǒǉ��i�d���h�~�K�[�h�j����������
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