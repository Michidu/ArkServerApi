#include "SafeZoneManager.h"

#include <Permissions.h>

#include "SafeZones.h"

namespace SafeZones
{
	SafeZoneManager& SafeZoneManager::Get()
	{
		static SafeZoneManager instance;
		return instance;
	}

	void SafeZoneManager::ReadSafeZones()
	{
		const auto& safe_zones = config["SafeZones"];
		for (const auto& safe_zone : safe_zones)
		{
			auto config_position = safe_zone["Position"];
			auto config_success_color = safe_zone["SuccessNotificationColor"];
			auto config_fail_color = safe_zone["FailNotificationColor"];

			std::string str_name = safe_zone["Name"];
			FString name = str_name.c_str();

			FVector position{config_position[0], config_position[1], config_position[2]};
			int radius = safe_zone["Radius"];
			bool prevent_pvp = safe_zone["PreventPVP"];
			bool prevent_structure_damage = safe_zone["PreventStructureDamage"];
			bool prevent_building = safe_zone["PreventBuilding"];
			bool kill_wild_dinos = safe_zone["KillWildDinos"];
			bool prevent_leaving = safe_zone["PreventLeaving"];
			bool prevent_entering = safe_zone["PreventEntering"];

			bool enable_events = safe_zone["EnableEvents"];
			bool screen_notifications = safe_zone["ScreenNotifications"];
			bool chat_notifications = safe_zone["ChatNotifications"];

			FLinearColor success_color{
				config_success_color[0], config_success_color[1], config_success_color[2], config_success_color[3]
			};
			FLinearColor fail_color{config_fail_color[0], config_fail_color[1], config_fail_color[2], config_fail_color[3]};

			std::vector<FString> messages;
			for (const auto& msg : safe_zone["Messages"])
			{
				messages.emplace_back(ArkApi::Tools::Utf8Decode(msg).c_str());
			}

			CreateSafeZone(std::make_shared<SafeZone>(name, position, radius, prevent_pvp, prevent_structure_damage,
			                                          prevent_building, kill_wild_dinos, prevent_leaving, prevent_entering,
			                                          enable_events, screen_notifications, chat_notifications, success_color,
			                                          fail_color, messages));
		}
	}

	void SafeZoneManager::CreateSafeZone(const std::shared_ptr<SafeZone>& safe_zone)
	{
		all_safezones_.Add(safe_zone);
	}

	/*ATriggerBase* SafeZoneManager::SpawnSphere(FVector& location, int radius, const std::string& type)
	{
		FActorSpawnParameters spawn_parameters;
		FRotator rotation{0, 0, 0};

		UClass* sphere_class = static_cast<UClass*>(TClassCompiledInDefer_ATriggerSphere::Register());

		ATriggerBase* trigger_sphere = static_cast<ATriggerBase*>(ArkApi::GetApiUtils().GetWorld()->SpawnActor(
			sphere_class, &location, &rotation,
			&spawn_parameters));

		USphereComponent* sphere_component = static_cast<USphereComponent*>(
			trigger_sphere->CollisionComponentField()().Object);

		const ECollisionResponse collision_type = //type == "Overlap"
			//? 
			ECollisionResponse::ECR_Overlap;
		//: ECollisionResponse::ECR_Block;

		sphere_component->bGenerateOverlapEvents() = true;
		sphere_component->SetCollisionResponseToAllChannels(collision_type);
		sphere_component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sphere_component->SetCollisionObjectType(ECollisionChannel::ECC_OverlapAll_Deprecated);

		sphere_component->SetSphereRadius(static_cast<float>(radius), true);

		return trigger_sphere;
	}*/

	bool SafeZoneManager::CanBuild(APlayerController* player, const FVector& location, bool notification)
	{
		const bool admins_ignore = config["AdminsIgnoreRestrictions"];

		const uint64 steam_id = ArkApi::IApiUtils::GetSteamIdFromController(player);
		if (admins_ignore && Permissions::IsPlayerInGroup(steam_id, "Admins"))
			return true;

		for (const auto& safe_zone : all_safezones_)
		{
			if (FVector::Distance(safe_zone->position, location) <= safe_zone->radius)
			{
				if (FString msg = safe_zone->messages[2];
					!msg.IsEmpty() && notification)
				{
					safe_zone->SendNotification(static_cast<AShooterPlayerController*>(player), msg, safe_zone->fail_color);
				}

				return false;
			}
		}

		return true;
	}

	bool SafeZoneManager::CheckActorAction(AActor* actor, int type)
	{
		bool is_protected = false;

		for (const auto& safe_zone : all_safezones_)
		{
			if (safe_zone->IsOverlappingActor(actor))
			{
				switch (type)
				{
				case 1:
					is_protected = safe_zone->prevent_pvp;
					break;
				case 2:
					is_protected = safe_zone->prevent_structure_damage;
					break;
				}
				break;
			}
		}

		return is_protected;
	}

