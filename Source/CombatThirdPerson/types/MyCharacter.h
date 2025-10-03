#pragma once
// C++: MyCharacter.h
UENUM(BlueprintType)
enum class ECharacterMood : uint8 {
	Neutral    UMETA(DisplayName = "Neutral"),
	Happy      UMETA(DisplayName = "Happy"),
	Sad        UMETA(DisplayName = "Sad"),
	Angry      UMETA(DisplayName = "Angry"),
	Surprised  UMETA(DisplayName = "Surprised")
};

//USTRUCT(BlueprintType)
//struct FDialogueLine {
	//GENERATED_BODY()

	/** Row name in the DataTable */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FName RowName;

	/** The text to display */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FText Text;

	/** Which mood to switch to after this line */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//ECharacterMood NextMood;

	/** Optional: which AnimMontage to play */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UAnimMontage* Montage;

	/** Optional: key for expression mapping */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FName ExpressionKey;

	/** Optional: names of next rows (for choice branches) */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TArray<FName> Choices;
//};
