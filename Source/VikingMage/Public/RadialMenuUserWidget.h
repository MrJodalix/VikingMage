// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"

#include "RadialMenuUserWidget.generated.h"

/**
 * A basic universal usable radial menu selection
 */
UCLASS()
class VIKINGMAGE_API URadialMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
		UTexture* T_CursorTexture;

	/**Size of each item on the canvas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector2D ItemSize = (100, 100);
	
	UPROPERTY(BlueprintReadOnly)
		FVector2D CenterPoint;

	/** Size of the viewport */
	UPROPERTY(BlueprintReadWrite)
		FVector2D ViewportSize;

	/**angle to mouse */
	UPROPERTY(BlueprintReadWrite)
		float RotationToMouse;

	/**Vector to first element */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D GCenterVectorVertical = FVector2D(0, 1);

protected:

	/**Texture for Cursor */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UImage* CursorImage;


	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


	UFUNCTION(BlueprintCallable)
		void CalcCenterPoint();

	UFUNCTION(BlueprintCallable)
		FVector2D GetViewportSize();

};
