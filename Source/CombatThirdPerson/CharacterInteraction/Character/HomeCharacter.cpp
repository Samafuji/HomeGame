// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInteraction/Character/HomeCharacter.h"
#include "Animation/AnimInstance.h"
#include "HomeAnimInstance.h"
#include "Home/TaskViewModel.h"
#include "Home/Components/CharacterStatusComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// StateTree（プラグインを有効化しておく）
//#include "StateTreeComponent.h"

// Sets default values
AHomeCharacter::AHomeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** ルートは既定の CapsuleComponent */
	UCapsuleComponent* RootCapsule = GetCapsuleComponent();
	check(RootCapsule);

	/** Mesh（ACharacter既定名 "CharacterMesh0"） */
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	// メッシュをCapsuleの少し前に
	MeshComp->SetupAttachment(RootCapsule);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); // 横向きアニメ用なら調整

	/** Box Collision（Capsuleの子） */
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(RootCapsule);
	BoxCollision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	BoxCollision->SetRelativeLocation(FVector(80.f, 0.f, 40.f));
	BoxCollision->SetCollisionProfileName(TEXT("BlockAll"));
	// HoverTrace を無視したいなら後で BP かコードで応答変更

	/** Arrow-AttachPoint（Capsuleの子） */
	Arrow_AttachPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow_AttachPoint"));
	Arrow_AttachPoint->SetupAttachment(RootCapsule);
	Arrow_AttachPoint->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	/** Widget（Arrowの子） */
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	Widget->SetupAttachment(Arrow_AttachPoint);
	Widget->SetWidgetSpace(EWidgetSpace::World);
	Widget->SetDrawSize(FVector2D(400.f, 100.f));
	Widget->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Widget->SetPivot(FVector2D(0.5f, 0.5f));
	// WidgetClass はエディタで割り当て

	/** TextRender_Dialog（Capsuleの子） */
	TextRender_Dialog = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender_Dialog"));
	TextRender_Dialog->SetupAttachment(RootCapsule);
	TextRender_Dialog->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	TextRender_Dialog->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRender_Dialog->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TextRender_Dialog->SetWorldSize(24.f);
	TextRender_Dialog->SetText(FText::FromString(TEXT("Hello")));

	/** Camera_Dialog（Capsuleの子） */
	Camera_Dialog = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_Dialog"));
	Camera_Dialog->SetupAttachment(RootCapsule);
	Camera_Dialog->SetRelativeLocation(FVector(-200.f, 0.f, 120.f));
	Camera_Dialog->SetRelativeRotation(FRotator(-5.f, 0.f, 0.f));
	Camera_Dialog->bUsePawnControlRotation = false;

	/** Head（Meshの子） */
	Head = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Head"));
	Head->SetupAttachment(MeshComp);
	Head->SetCapsuleHalfHeight(15.f);
	Head->SetCapsuleRadius(12.f);
	Head->SetRelativeLocation(FVector(0.f, 0.f, 160.f)); // 必要なら頭ボーンに Socket 親子付けも可
	Head->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // クリック/Overlap 等に応じて調整
	Head->SetGenerateOverlapEvents(true);

	/** Body（Meshの子） */
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	Body->SetupAttachment(MeshComp);
	Body->SetCapsuleHalfHeight(55.f);
	Body->SetCapsuleRadius(30.f);
	Body->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	Body->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Body->SetGenerateOverlapEvents(true);

	/** Body（Meshの子） */
	Breast_R = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Breast_R"));
	Breast_R->SetupAttachment(MeshComp);
	Breast_R->SetCapsuleHalfHeight(55.f);
	Breast_R->SetCapsuleRadius(30.f);
	Breast_R->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	Breast_R->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Breast_R->SetGenerateOverlapEvents(true);

	/** Body（Meshの子） */
	Breast_L = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Breast_L"));
	Breast_L->SetupAttachment(MeshComp);
	Breast_L->SetCapsuleHalfHeight(55.f);
	Breast_L->SetCapsuleRadius(30.f);
	Breast_L->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	Breast_L->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Breast_L->SetGenerateOverlapEvents(true);

	/** Component Status **/
	Status = CreateDefaultSubobject<UCharacterStatusComponent>(TEXT("Status"));
	/** Character Movement（既定で存在）を少し調整（必要なら） */
	UCharacterMovementComponent* Move = GetCharacterMovement();
	check(Move);
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	bUseControllerRotationYaw = false;

	/** StateTree コンポーネント */
	//StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	// 実行時にアセットを割り当てる場合はエディタ上で StateTree を設定（またはコードでロード）
}

