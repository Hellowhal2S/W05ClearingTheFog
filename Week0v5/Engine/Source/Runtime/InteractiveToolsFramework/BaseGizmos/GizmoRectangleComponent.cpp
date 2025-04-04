#include "GizmoRectangleComponent.h"

#include "Engine/World.h"
#include "Actors/Player.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"


UGizmoRectangleComponent::UGizmoRectangleComponent()
{
}

UGizmoRectangleComponent::~UGizmoRectangleComponent()
{
}

void UGizmoRectangleComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UGizmoRectangleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}