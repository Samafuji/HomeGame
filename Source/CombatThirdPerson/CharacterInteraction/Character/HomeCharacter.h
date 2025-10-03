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
	// クラス内（public）
	UFUNCTION(BlueprintCallable, Category = "Interaction|HeadPat")
	void SetHeadPatAngles(float YawDeg, float PitchDeg, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "Interaction|HeadPat")
	void ResetHeadPat();

	// 可動範囲（必要ならEditAnywhereに）
	float HeadRollMax = 15.f; // うなずき
	float HeadPitchMax = 10.f; // 左右

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life|VM")
	UTaskViewModel* TaskViewModelRef = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// クリック時に流すモーション（エディタで割当）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* HeadClickMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* BodyClickMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* Breast_RMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Home|Anim")
	UAnimMontage* Breast_LMontage;

	// 任意：同じモーションでもセクション切替したい場合
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
	/** CapsuleComponent（ルート）は ACharacter が既に持っている */

	/** Box Collision（Capsuleの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxCollision;

	/** Arrow-AttachPoint（Capsuleの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* Arrow_AttachPoint;

	/** Widget（Arrowの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	/** TextRender_Dialog（Capsuleの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* TextRender_Dialog;

	/** Camera_Dialog（Capsuleの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera_Dialog;

	/** Mesh(CharacterMesh0) は ACharacter が既に "CharacterMesh0" 名で所持 */

	/** Head（Meshの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Head;

	/** Body（Meshの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Body;

	/** Breast_R（Meshの子） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Breast_R;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Breast_L;


	/** CharacterMovement は ACharacter が既に所持 */

	/** StateTree コンポーネント */
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HomeCharacter", meta = (AllowPrivateAccess = "true"))
	UStateTreeComponent* StateTreeComponent;*/

};
