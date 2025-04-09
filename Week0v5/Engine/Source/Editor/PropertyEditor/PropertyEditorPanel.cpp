#include "PropertyEditorPanel.h"

#include "Engine/World.h"
#include "Actors/Player.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/UText.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/RotatingMovementComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Math/MathUtility.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include <Components/CubeComp.h>
#include <Components/UParticleSubUVComp.h>

#include "Components/HeightFogComponent.h"

void PropertyEditorPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */    
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Detail", nullptr, PanelFlags);
    
    AEditorPlayer* player = GEngine->GetWorld()->GetEditorPlayer();
    AActor* PickedActor = GEngine->GetWorld()->GetSelectedActor();

    if (PickedActor && PickedActor != LastActor)
    {
        PickedComponent = PickedActor->GetRootComponent();
    }

    LastActor = PickedActor;

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor)
    {
        if (ImGui::TreeNodeEx("Components", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
        {
            DrawSceneComponentTree(PickedActor->GetRootComponent(), PickedComponent);
            //SceneComponent가 아닌 경우
            const TSet<UActorComponent*>& AllComponents = PickedActor->GetComponents();
            for (UActorComponent* Component : AllComponents)
            {
                if (!Component->IsA<USceneComponent>())
                {
                    FString Label = *Component->GetName();
                    bool bSelected = (PickedComponent == Component);

                    // 리프 노드 플래그 추가
                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;
                    if (bSelected)
                        nodeFlags |= ImGuiTreeNodeFlags_Selected;

                    // 노드 생성 (bOpened는 항상 false)
                    bool bOpened = ImGui::TreeNodeEx(*Label, nodeFlags);

                    // 클릭 이벤트 처리
                    if (ImGui::IsItemClicked())
                    {
                        PickedComponent = Component;
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::Button("+", ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
            {
                ImGui::OpenPopup("AddComponentPopup");
            }

            // 팝업 메뉴
            if (ImGui::BeginPopup("AddComponentPopup"))
            {
                if (ImGui::Selectable("TextComponent"))
                {
                    UText* TextComponent = PickedActor->AddComponent<UText>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        TextComponent->DetachFromParent();
                        TextComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = TextComponent;
                    TextComponent->SetTexture(L"Assets/Texture/font.png");
                    TextComponent->SetRowColumnCount(106, 106);
                    TextComponent->SetText(L"안녕하세요 Jungle");
                }
                if (ImGui::Selectable("BillboardComponent"))    
                {
                    UBillboardComponent* BillboardComponent = PickedActor->AddComponent<UBillboardComponent>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        BillboardComponent->DetachFromParent();
                        BillboardComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = BillboardComponent;
                    BillboardComponent->SetTexture(L"Assets/Texture/Pawn_64x.png");
                    BillboardComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 3.0f));
                }
                if (ImGui::Selectable("LightComponent"))
                {
                    ULightComponentBase* LightComponent = PickedActor->AddComponent<ULightComponentBase>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        LightComponent->DetachFromParent();
                        LightComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = LightComponent;
                }
                if (ImGui::Selectable("ParticleComponent"))
                {
                    UParticleSubUVComp* ParticleComponent = PickedActor->AddComponent<UParticleSubUVComp>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        ParticleComponent->DetachFromParent();
                        ParticleComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = ParticleComponent;
                    ParticleComponent->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
                    ParticleComponent->SetRowColumnCount(6, 6);
                    ParticleComponent->SetRelativeScale(FVector(10.0f, 10.0f, 1.0f));
                    ParticleComponent->Activate();
                }
                if (ImGui::Selectable("StaticMeshComponent"))
                {
                    UStaticMeshComponent* StaticMeshComponent = PickedActor->AddComponent<UStaticMeshComponent>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        StaticMeshComponent->DetachFromParent();
                        StaticMeshComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = StaticMeshComponent;
                    FManagerOBJ::CreateStaticMesh("Assets/Cube.obj");
                    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Cube.obj"));
                }
                //if (ImGui::Selectable("CubeComponent"))
                //{
                //    UCubeComp* CubeComponent = PickedActor->AddComponent<UCubeComp>();
                //    PickedComponent = CubeComponent;
                //}
                if (ImGui::Selectable("PointLightComponent"))
                {
                    UPointLightComponent* PointLightComponent = PickedActor->AddComponent<UPointLightComponent>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        PointLightComponent->DetachFromParent();
                        PointLightComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = PointLightComponent;
                }
                if (ImGui::Selectable("DirectionalLightComponent"))
                {
                    UDirectionalLightComponent* DirectionalLightComponent = PickedActor->AddComponent<UDirectionalLightComponent>();
                    if (USceneComponent* ParentComponent = Cast<USceneComponent>(PickedComponent))
                    {
                        DirectionalLightComponent->DetachFromParent();
                        DirectionalLightComponent->SetupAttachment(ParentComponent);
                    }
                    PickedComponent = DirectionalLightComponent;
                }
                if (ImGui::Selectable("ProjectileMovementComponent"))
                {
                    UProjectileMovementComponent* ProjectileMovementComponent = PickedActor->AddComponent<UProjectileMovementComponent>();
                    ProjectileMovementComponent->SetUpdatedComponent(PickedActor->GetRootComponent());
                    ProjectileMovementComponent->SetOwner(PickedActor);
                    PickedComponent = ProjectileMovementComponent;
                }
                if (ImGui::Selectable("RotatingMovementComponent"))
                {
                    URotatingMovementComponent* RotatingMovementComponent = PickedActor->AddComponent<URotatingMovementComponent>();
                    RotatingMovementComponent->SetUpdatedComponent(PickedActor->GetRootComponent());
                    RotatingMovementComponent->SetOwner(PickedActor);
                    PickedComponent = RotatingMovementComponent;
                }
                ImGui::EndPopup();
            }
            ImGui::TreePop();
        }
    }

    if (PickedActor && PickedComponent && PickedComponent->IsA<UMovementComponent>())
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (PickedComponent != LastComponent)
        {
            LastComponent = PickedComponent;
            bFirstFrame = true;
        }
        if (UProjectileMovementComponent* ProjectileComp = Cast<UProjectileMovementComponent>(PickedComponent))
        {
            if (ImGui::CollapsingHeader("Projectile Movement Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Initial Speed",
                    &ProjectileComp->InitialSpeed,
                    1.0f, 0.0f, 10000.0f, "%.0f");
                ImGui::DragFloat("Max Speed",
                    &ProjectileComp->MaxSpeed,
                    1.0f, 0.0f, 10000.0f, "%.0f");
                FVector Velocity = ProjectileComp->Velocity;
                if (ImGui::DragFloat3("Velocity",
                    &Velocity.x,
                    0.1f, -10.0f, 10.0f, "%.3f"))
                {
                    ProjectileComp->Velocity = Velocity;
                }
                if (ImGui::DragFloat3("Acceleration",
                    &ProjectileComp->Acceleration.x,
                    0.00001f, -0.01f, 0.01f, "%.5f"))
                {
                }
            }
        }
        if (URotatingMovementComponent* RotatingComp = Cast<URotatingMovementComponent>(PickedComponent))
        {
            if (ImGui::CollapsingHeader("Rotating Movement Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                FVector CurrentEuler = RotatingComp->RotationRate;
                float Pitch = CurrentEuler.x;
                float Yaw = CurrentEuler.y;
                float Roll = CurrentEuler.z;

                ImGui::Text("Rotation Rate (deg/s)");
                ImGui::DragFloat("Pitch", &Pitch, 1.0f, -1000.0f, 1000.0f, "%.1f");
                ImGui::DragFloat("Yaw", &Yaw, 1.0f, -1000.0f, 1000.0f, "%.1f");
                ImGui::DragFloat("Roll", &Roll, 1.0f, -1000.0f, 1000.0f, "%.1f");

                RotatingComp->RotationRate = FVector(Pitch, Yaw, Roll);

                FVector Pivot = RotatingComp->PivotTranslation;
                if (ImGui::DragFloat3("Pivot Translation", &Pivot.x, 1.0f, -1000.0f, 1000.0f))
                {
                    RotatingComp->PivotTranslation = Pivot;
                }

                bool bLocalSpace = RotatingComp->bRotationInLocalSpace;
                if (ImGui::Checkbox("Rotation in Local Space", &bLocalSpace))
                {
                    RotatingComp->bRotationInLocalSpace = bLocalSpace;
                }
            }
        }
        ImGui::PopStyleColor();
    }

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor && PickedComponent && PickedComponent->IsA<USceneComponent>())
    {
        USceneComponent* SceneComp = Cast<USceneComponent>(PickedComponent);
        ImGui::SetItemDefaultFocus();
        // TreeNode 배경색을 변경 (기본 상태)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            if (PickedComponent != LastComponent)
            {
                LastComponent = PickedComponent;
                bFirstFrame = true;
                Location = SceneComp->GetRelativeLocation();
                Rotation = SceneComp->GetRelativeRotation();
                Scale = SceneComp->GetRelativeScale();
            }

            bool bChanged = false;

            bChanged |= FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
            ImGui::Spacing();

            bChanged |= FImGuiWidget::DrawVec3Control("Rotation", Rotation, 0, 85);
            ImGui::Spacing();

            bChanged |= FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
            ImGui::Spacing();

            if (bChanged && !bFirstFrame)
            {
                SceneComp->SetRelativeLocation(Location);
                SceneComp->SetRelativeRotation(Rotation);
                SceneComp->SetRelativeScale(Scale);
            }

            std::string coordiButtonLabel;
            if (player->GetCoordiMode() == CoordiMode::CDM_WORLD)
                coordiButtonLabel = "World";
            else if (player->GetCoordiMode() == CoordiMode::CDM_LOCAL)
                coordiButtonLabel = "Local";

            if (ImGui::Button(coordiButtonLabel.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
            {
                player->AddCoordiMode();
            }
            ImGui::TreePop(); // 트리 닫기
        }
        ImGui::PopStyleColor();
        bFirstFrame = false;
    }

    if (PickedActor && PickedComponent && PickedComponent->IsA<UPointLightComponent>())
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (PickedComponent != LastComponent)
        {
            LastComponent = PickedComponent;
            bFirstFrame = true;
        }
        UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(PickedComponent);
        if (PointLightComp)
        {
            if (ImGui::CollapsingHeader("Point Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                float Intensity = PointLightComp->GetIntensity();
                float Radius = PointLightComp->GetRadius();
                float RadiusFallOff = PointLightComp->GetRadiusFallOff();
                ImGui::DragFloat("Intensity", &Intensity, 2.f, 0.0f, 5000.0f, "%.0f");
                ImGui::DragFloat("Radius", &Radius, 0.1f, 0.0f, 500.0f, "%.1f");
                ImGui::DragFloat("Falloff", &RadiusFallOff, 0.01f, 0.01f, 10.0f, "%.2f");
                FLinearColor Color = PointLightComp->GetColor();
                float colorArray[4] = { Color.R, Color.G, Color.B, Color.A };
                if (ImGui::ColorEdit4("Color", colorArray, ImGuiColorEditFlags_Float))
                {
                    Color.R = colorArray[0];
                    Color.G = colorArray[1];
                    Color.B = colorArray[2];
                    Color.A = colorArray[3];
                }

                PointLightComp->SetIntensity(Intensity);
                PointLightComp->SetRadius(Radius);
                PointLightComp->SetRadiusFallOff(RadiusFallOff);
                PointLightComp->SetColor({ colorArray[0], colorArray[1], colorArray[2], colorArray[3] });
            }
        }
        ImGui::PopStyleColor();
    }

    if (PickedActor && PickedComponent && PickedComponent->IsA<UDirectionalLightComponent>())
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (PickedComponent != LastComponent)
        {
            LastComponent = PickedComponent;
            bFirstFrame = true;
        }
        UDirectionalLightComponent* PointLightComp = Cast<UDirectionalLightComponent>(PickedComponent);
        if (PointLightComp)
        {
            if (ImGui::CollapsingHeader("Point Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                float Intensity = PointLightComp->GetIntensity();
                ImGui::DragFloat("Intensity", &Intensity, 2.f, 0.0f, 5000.0f, "%.0f");
                FLinearColor Color = PointLightComp->GetColor();
                float colorArray[4] = { Color.R, Color.G, Color.B, Color.A };
                if (ImGui::ColorEdit4("Color", colorArray, ImGuiColorEditFlags_Float))
                {
                    Color.R = colorArray[0];
                    Color.G = colorArray[1];
                    Color.B = colorArray[2];
                    Color.A = colorArray[3];
                }
                PointLightComp->SetIntensity(Intensity);
                PointLightComp->SetColor({ colorArray[0], colorArray[1], colorArray[2], colorArray[3] });
            }
        }
        ImGui::PopStyleColor();
    }

    //if (PickedActor && PickedComponent && PickedComponent->IsA<ULightComponentBase>())
    //{
    //    ULightComponentBase* lightObj = Cast<ULightComponentBase>(PickedComponent);
    //    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    //    if (ImGui::TreeNodeEx("SpotLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    //    {
    //        FVector4 currColor = lightObj->GetColor();

    //        float r = currColor.x;
    //        float g = currColor.y;
    //        float b = currColor.z;
    //        float a = currColor.w;
    //        float h, s, v;
    //        float lightColor[4] = { r, g, b, a };

    //        // SpotLight Color
    //        if (ImGui::ColorPicker4("##SpotLight Color", lightColor,
    //            ImGuiColorEditFlags_DisplayRGB |
    //            ImGuiColorEditFlags_NoSidePreview |
    //            ImGuiColorEditFlags_NoInputs |
    //            ImGuiColorEditFlags_Float))

    //        {

    //            r = lightColor[0];
    //            g = lightColor[1];
    //            b = lightColor[2];
    //            a = lightColor[3];
    //            lightObj->SetColor(FVector4(r, g, b, a));
    //        }
    //        RGBToHSV(r, g, b, h, s, v);
    //        // RGB/HSV
    //        bool changedRGB = false;
    //        bool changedHSV = false;

    //        // RGB
    //        ImGui::PushItemWidth(50.0f);
    //        if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
    //        ImGui::SameLine();
    //        if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
    //        ImGui::SameLine();
    //        if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
    //        ImGui::Spacing();
    //        
    //        // HSV
    //        if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
    //        ImGui::SameLine();
    //        if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
    //        ImGui::SameLine();
    //        if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
    //        ImGui::PopItemWidth();
    //        ImGui::Spacing();
    //        
    //        if (changedRGB && !changedHSV)
    //        {
    //            // RGB -> HSV
    //            RGBToHSV(r, g, b, h, s, v);
    //            lightObj->SetColor(FVector4(r, g, b, a));
    //        }
    //        else if (changedHSV && !changedRGB)
    //        {
    //            // HSV -> RGB
    //            HSVToRGB(h, s, v, r, g, b);
    //            lightObj->SetColor(FVector4(r, g, b, a));
    //        }

    //        // Light Radius
    //        float radiusVal = lightObj->GetRadius();
    //        if (ImGui::SliderFloat("Radius", &radiusVal, 1.0f, 100.0f))
    //        {
    //            lightObj->SetRadius(radiusVal);
    //        }
    //        ImGui::TreePop();
    //    }
    //    ImGui::PopStyleColor();
    //}

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor && PickedComponent && PickedComponent->IsA<UText>())
    {
        UText* textOBj = Cast<UText>(PickedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            if (textOBj) {
                textOBj->SetTexture(L"Assets/Texture/font.png");
                textOBj->SetRowColumnCount(106, 106);
                FWString wText = textOBj->GetText();
                int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string u8Text(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

                static char buf[256];
                strcpy_s(buf, u8Text.c_str());

                ImGui::Text("Text: ", buf);
                ImGui::SameLine();
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    textOBj->ClearText();
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                    FWString newWText(wlen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                    textOBj->SetText(newWText);
                }
                ImGui::PopItemFlag();
            }
            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor && PickedComponent && PickedComponent->IsA<UStaticMeshComponent>())
    {
        UStaticMeshComponent* StaticMeshComponent = PickedActor->GetComponentByClass<UStaticMeshComponent>();
        RenderForStaticMesh(StaticMeshComponent);
        RenderForMaterial(StaticMeshComponent);
    }

    if (PickedActor && PickedComponent && PickedComponent->IsA<UBillboardComponent>())
    {
        static const char* CurrentBillboardName = "Pawn";
        if (ImGui::TreeNodeEx("BillBoard", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::BeginCombo("##", CurrentBillboardName, ImGuiComboFlags_None))
            {
                if (ImGui::Selectable("Pawn", strcmp(CurrentBillboardName, "Pawn") == 0))
                {
                    CurrentBillboardName = "Pawn";
                    Cast<UBillboardComponent>(PickedComponent)->SetTexture(L"Assets/Texture/Pawn_64x.png");
                }
                if (ImGui::Selectable("PointLight", strcmp(CurrentBillboardName, "PointLight") == 0))
                {
                    CurrentBillboardName = "PointLight";
                    Cast<UBillboardComponent>(PickedComponent)->SetTexture(L"Assets/Texture/PointLight_64x.png");
                }
                if (ImGui::Selectable("SpotLight", strcmp(CurrentBillboardName, "SpotLight") == 0))
                {
                    CurrentBillboardName = "SpotLight";
                    Cast<UBillboardComponent>(PickedComponent)->SetTexture(L"Assets/Texture/SpotLight_64x.png");
                }

                ImGui::EndCombo();
            }
            ImGui::TreePop();
        }
    }
    if (PickedActor && PickedComponent && PickedComponent->IsA<UHeightFogComponent>())
    {
      UHeightFogComponent* fog = Cast<UHeightFogComponent>(PickedComponent);
        // Light Radius

      ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("ExponentialFog Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            float depthStart = fog->GetDepthStart();
            if (ImGui::SliderFloat("Depth Start", &depthStart, 0.0f, 20.0f))
            {
                fog->SetDepthStart(depthStart);
            }
            float depthFalloff = fog->GetDepthFalloff();
            if (ImGui::SliderFloat("Depth Falloff", &depthFalloff, 20.0f, 100.0f))
            {
                fog->SetDepthFalloff(depthFalloff);
            }
            float depthDensity= fog->GetFogDensity();
            if (ImGui::SliderFloat("Fog Depth Density", &depthDensity, 0.0f, 10.0f))
            {
                fog->SetFogDensity(depthDensity);
            }
            float heightStart= fog->GetHeightStart();
            if (ImGui::SliderFloat("Height Start", &heightStart, 0.0f, 20.0f))
            {
                fog->SetHeightStart(heightStart);
            }
            float heightFalloff= fog->GetHeightFalloff();
            if (ImGui::SliderFloat("Height Falloff", &heightFalloff, 20.0f, 100.0f))
            {
                fog->SetHeightFalloff(heightFalloff);
            }
            float heightDencity= fog->GetHeightDensity();
            if (ImGui::SliderFloat("Fog Height Density", &heightDencity, 0.0f, 100.0f))
            {
                fog->SetHeightDensity(heightDencity);
            }
            
            FVector4 currColor = fog->GetFogColor();
            float r = currColor.x;
            float g = currColor.y;
            float b = currColor.z;
            float a = currColor.w;
            float h, s, v;
            float lightColor[4] = { r, g, b, a };

            // SpotLight Color
            if (ImGui::ColorPicker4("Fog Color", lightColor,
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_NoInputs |
                ImGuiColorEditFlags_Float))

            {

                r = lightColor[0];
                g = lightColor[1];
                b = lightColor[2];
                a = lightColor[3];
                fog->SetFogColor(FVector4(r, g, b, a));
            }
            RGBToHSV(r, g, b, h, s, v);
            // RGB/HSV
            bool changedRGB = false;
            bool changedHSV = false;

            // RGB
            ImGui::PushItemWidth(50.0f);
            if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::Spacing();
            
            // HSV
            if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::PopItemWidth();
            ImGui::Spacing();
            
            if (changedRGB && !changedHSV)
            {
                // RGB -> HSV
                RGBToHSV(r, g, b, h, s, v);
                fog->SetFogColor(FVector4(r, g, b, a));
            }
            else if (changedHSV && !changedRGB)
            {
                // HSV -> RGB
                HSVToRGB(h, s, v, r, g, b);
                fog->SetFogColor(FVector4(r, g, b, a));
            }

            ImGui::TreePop();
        }

        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void PropertyEditorPanel::DrawSceneComponentTree(USceneComponent* Component, UActorComponent*& PickedComponent)
{
    if (!Component) return;

   FString Label = *Component->GetName();
   bool bSelected = (PickedComponent == Component);

   ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
   if (bSelected)
       nodeFlags |= ImGuiTreeNodeFlags_Selected;

   // 노드를 클릭 가능한 셀렉션으로 표시
   bool bOpened = ImGui::TreeNodeEx(*Label, nodeFlags);

   // 클릭되었을 때 선택 갱신
   if (ImGui::IsItemClicked())
   {
       PickedComponent = Component;
   }

   // 자식 재귀 호출
   if (bOpened)
   {
       for (USceneComponent* Child : Component->GetAttachChildren())
       {
           DrawSceneComponentTree(Child, PickedComponent);
       }
       ImGui::TreePop();
   }
}

void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp)
{
    if (StaticMeshComp->GetStaticMesh() == nullptr)
    {
        return;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("StaticMesh");
        ImGui::SameLine();

        FString PreviewName = StaticMeshComp->GetStaticMesh()->GetRenderData()->DisplayName;
        const TMap<FWString, UStaticMesh*> Meshes = FManagerOBJ::GetStaticMeshes();
        if (ImGui::BeginCombo("##StaticMesh", GetData(PreviewName), ImGuiComboFlags_None))
        {
            for (auto Mesh : Meshes)
            {
                if (ImGui::Selectable(GetData(Mesh.Value->GetRenderData()->DisplayName), false))
                {
                    StaticMeshComp->SetStaticMesh(Mesh.Value);
                }
            }

            ImGui::EndCombo();
        }
        
        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}


void PropertyEditorPanel::RenderForMaterial(UStaticMeshComponent* StaticMeshComp)
{
    if (StaticMeshComp->GetStaticMesh() == nullptr)
    {
        return;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        for (uint32 i = 0; i < StaticMeshComp->GetNumMaterials(); ++i)
        {
            if (ImGui::Selectable(GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    std::cout << GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()) << std::endl;
                    SelectedMaterialIndex = i;
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }

        if (ImGui::Button("    +    ")) {
            IsCreateMaterial = true;
        }
        
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("SubMeshes", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        auto subsets = StaticMeshComp->GetStaticMesh()->GetRenderData()->MaterialSubsets;
        for (uint32 i = 0; i < subsets.Num(); ++i)
        {
            std::string temp = "subset " + std::to_string(i);
            if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    StaticMeshComp->SetselectedSubMeshIndex(i);
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }
        std::string temp = "clear subset";
        if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
                StaticMeshComp->SetselectedSubMeshIndex(-1);
        }

        ImGui::TreePop();
    }

    ImGui::PopStyleColor();

    if (SelectedMaterialIndex != -1)
    {
        RenderMaterialView(SelectedStaticMeshComp->GetMaterial(SelectedMaterialIndex));
    }
    if (IsCreateMaterial) {
        RenderCreateMaterialView();
    }
}

void PropertyEditorPanel::RenderMaterialView(UMaterial* Material)
{
    ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;
    
    FVector MatDiffuseColor = Material->GetMaterialInfo().Diffuse;
    FVector MatSpecularColor = Material->GetMaterialInfo().Specular;
    FVector MatAmbientColor = Material->GetMaterialInfo().Ambient;
    FVector MatEmissiveColor = Material->GetMaterialInfo().Emissive;

    float dr = MatDiffuseColor.x;
    float dg = MatDiffuseColor.y;
    float db = MatDiffuseColor.z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Material Name |");
    ImGui::SameLine();
    ImGui::Text(*Material->GetMaterialInfo().MTLName);
    ImGui::Separator();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        Material->SetDiffuse(NewColor);
    }

    float sr = MatSpecularColor.x;
    float sg = MatSpecularColor.y;
    float sb = MatSpecularColor.z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        Material->SetSpecular(NewColor);
    }


    float ar = MatAmbientColor.x;
    float ag = MatAmbientColor.y;
    float ab = MatAmbientColor.z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        Material->SetAmbient(NewColor);
    }


    float er = MatEmissiveColor.x;
    float eg = MatEmissiveColor.y;
    float eb = MatEmissiveColor.z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        Material->SetEmissive(NewColor);
    }

    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Choose Material");
    ImGui::Spacing();
    
    ImGui::Text("Material Slot Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(SelectedStaticMeshComp->GetMaterialSlotNames()[SelectedMaterialIndex].ToString()));

    ImGui::Text("Override Material |");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    // 메테리얼 이름 목록을 const char* 배열로 변환
    std::vector<const char*> materialChars;
    for (const auto& material : FManagerOBJ::GetMaterials()) {
        materialChars.push_back(*material.Value->GetMaterialInfo().MTLName);
    }

    //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    //if (currentMaterialIndex >= FManagerOBJ::GetMaterialNum())
    //    currentMaterialIndex = 0;

    if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, materialChars.data(), FManagerOBJ::GetMaterialNum())) {
        UMaterial* material = FManagerOBJ::GetMaterial(materialChars[CurMaterialIndex]);
        SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, material);
    }
    
    if (ImGui::Button("Close"))
    {
        SelectedMaterialIndex = -1;
        SelectedStaticMeshComp = nullptr;
    }
     
    ImGui::End();
}

void PropertyEditorPanel::RenderCreateMaterialView()
{
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    ImGui::Text("New Name");
    ImGui::SameLine();
    static char materialName[256] = "New Material";
    // 기본 텍스트 입력 필드
    ImGui::SetNextItemWidth(128);
    if (ImGui::InputText("##NewName", materialName, IM_ARRAYSIZE(materialName))) {
        tempMaterialInfo.MTLName = materialName;
    }

    FVector MatDiffuseColor = tempMaterialInfo.Diffuse;
    FVector MatSpecularColor = tempMaterialInfo.Specular;
    FVector MatAmbientColor = tempMaterialInfo.Ambient;
    FVector MatEmissiveColor = tempMaterialInfo.Emissive;

    float dr = MatDiffuseColor.x;
    float dg = MatDiffuseColor.y;
    float db = MatDiffuseColor.z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Set Property");
    ImGui::Indent();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        tempMaterialInfo.Diffuse = NewColor;
    }

    float sr = MatSpecularColor.x;
    float sg = MatSpecularColor.y;
    float sb = MatSpecularColor.z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        tempMaterialInfo.Specular = NewColor;
    }


    float ar = MatAmbientColor.x;
    float ag = MatAmbientColor.y;
    float ab = MatAmbientColor.z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        tempMaterialInfo.Ambient = NewColor;
    }


    float er = MatEmissiveColor.x;
    float eg = MatEmissiveColor.y;
    float eb = MatEmissiveColor.z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        tempMaterialInfo.Emissive = NewColor;
    }
    ImGui::Unindent();

    ImGui::NewLine();
    if (ImGui::Button("Create Material")) {
        FManagerOBJ::CreateMaterial(tempMaterialInfo);
    }

    ImGui::NewLine();
    if (ImGui::Button("Close"))
    {
        IsCreateMaterial = false;
    }

    ImGui::End();
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