	void SafeZoneManager::OnEnterSafeZone(const std::shared_ptr<SafeZone>& safe_zone, AActor* other_actor)
	{
		if (!other_actor)
			return;

		if (other_actor->IsA(AShooterCharacter::GetPrivateStaticClass()))
		{
			AShooterPlayerController* player = ArkApi::GetApiUtils().FindControllerFromCharacter(
				static_cast<AShooterCharacter*>(other_actor));
			if (player)
			{
				if (safe_zone->prevent_leaving || safe_zone->prevent_entering)
				{
					if (players_pos_.find(player) != players_pos_.end())
					{
						auto& player_pos = players_pos_[player];

						player_pos.in_zone = true;

						if (safe_zone->prevent_entering)
						{
							const FVector& last_pos = player_pos.outzone_pos;
							player->SetPlayerPos(last_pos.X, last_pos.Y, last_pos.Z);

							safe_zone->actors.RemoveSingle(other_actor);
							return;
						}
					}
					else
					{
						players_pos_[player] = {true, player->DefaultActorLocationField()(), player->DefaultActorLocationField()()};
					}
				}

				safe_zone->SendNotification(player, FString::Format(*safe_zone->messages[0], *safe_zone->name),
				                            safe_zone->success_color);
			}
		}
		else if (safe_zone->kill_wild_dinos && other_actor->TargetingTeamField()() < 50000 &&
			other_actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass()))
		{
			APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(other_actor);
			dino->Suicide();
		}

		// Execute callbacks
		for (const auto& callback : safe_zone->on_actor_begin_overlap)
		{
			callback(other_actor);
		}
	}

	void SafeZoneManager::OnLeaveSafeZone(const std::shared_ptr<SafeZone>& safe_zone, AActor* other_actor)
	{
		if (!other_actor)
			return;

		if (other_actor->IsA(AShooterCharacter::GetPrivateStaticClass()))
		{
			AShooterPlayerController* player = ArkApi::GetApiUtils().FindControllerFromCharacter(
				static_cast<AShooterCharacter*>(other_actor));
			if (player)
			{
				if (safe_zone->prevent_leaving || safe_zone->prevent_entering)
				{
					auto& player_pos = players_pos_[player];

					player_pos.in_zone = false;

					if (safe_zone->prevent_leaving)
					{
						const FVector& last_pos = player_pos.inzone_pos;
						player->SetPlayerPos(last_pos.X, last_pos.Y, last_pos.Z);

						safe_zone->actors.Add(other_actor);
						return;
					}
				}

				safe_zone->SendNotification(player, FString::Format(*safe_zone->messages[1], *safe_zone->name),
				                            safe_zone->fail_color);
			}
		}

		// Execute callbacks
		for (const auto& callback : safe_zone->on_actor_end_overlap)
		{
			callback(other_actor);
		}
	}

	void SafeZoneManager::UpdateOverlaps()
	{
		UWorld* world = ArkApi::GetApiUtils().GetWorld();
		if (!world)
			return;

		for (const auto& safe_zone : all_safezones_)
		{
			// Make a copy of the old actors array
			TArray<AActor*> old_actors = safe_zone->actors;

			TArray<AActor*> new_actors;

			TArray<AActor*> actors_ignore;
			TArray<TEnumAsByte<enum EObjectTypeQuery>> types;

			UKismetSystemLibrary::SphereOverlapActors_NEW(world, safe_zone->position, static_cast<float>(safe_zone->radius),
			                                              &types, nullptr, &actors_ignore, &new_actors);

			// Update safe zone actors list
			safe_zone->actors = new_actors;

			if (safe_zone->enable_events)
			{
				for (int i = 0; i < old_actors.Num() && new_actors.Num() > 0; ++i)
				{
					const bool allow_shrinking = false;
					if (new_actors.RemoveSingleSwap(old_actors[i], allow_shrinking) > 0)
					{
						old_actors.RemoveAtSwap(i, 1, allow_shrinking);
						--i;
					}
				}

				// old_actors now contains only previous overlaps
				for (const auto& actor : old_actors)
				{
					OnLeaveSafeZone(safe_zone, actor);
				}

				// new_actors now contains only new overlaps
				for (const auto& actor : new_actors)
				{
					OnEnterSafeZone(safe_zone, actor);
				}
			}
		}

		const auto& player_controllers = world->PlayerControllerListField()();
		for (TWeakObjectPtr<APlayerController> player_controller : player_controllers)
		{
			AShooterPlayerController* player = static_cast<AShooterPlayerController*>(player_controller.Get());
			if (player)
			{
				if (players_pos_.find(player) == players_pos_.end())
					players_pos_[player] = {false, player->DefaultActorLocationField()(), player->DefaultActorLocationField()()};
				else if (players_pos_[player].in_zone)
					players_pos_[player].inzone_pos = player->DefaultActorLocationField()();
				else
					players_pos_[player].outzone_pos = player->DefaultActorLocationField()();
			}
		}
	}

	TArray<std::shared_ptr<SafeZone>>& SafeZoneManager::GetAllSafeZones()
	{
		return all_safezones_;
	}

	// Free function
	ISafeZoneManager& GetSafeZoneManager()
	{
		return SafeZoneManager::Get();
	}
}
