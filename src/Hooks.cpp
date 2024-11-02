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
        BowHit::Install();

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
        
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(42832, 44001), REL::VariantOffset(0x1a5, 0x1a4, 0x1a5) };
        _originalCall = trampoline.write_call<5>(target.address(), &PitFighter);


        ////SE: 140628C20 - 37673

        //auto& trampoline = SKSE::GetTrampoline();
        //REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37673, 38627), REL::VariantOffset(0x3c0, 0x4A8, 0x3c0) };
        //_originalCall = trampoline.write_call<5>(target.address(), &CHit);
    }

    /*
    public:
		static void Hook()
		{
            _get_damage = SKSE::GetTrampoline().write_call<5>((RELOCATION_ID(42832, 44001), REL::VariantOffset(0x1a5, 0x1a4, 0x1a5).address), get_damage);  // SkyrimSE.exe+7429F5
		}

        FENIX
    */

    float CombatHit::PitFighter(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow)
    {
        dlog("hook started");
        const Settings* settings = Settings::GetSingleton();
        RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
        RE::Actor* actor = skyrim_cast<RE::Actor*>(a);

        auto dam = _originalCall(_weap, a, DamageMult, isbow);
        if (player->HasPerk(settings->dummyPerkDodge)) {
            if (actor != player) {
                return dam;
            }

            if (Conditions::NumNearbyActors(player, 500.0f, false) > 0) {
                dlog("first condition cehck, there are {} enemies", Conditions::NumNearbyActors(player, 500.0f, false));
                if (!isbow) {
                    logger::debug("----------------------------------------------------");
                    logger::debug("started hooked damage calc: {} was hit with {}", actor->GetName(), DamageMult);
                    std::int32_t enemyNum = Conditions::NumNearbyActors(player, 500.0f, false);
                    dlog("{} is surrounded by {} enemies", player->GetDisplayFullName(), (int)enemyNum);
                    if (enemyNum == 2)
                        dam *= 1.15f;
                    if (enemyNum == 3)
                        dam *= 1.30f;
                    if (enemyNum >= 4)
                        dam *= 1.50f;
                    logger::debug("new damage for melee is {}. Original damage was: {} \n", dam, DamageMult); 
                    return dam;
                }              
            }
        }
        return dam;
    }



    /*
    
    __ ADDRESS AND OFFSET __
    SE ID: 42928 SE Offset: 0x604
    AE ID: 44108 AE Offset: 0x5d6 (Heuristic)
    
    */



    void BowHit::Install()
    {
        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(42928, 44108), REL::VariantOffset(0x604, 0x5d6, 0x604) };
        _originalCall = trampoline.write_call<5>(target.address(), &PitFighterBow);

    }

    float BowHit::PitFighterBow(float a1, float a2)
    {
        RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
        Settings* settings = Settings::GetSingleton();
        auto dam = _originalCall(a1, a2);
        dlog("sanity check");
        dlog("a1 is {}", a1);
        dlog("a2 is {}", a2);
        dlog("pig fighter bow is hooked. dam is: {}", dam);
        if (player->IsInCombat() && player->HasPerk(settings->dummyPerkDodge) && player->IsAttacking()) {
            std::int32_t enemyNum = Conditions::NumNearbyActors(player, 500.0f, false);
            dlog("sanity check, enemy number is {}", enemyNum);
            if (enemyNum >= 4) {                
                dam *= 1.15f;
                dlog("return with more than 4 enemies, dam is {}", dam);
            }                
            if (enemyNum == 3){
                dam *= 1.30f;
                dlog("return with 3 enemies, dam is {}", dam);
            }                
            if (enemyNum <= 2 ) {
                dam *= 1.50f;
                dlog("return with 2 enemies, dam is {}", dam);
            }                
            return dam;
        }
        dlog("return without modifying");
        return dam;
    }

} // namespace Hooks
