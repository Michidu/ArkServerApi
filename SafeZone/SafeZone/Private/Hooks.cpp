#include "Hooks.h"
#include "SafeZoneManager.h"

namespace SafeZones::Hooks
{
	DECLARE_HOOK(AShooterGameMode_InitGame, void, AShooterGameMode*, FString*, FString*, FString*);

	DECLARE_HOOK(APrimalStructure_IsAllowedToBuild, int, APrimalStructure*, APlayerController*, FVector, FRotator,
		FPlacementData*, bool, FRotator, bool);
	DECLARE_HOOK(APrimalCharacter_TakeDamage, float, APrimalCharacter*, float, FDamageEvent*, AController*, AActor*);
	DECLARE_HOOK(APrimalStructure_TakeDamage, float, APrimalStructure*, float, FDamageEvent*, AController*, AActor*);
	DECLARE_HOOK(APrimalDinoCharacter_CanCarryCharacter, bool, APrimalDinoCharacter*, APrimalCharacter*);

	void Hook_AShooterGameMode_InitGame(AShooterGameMode* a_shooter_game_mode, FString* map_name, FString* options,
	                                    FString* error_message)
	{
		AShooterGameMode_InitGame_original(a_shooter_game_mode, map_name, options, error_message);

		SafeZoneManager::Get().ReadSafeZones();
	}

	int Hook_APrimalStructure_IsAllowedToBuild(APrimalStructure* _this, APlayerController* PC, FVector AtLocation,
	                                           FRotator AtRotation, FPlacementData* OutPlacementData,
	                                           bool bDontAdjustForMaxRange, FRotator PlayerViewRotation,
	                                           bool bFinalPlacement)
	{
		if (bFinalPlacement && PC && !SafeZoneManager::Get().CanBuild(PC, AtLocation, true))
			return 0;

		return APrimalStructure_IsAllowedToBuild_original(_this, PC, AtLocation, AtRotation, OutPlacementData,
			bDontAdjustForMaxRange, PlayerViewRotation, bFinalPlacement);
	}

	float Hook_APrimalCharacter_TakeDamage(APrimalCharacter* _this, float Damage, FDamageEvent* DamageEvent,
	                                       AController* EventInstigator, AActor* DamageCauser)
	{
		if (_this->IsA(AShooterCharacter::GetPrivateStaticClass()) &&
			SafeZoneManager::Get().CheckActorAction(_this, 1))
		{
			return 0;
		}
		if (_this->IsA(APrimalDinoCharacter::GetPrivateStaticClass()) &&
			_this->TargetingTeamField()() > 50000 &&
			SafeZoneManager::Get().CheckActorAction(_this, 1))
		{
			return 0;
		}
		if (EventInstigator && EventInstigator->CharacterField()() &&
			SafeZoneManager::Get().CheckActorAction(EventInstigator->CharacterField()(), 1))
		{
			return 0;
		}

		return APrimalCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
	}

	float Hook_APrimalStructure_TakeDamage(APrimalStructure* _this, float Damage, FDamageEvent* DamageEvent,
	                                       AController* EventInstigator, AActor* DamageCauser)
	{
		return SafeZoneManager::Get().CheckActorAction(_this, 2)
			       ? 0
			       : APrimalStructure_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
	}

	bool Hook_APrimalDinoCharacter_CanCarryCharacter(APrimalDinoCharacter* _this, APrimalCharacter* CanCarryPawn)
	{
		return SafeZoneManager::Get().CheckActorAction(_this, 1)
			       ? false
			       : APrimalDinoCharacter_CanCarryCharacter_original(_this, CanCarryPawn);
	}

	void Timer()
	{
		SafeZoneManager::Get().UpdateOverlaps();
	}

	/*void OnTick(float)
	{
		auto& all_safezones = SafeZoneManager::Get().GetAllSafeZones();
		for (const auto& safe_zone : all_safezones)
		{
			if (safe_zone->actors.Num() > 0)
			{
				for (AActor* actor : safe_zone->actors)
				{
					if (actor->IsA(AShooterCharacter::GetPrivateStaticClass()) && FVector::DistSquared(
						safe_zone->position, actor->DefaultActorLocationField()()) > FMath::Square(safe_zone->radius))
					{
						AShooterCharacter* character = static_cast<AShooterCharacter*>(actor);

						FVector velocity;
						character->GetActorForwardVector(&velocity);

						character->CharacterMovementField()()->AddImpulse(velocity * -100, true, false, false);
					}
				}
			}
		}
	}*/

	void InitHooks()
	{
		auto& hooks = ArkApi::GetHooks();

		hooks.SetHook("AShooterGameMode.InitGame", &Hook_AShooterGameMode_InitGame,
		              &AShooterGameMode_InitGame_original);

		hooks.SetHook("APrimalStructure.IsAllowedToBuild", &Hook_APrimalStructure_IsAllowedToBuild,
		              &APrimalStructure_IsAllowedToBuild_original);
		hooks.SetHook("APrimalCharacter.TakeDamage", &Hook_APrimalCharacter_TakeDamage,
		              &APrimalCharacter_TakeDamage_original);
		hooks.SetHook("APrimalStructure.TakeDamage", &Hook_APrimalStructure_TakeDamage,
		              &APrimalStructure_TakeDamage_original);
		hooks.SetHook("APrimalDinoCharacter.CanCarryCharacter", &Hook_APrimalDinoCharacter_CanCarryCharacter,
		              &APrimalDinoCharacter_CanCarryCharacter_original);

		ArkApi::GetCommands().AddOnTimerCallback("SafeZoneTimer", &Timer);
	}

	void RemoveHooks()
	{
		auto& hooks = ArkApi::GetHooks();

		hooks.DisableHook("AShooterGameMode.InitGame", &Hook_AShooterGameMode_InitGame);

		hooks.DisableHook("APrimalStructure.IsAllowedToBuild", &Hook_APrimalStructure_IsAllowedToBuild);
		hooks.DisableHook("APrimalCharacter.TakeDamage", &Hook_APrimalCharacter_TakeDamage);
		hooks.DisableHook("APrimalStructure.TakeDamage", &Hook_APrimalStructure_TakeDamage);
		hooks.DisableHook("APrimalDinoCharacter.CanCarryCharacter", &Hook_APrimalDinoCharacter_CanCarryCharacter);

		ArkApi::GetCommands().RemoveOnTimerCallback("SafeZoneTimer");
	}
}
