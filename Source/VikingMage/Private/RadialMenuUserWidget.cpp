// Fill out your copyright notice in the Description page of Project Settings.


#include "RadialMenuUserWidget.h"

#include "VectorBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/UserInterfaceSettings.h"

FVector2D URadialMenuUserWidget::GetViewportSize()
{
	FVector2D Result(0, 0);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Result);
	}

	return Result;
}

/**
 *Gets the mouse position on the viewport
 *
 *@return true, if position was found 
 */
bool GetMouseOnViewport(FVector2D& MouseOnViewport)
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetMousePosition(MouseOnViewport);
		return true;
	}
	return false;
}

void URadialMenuUserWidget::CalcCenterPoint()
{
	CenterPoint = ViewportSize / 2.0f;
}

void URadialMenuUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CalcCenterPoint();

	//Set Cursor Texture
	if (CursorImage && CursorImage->GetDynamicMaterial()) {
		CursorImage->GetDynamicMaterial()->SetTextureParameterValue("Image", T_CursorTexture);
	}	
}

void URadialMenuUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//Get Direction to Mouse from Center
	if (this->GetIsEnabled()) {

		FVector2D MousePosition;
		FVector2D DirectionToMouse;
		CalcCenterPoint();

		if (GetMouseOnViewport(MousePosition))
		{
			DirectionToMouse = MousePosition - CenterPoint;
			DirectionToMouse.Normalize();
		}
		
		//Set RotationToMouse
		RotationToMouse = UVectorBlueprintLibrary::CalcAngleBetweenVectors2D(DirectionToMouse, GCenterVectorVertical);

		if (DirectionToMouse.X >= 0)
		{
			RotationToMouse *= (-1);
		}

		CursorImage->SetRenderTransformAngle(RotationToMouse);	
	}
}






