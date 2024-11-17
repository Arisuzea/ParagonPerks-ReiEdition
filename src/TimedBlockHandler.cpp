#include "TimedBlockHandler.h"


namespace TimedBlockHandler {


		void BlockHandler::ProcessHitEventForParry(RE::Actor* target, RE::Actor* aggressor)

		{
			auto settings = Settings::GetSingleton();
			if (Conditions::PlayerHasActiveMagicEffect(settings->MAG_ParryWindowEffect)) {
				for (auto& actors : Conditions::GetNearbyActors(target, settings->surroundingActorsRange, false)) {
					if (actors != aggressor) {
						Conditions::ApplySpell(target, actors, settings->MAGParryStaggerSpell);
					}
				}
				Conditions::ApplySpell(target, aggressor, settings->MAGParryStaggerSpell);
				Conditions::ApplySpell(aggressor, target, settings->APOParryBuffSPell);
				target->PlaceObjectAtMe(settings->APOSparksFlash, false);
			}
		}

		void BlockHandler::ProcessHitEventForParryShield(RE::Actor* target, RE::Actor* aggressor, bool should_stagger)

		{
			auto settings = Settings::GetSingleton();
			if (Conditions::PlayerHasActiveMagicEffect(settings->MAG_ParryWindowEffect)) {
				for (auto& actors : Conditions::GetNearbyActors(target, settings->surroundingActorsRange, false)) {
					if (actors != aggressor) {
						if (should_stagger) {
							Conditions::ApplySpell(target, actors, settings->MAGParryStaggerSpell);
						}
					}
				}
				if (should_stagger) {
					Conditions::ApplySpell(target, aggressor, settings->MAGParryStaggerSpell);
				}            
				Conditions::ApplySpell(aggressor, target, settings->APOParryBuffSPell);
				target->PlaceObjectAtMe(settings->APOSparksShieldFlash, false);
			}
		}

		void BlockHandler::destroyProjectile(RE::Projectile* a_projectile)

		{
			Hooks::destroyProjectile(a_projectile);
		}

		void BlockHandler::BlockProjectile(RE::Actor* a_blocker, RE::Projectile* a_projectile)
		{
			destroyProjectile(a_projectile);
			if (a_blocker->IsBlocking()) {
				a_blocker->NotifyAnimationGraph("BlockHitStart");
			}
			return;
		}

		void BlockHandler::PlaySparks(RE::Actor* defender)
		{
			const Settings* settings = Settings::GetSingleton();
			defender->PlaceObjectAtMe(settings->APOSparks, false);
			defender->PlaceObjectAtMe(settings->APOSparksPhysics, false);
		}

		inline bool BlockHandler::timedBlockAllowed(RE::Actor* defender)
		{
			return Conditions::ActorHasActiveEffect(defender, Settings::MAG_ParryWindowEffect);
		}


		inline bool BlockHandler::isInBlockAngle(RE::Actor* blocker, RE::TESObjectREFR* a_obj)
		{
			auto angle = blocker->GetHeadingAngle(a_obj->GetPosition(), false);
			dlog("heading angle is {} and compare value is {}", angle, Settings::blockAngleSetting);
			return (angle <= Settings::blockAngleSetting && angle >= -Settings::blockAngleSetting);
		}

		bool BlockHandler::processProjectileBlock(RE::Actor* a_blocker, RE::Projectile* a_projectile, RE::hkpCollidable* a_projectile_collidable)
		{
			if (!a_blocker->IsPlayerRef()) {
				return false;
			}
			auto shooter = a_projectile->GetProjectileRuntimeData().shooter.get().get();
			if ((isInBlockAngle(a_blocker, a_projectile) || isInBlockAngle(a_blocker, shooter) && a_blocker->IsBlocking())) {
				dlog("condition for arrow parry true");
				// evaluate cost
				float cost{10.0f};
				if (a_projectile->GetProjectileRuntimeData().spell) {  // parry spell
					dlog("found spell to parry");
					cost = Conditions::projectileBlockCost(a_blocker, 10.0f);
					if (a_blocker->HasPerk(Settings::MagicParryPerk)) {
						dlog("blocked {} from {} it costed {}", a_projectile->GetProjectileRuntimeData().spell->GetName(), shooter->GetName(), cost);
						return tryBlockProjectile(a_blocker, a_projectile, cost);
					}
				} 
				else { // parry arrow
					float cost = Conditions::projectileBlockCost(a_blocker, 10.0f);	
					if (a_blocker->HasPerk(Settings::ArrowParryPerk)) {
						dlog("blocked {} from {} it costed {}", a_projectile->GetName(), shooter->GetName(), cost);
						return tryBlockProjectile(a_blocker, a_projectile, cost);
					}
				}
				return false;
			}
			return false;
		}

		bool BlockHandler::tryBlockProjectile(RE::Actor* a_blocker, RE::Projectile* a_projectile, float a_cost)
		{
			if (Conditions::tryDamageAV(a_blocker, RE::ActorValue::kStamina, a_cost) ) {
				if (a_blocker->IsPlayerRef()) {
					auto pc = RE::PlayerCharacter::GetSingleton();
					if (pc) {
						pc->AddSkillExperience(RE::ActorValue::kBlock, a_cost/3);
					}
				}
				auto aggressor = a_projectile->GetProjectileRuntimeData().shooter.get().get()->As<RE::Actor>();
				destroyProjectile(a_projectile);
				if (!aggressor) {
					dlog("no aggressor detected");
				}

				if (aggressor) {
					ProcessHitEventForParryShield(a_blocker, aggressor, false);
				}				
				if (a_blocker->IsBlocking()) {
					a_blocker->NotifyAnimationGraph("BlockHitStart");
				}				
				return true;
			}
			return false;
		}
}

