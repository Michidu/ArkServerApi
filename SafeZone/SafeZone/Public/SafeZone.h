#pragma once

#include <API/ARK/Ark.h>

#ifdef ZONE_EXPORTS
#define ZONE_API __declspec(dllexport)
#else
#define ZONE_API __declspec(dllimport)
#endif

namespace SafeZones
{
	struct SafeZone
	{
		SafeZone(FString name, const FVector& position, int radius, bool prevent_pvp, bool prevent_structure_damage,
		         bool prevent_building, bool kill_wild_dinos, bool enable_events, bool screen_notifications,
		         bool chat_notifications,
		         const FLinearColor& enter_notification_color, const FLinearColor& leave_notification_color,
		         std::vector<FString> messages)
			: name(std::move(name)),
			  position(position),
			  radius(radius),
			  prevent_pvp(prevent_pvp),
			  prevent_structure_damage(prevent_structure_damage),
			  prevent_building(prevent_building),
			  kill_wild_dinos(kill_wild_dinos),
			  enable_events(enable_events),
			  screen_notifications(screen_notifications),
			  chat_notifications(chat_notifications),
			  enter_notification_color(enter_notification_color),
			  leave_notification_color(leave_notification_color),
			  messages(std::move(messages))
		{
		}

		FString name;
		FVector position;
		int radius;

		bool prevent_pvp;
		bool prevent_structure_damage;
		bool prevent_building;
		bool kill_wild_dinos;

		bool enable_events;
		bool screen_notifications;
		bool chat_notifications;

		FLinearColor enter_notification_color;
		FLinearColor leave_notification_color;

		std::vector<FString> messages;

		TArray<AActor*> actors;

		TArray<std::function<void(AActor*)>> on_actor_begin_overlap;
		TArray<std::function<void(AActor*)>> on_actor_end_overlap;

		// Functions

		ZONE_API bool IsOverlappingActor(AActor* other) const;
		ZONE_API void SendNotification(AShooterPlayerController* player, const FString& message,
		                               const FLinearColor& color) const;
	};
}
