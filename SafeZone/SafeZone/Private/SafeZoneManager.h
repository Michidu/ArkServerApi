#pragma once

#include "../Public/ISafeZoneManager.h"

/*struct TClassCompiledInDefer_ATriggerSphere
{
	static UField* Register() { return NativeCall<UField *>(nullptr, "TClassCompiledInDefer<ATriggerSphere>.Register"); }
};

struct ATriggerBase : AActor
{
	FieldValue<TSubobjectPtr<UShapeComponent>> CollisionComponentField()
	{
		return {this, "ATriggerBase.CollisionComponent"};
	}
};*/

namespace SafeZones
{
	class SafeZoneManager : public ISafeZoneManager
	{
	public:
		static SafeZoneManager& Get();

		SafeZoneManager(const SafeZoneManager&) = delete;
		SafeZoneManager(SafeZoneManager&&) = delete;
		SafeZoneManager& operator=(const SafeZoneManager&) = delete;
		SafeZoneManager& operator=(SafeZoneManager&&) = delete;

		void CreateSafeZone(const std::shared_ptr<SafeZone>& safe_zone) override;
		bool CanBuild(APlayerController* player, const FVector& location, bool notification) override;
		bool CheckActorAction(AActor* actor, int type) override;

		void OnEnterSafeZone(const std::shared_ptr<SafeZone>& safe_zone, AActor* other_actor);
		void OnLeaveSafeZone(const std::shared_ptr<SafeZone>& safe_zone, AActor* other_actor);

		void ReadSafeZones();
		void UpdateOverlaps();

		TArray<std::shared_ptr<SafeZone>>& GetAllSafeZones();

	private:
		struct PlayerPos
		{
			bool in_zone{};
			FVector inzone_pos;
			FVector outzone_pos;
		};

		SafeZoneManager() = default;
		~SafeZoneManager() = default;

		std::unordered_map<AShooterPlayerController*, PlayerPos> players_pos_;

		TArray<std::shared_ptr<SafeZone>> all_safezones_;
	};
}
