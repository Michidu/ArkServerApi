#pragma once

#include "SafeZone.h"

namespace SafeZones
{
	class ZONE_API ISafeZoneManager
	{
	public:
		virtual ~ISafeZoneManager() = default;

		virtual void CreateSafeZone(const std::shared_ptr<SafeZone>& safe_zone) = 0;
		virtual bool CanBuild(APlayerController* player, const FVector& location, bool notification = false) = 0;
		virtual bool CheckActorAction(AActor* actor, int type) = 0;
	};

	ZONE_API ISafeZoneManager& APIENTRY GetSafeZoneManager();
}
