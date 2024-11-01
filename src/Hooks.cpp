#include "Events.h"
#include "UpdateManager.h"
#include "patches/ArmorRatingScaling.h"
#include "patches/BashBlockStaminaPatch.h"
#include "patches/MiscPatches.h"

namespace Hooks
{
    bool InstallHooks()
    {
        if (!UpdateManager::Install()) {
            return false;
        }
        if (!MiscPatches::InstallScalePatch()) {
            return false;
        }
        if (!MiscPatches::InstallFBlockPatch()) {
            return false;
        }
        if (!MiscPatches::InstallSpellCapPatch()) {
            return false;
        }

        WeaponFireHandler::InstallArrowReleaseHook();

        auto runtime = REL::Module::GetRuntime();
        if (Settings::GetSingleton()->armorScalingEnabled) {
            if (runtime == REL::Module::Runtime::AE) {
                logger::info("Installing ar hook AE");
                ArmorRatingScaling::InstallArmorRatingHookAE();
            }
            else {
                logger::info("Installing ar hook SE");
                ArmorRatingScaling::InstallArmorRatingHookSE();
            }
            logger::info("Installed ar hook");
        }

        if (!BashBlockStaminaPatch::InstallBlockMultHook()) {
            return false;
        }
        ActorUpdateHook::InstallUpdateActor();
        CombatHit::Install();

        return true;
    }

    bool InstallBashMultHook()
    {
        return BashBlockStaminaPatch::InstallBashMultHook();
    }


    void ActorUpdateHook::InstallUpdateActor()
    {
        REL::Relocation<std::uintptr_t> ActorVTABLE{ RE::VTABLE_Character[0] };

        _ActorUpdate = ActorVTABLE.write_vfunc(0xAD, ActorUpdate);
        logger::info("hook:NPC Update");
    }

    void ActorUpdateHook::ActorUpdate(RE::Character* a_this, float a_delta)
    {
        Settings* settings = Settings::GetSingleton();
        if (Conditions::ActorHasActiveEffect(a_this, settings->StaminaPenEffectNPC)) {
            dlog("has effect, gray out meter");
            Conditions::greyoutAvMeter(a_this, RE::ActorValue::kStamina);
        }
        else {
            Conditions::revertAvMeter(a_this, RE::ActorValue::kStamina);
        }
        return _ActorUpdate(a_this, a_delta);
    }
    void CombatHit::Install()
    {
        //SE: 140628C20 - 37673
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0) };
        _originalCall = trampoline.write_call<5>(target.address(), &CHit);
    }

    /*
    public:
		static void Hook()
		{
			_get_damage = SKSE::GetTrampoline().write_call<5>(REL::ID(42832).address() + 0x1a5, get_damage);  // SkyrimSE.exe+7429F5
		}

        FENIX
    */

    void CombatHit::PitFighter(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        const Settings* settings = Settings::GetSingleton();

        if (a_this->HasPerk(settings->dummyPerkDodge)) {
            float remaining = a_hitData->totalDamage;
            if (a_hitData->weapon && Conditions::NumNearbyActors(a_this, 500.0f, true) > 0) {
                if (!a_hitData->weapon->IsBow() && !a_hitData->weapon->IsCrossbow()) {
                    logger::debug("----------------------------------------------------");
                    logger::debug("started hooked damage calc: {} was hit by {} with {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName(),a_hitData->weapon->GetName());
                    std::int32_t enemyNum = Conditions::NumNearbyActors(a_this, 500.0f, false);
                    dlog("{} is surrounded by {} enemies", a_this->GetDisplayFullName(), (int)enemyNum);
                    if (enemyNum == 2)
                        remaining *= 2.0f;
                    if (enemyNum == 3)
                        remaining *= 5.30f;
                    if (enemyNum >= 4)
                        remaining *= 11.5;
                    logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
                    _originalCall(a_this, a_hitData);
                    a_hitData->totalDamage = remaining;
                }
                else if (a_hitData->weapon->IsBow() || a_hitData->weapon->IsCrossbow()) {
                    logger::debug("started hooked damage calc: {} has hit {} with {}", a_this->GetName(), a_hitData->aggressor.get().get()->GetDisplayFullName(), a_hitData->weapon->GetName());
                    std::int32_t enemyNum = Conditions::NumNearbyActors(a_this, 500.0f, true);
                    if (enemyNum == 1)
                        remaining *= 1.5f;
                    if (enemyNum == 2)
                        remaining *= 1.30f;
                    if (enemyNum >= 3)
                        remaining *= 1.15;
                    logger::debug("new damage for melee is {}. Original damage was: {} \n", remaining, a_hitData->totalDamage);
                    _originalCall(a_this, a_hitData);
                    a_hitData->totalDamage = remaining;
                }
                
            }
        }
    }
    void CombatHit::CHit(RE::Actor* a_this, RE::HitData* a_hitData)
    {
        PitFighter(a_this, a_hitData);
        _originalCall(a_this, a_hitData);
    }

} // namespace Hooks
