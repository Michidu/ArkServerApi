#include "../Public/SafeZone.h"

#include "SafeZones.h"

namespace SafeZones
{
	bool SafeZone::IsOverlappingActor(AActor* other) const
	{
		return actors.Find(other) != INDEX_NONE;
	}

	void SafeZone::SendNotification(AShooterPlayerController* player, const FString& message,
	                                const FLinearColor& color) const
	{
		if (screen_notifications)
		{
			const float display_scale = config["NotificationScale"];
			const float display_time = config["NotificationDisplayTime"];

			ArkApi::GetApiUtils().SendNotification(player, color, display_scale, display_time,
			                                       nullptr, *message);
		}

		if (chat_notifications)
		{
			ArkApi::GetApiUtils().SendChatMessage(player, name,
			                                      fmt::format(L"<RichColor Color=\"{0}, {1}, {2}, {3}\">{4}</>", color.R,
			                                                  color.G, color.B, color.A, *message).c_str());
		}
	}
}
