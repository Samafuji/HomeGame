// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterInteraction/Character/HomeAnimInstance.h"
#include "HomeCharacter.generated.h"

class UBoxComponent;
class UArrowComponent;
class UWidgetComponent;
class UTextRenderComponent;
class UCameraComponent;
class UStateTreeComponent;
class UCapsuleComponent;
class UTaskViewModel;
class UCharacterStatusComponent;

UCLASS()
class COMBATTHIRDPERSON_API AHomeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHomeCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life|Status")
	UCharacterStatusComponent* Status;

	UPROPERTY(EditAnywhere, Category = "Life|Meta") FName CharacterId = "Default01";

	UPROPERTY(EditAnywhere, Category = "Life|Affection") int32 Aff_Head = +2;
	UPROPERTY(EditAnywhere, Category = "Life|Affection") int32 Aff_Body = +1;
	UPROPERTY(EditAnywhere, Category = "Life|Affection") int32 Aff_BreastR = -4;
	UPROPERTY(EditAnywhere, Category = "Life|Affection") int32 Aff_BreastL = -4;
	UPROPERTY(EditAnywhere, Category = "Life|Affection") int32 Aff_Nothing = -1;
	UFUNCTION()
	void TriggerNothingClick();

	// Click motion
	UFUNCTION()
	void TriggerHeadClick();

	UFUNCTION()
	void TriggerBodyClick();

	UFUNCTION()
	void TriggerBreastRClick();
	UFUNCTION()
	void TriggerBreastLClick();

	///////////////////// Interaction Bone
	// �N���X���ipublic�j
	UFUNCTION(BlueprintCallable, Category = "Interaction|HeadPat")
	void SetHeadPatAngles(float YawDeg, float PitchDeg, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "Interaction|HeadPat")
	void ResetHeadPat();

	// ���͈́i�K�v�Ȃ�EditAnywhere�Ɂj
	float HeadRollMax = 15.f; // ���Ȃ���
	float HeadPitchMax = 10.f; // ���E

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life|VM")
	UTaskViewModel* TaskViewModelRef = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// �N���b�N���ɗ������[�V�����i�G�f�B�^�Ŋ����j
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* HeadClickMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* BodyClickMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* Breast_RMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* Breast_LMontage;

	// �C�ӁF�������[�V�����ł��Z�N�V�����ؑւ������ꍇ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	FName HeadMontageSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	FName BodyMontageSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	FName Breast_RMontageSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	FName Breast_LMontageSection = NAME_None;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** CapsuleComponent�i���[�g�j�� ACharacter �����Ɏ����Ă��� */

	/** Box Collision�iCapsule�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxCollision;

	/** Arrow-AttachPoint�iCapsule�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* Arrow_AttachPoint;

	/** Widget�iArrow�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	/** TextRender_Dialog�iCapsule�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* TextRender_Dialog;

	/** Camera_Dialog�iCapsule�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera_Dialog;

	/** Mesh(CharacterMesh0) �� ACharacter ������ "CharacterMesh0" ���ŏ��� */

	/** Head�iMesh�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Head;

	/** Body�iMesh�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Body;

	/** Breast_R�iMesh�̎q�j */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Breast_R;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Breast_L;


	/** CharacterMovement �� ACharacter �����ɏ��� */

	/** StateTree �R���|�[�l���g */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UStateTreeComponent* StateTreeComponent;*/

};
