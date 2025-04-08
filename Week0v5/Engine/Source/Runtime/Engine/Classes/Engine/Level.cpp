#include "Level.h"
#include "GameFramework/Actor.h"

ULevel::ULevel()
{
}

ULevel::ULevel(const ULevel& Other)
    : UObject(Other),  // UObject 기반 클래스 복사
    PendingBeginPlayActors(Other.PendingBeginPlayActors) // TArray 깊은 복사
{
}

ULevel::~ULevel()
{
}

UObject* ULevel::Duplicate() const
{
    ULevel* CloneLevel = FObjectFactory::ConstructObjectFrom<ULevel>(this);
    CloneLevel->DuplicateSubObjects(this);
    CloneLevel->PostDuplicate();
    return CloneLevel;
}

void ULevel::DuplicateSubObjects(const UObject* SourceObj)
{
    for (AActor* Actor : Cast<ULevel>(SourceObj)->GetActors())
    {
        AActor* dupActor = static_cast<AActor*>(Actor->Duplicate());
        Actors.Add(dupActor);
    }
}

void ULevel::PostDuplicate()
{
    UObject::PostDuplicate();
}
