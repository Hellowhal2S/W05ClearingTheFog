#include "EngineStatics.h"
#include "Define.h"
uint32 UEngineStatics::NextUUID = 0;

UEngineStatics::UEngineStatics()
{
}

UEngineStatics::~UEngineStatics()
{
}

uint32 UEngineStatics::GenUUID()
{
    UE_LOG(LogLevel::Display, "Generate UUID : %d", NextUUID);
    return NextUUID++;
}
