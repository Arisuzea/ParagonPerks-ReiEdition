#pragma once
#include "Settings.h"
#include "Conditions.h"

namespace TimedBlockHandler 
{
	class BlockHandler : public Singleton<BlockHandler>
	{
	public: 

		void ProcessHitEventForParry(RE::Actor* target, RE::Actor* aggressor);
		void ProcessHitEventForParryShield(RE::Actor* target, RE::Actor* aggressor, bool should_stagger);
		void destroyProjectile(RE::Projectile* a_projectile);
		void BlockProjectile(RE::Actor* a_blocker, RE::Projectile* a_projectile);
		void PlaySparks(RE::Actor* defender);
		inline bool timedBlockAllowed(RE::Actor* defender);

		inline bool isInBlockAngle(RE::Actor* blocker, RE::TESObjectREFR* a_obj);

		// VALHALLA COMBAT MOD: https://github.com/D7ry/valhallaCombat/blob/48fb4c3b9bb6bbaa691ce41dbd33f096b74c07e3/src/include/blockHandler.h

		bool processProjectileBlock(RE::Actor* a_blocker, RE::Projectile* a_projectile, RE::hkpCollidable* a_projectile_collidable);

	private:

		bool tryBlockProjectile_Arrow(RE::Actor* a_blocker, RE::Projectile* a_projectile);
		bool tryBlockProjectile(RE::Actor* a_blocker, RE::Projectile* a_projectile, float a_cost);




	};


}
