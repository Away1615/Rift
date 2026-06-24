#include "Data/Audio/RiftAudioConfig.h"

#include "Engine/World.h"
#include "Misc/PackageName.h"

namespace
{
bool IsMapInList(const FString& WorldPackageName, const TArray<TSoftObjectPtr<UWorld>>& Maps)
{
	const FString NormalizedWorldPackageName = UWorld::RemovePIEPrefix(WorldPackageName);
	const FString WorldShortName = FPackageName::GetShortName(NormalizedWorldPackageName);

	for (const TSoftObjectPtr<UWorld>& Map : Maps)
	{
		const FString MapPackageName = Map.ToSoftObjectPath().GetLongPackageName();
		if (MapPackageName.IsEmpty())
		{
			continue;
		}

		if (NormalizedWorldPackageName == MapPackageName ||
			WorldShortName == FPackageName::GetShortName(MapPackageName))
		{
			return true;
		}
	}

	return false;
}
}

bool URiftAudioConfig::IsFrontendMap(const FString& WorldPackageName) const
{
	return IsMapInList(WorldPackageName, FrontendMaps);
}

USoundBase* URiftAudioConfig::GetUISound(const ERiftUISoundType SoundType) const
{
	switch (SoundType)
	{
	case ERiftUISoundType::ButtonHover:
		return ButtonHoverSound;
	case ERiftUISoundType::ButtonClick:
		return ButtonClickSound;
	case ERiftUISoundType::Back:
		return BackSound;
	case ERiftUISoundType::Error:
		return ErrorSound;
	default:
		return nullptr;
	}
}