// Called when the game starts or when spawned
void AHomeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// ★ OnClicked デリゲートにバインド（最短）
	if (Head)
	{
		//Head->OnClicked.AddDynamic(this, &AHomeCharacter::TriggerHeadClick);
	}
	if (Body)
	{
		//Body->OnClicked.AddDynamic(this, &AHomeCharacter::TriggerBodyClick);
	}
}

// Called every frame
void AHomeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AHomeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

static void PlayMontageSafe(USkeletalMeshComponent* Mesh, UAnimMontage* Montage, const FName& Section)
{
	if (!Mesh || !Montage) return;
	if (UAnimInstance* Anim = Mesh->GetAnimInstance())
	{
		float Len = Anim->Montage_Play(Montage, 1.0f);
		if (Len > 0.f && Section != NAME_None)
		{
			Anim->Montage_JumpToSection(Section, Montage);
		}
	}
}

void AHomeCharacter::TriggerNothingClick()
{
	if (Status) Status->AddAffection(Aff_Nothing); // ← 何もしない領域は Aff_Nothing
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Nothing Click"));
}
void AHomeCharacter::TriggerHeadClick()
{
	PlayMontageSafe(GetMesh(), HeadClickMontage, HeadMontageSection);
	if (Status) Status->AddAffection(Aff_Head);    // ← 頭クリックは Aff_Head
}

void AHomeCharacter::TriggerBodyClick()
{
	PlayMontageSafe(GetMesh(), BodyClickMontage, BodyMontageSection);
	if (Status) Status->AddAffection(Aff_Body);    // ← 体クリックは Aff_Body
}

void AHomeCharacter::TriggerBreastRClick()
{
	PlayMontageSafe(GetMesh(), Breast_RMontage, Breast_RMontageSection);
	if (Status) Status->AddAffection(Aff_BreastR);
}

void AHomeCharacter::TriggerBreastLClick()
{
	PlayMontageSafe(GetMesh(), Breast_LMontage, Breast_LMontageSection);
	if (Status) Status->AddAffection(Aff_BreastL);
}

// set bone
void AHomeCharacter::SetHeadPatAngles(float RollDeg, float PitchDeg, float Alpha)
{
	if (USkeletalMeshComponent* Sk = GetMesh())
	{
		if (UHomeAnimInstance* AI = Cast<UHomeAnimInstance>(Sk->GetAnimInstance()))
		{
			const float R = FMath::Clamp(RollDeg, -HeadRollMax, HeadRollMax);
			const float P = FMath::Clamp(PitchDeg, -HeadPitchMax, HeadPitchMax);

			/*AI->HeadPatRotCS = FRotator(P, Y, 0.f);
			AI->HeadPatAlpha = FMath::Clamp(Alpha, 0.f, 1.f);*/

			AI->HeadPatRotCS = FRotator(P, 0.f, R);
			AI->HeadPatAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
		}
	}
}

void AHomeCharacter::ResetHeadPat()
{
	if (USkeletalMeshComponent* Sk = GetMesh())
	{
		if (UHomeAnimInstance* AI = Cast<UHomeAnimInstance>(Sk->GetAnimInstance()))
		{
			AI->HeadPatRotCS = FRotator::ZeroRotator;
			AI->HeadPatAlpha = 0.f;
		}
	}
}