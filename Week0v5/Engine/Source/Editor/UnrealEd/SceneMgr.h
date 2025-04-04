#pragma once
#include "Define.h"
#include "Container/Map.h"

class UObject;
struct SceneData {
    int32 Version;
    int32 NextUUID;
    TMap<int32, UObject*> Primitives;
    TMap<int32, UObject*> Cameras;
};
class FSceneMgr
{
public:
    static void ParseSceneData(const FString& jsonStr);
    static FString LoadSceneFromFile(const FString& filename);
    static std::string SerializeSceneData(const SceneData& sceneData);
    static bool SaveSceneToFile(const FString& filename, const SceneData& sceneData);
};

