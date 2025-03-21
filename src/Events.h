#pragma once
#include <Conditions.h>
#include <Hooks.h>
#include <InputHandler.h>
#include <RecentHitEventData.h>
#include <TimedBlockHandler.h>

using EventResult = RE::BSEventNotifyControl;

class OnHitEventHandler : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
    std::atomic_bool do_once = false;
    std::multimap<std::uint32_t, RecentHitEventData> recentGeneralHits;

    static OnHitEventHandler* GetSingleton()
    {
        static OnHitEventHandler singleton;
        return &singleton;
    }

    // TODO-Temp fix until I can upgrade clib versions
    std::uint32_t GetDurationOfApplicationRunTime()
    {
        REL::Relocation<std::uint32_t*> runtime{ RELOCATION_ID(523662, 410201) };
        return *runtime;
    }

    EventResult ProcessEvent(const RE::TESHitEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override
    {
        auto tb = TimedBlockHandler::BlockHandler::GetSingleton();
        using HitFlag = RE::TESHitEvent::Flag;
        if (!a_event || !a_event->target || !a_event->cause) {
            logger::debug("no target, no event, no cause");
            return continueEvent;
        }
        auto defender = a_event->target ? a_event->target->As<RE::Actor>() : nullptr;
        if (!defender) {
            dlog("no defender");
            return continueEvent;
        }
        auto aggressor = a_event->cause ? a_event->cause->As<RE::Actor>() : nullptr;
        if (!aggressor) {
            dlog("no aggressor");
            return continueEvent;
        }
        // Credits: https://github.com/colinswrath/handtohand/blob/main/src/Events.h for hand to hand event

        auto applicationRuntime = GetDurationOfApplicationRunTime();

        bool skipEvent = ShouldSkipHitEvent(aggressor, defender, applicationRuntime);	//Filters out dupe events

        if (!skipEvent) {

            if (!a_event->flags.any(HitFlag::kBashAttack) && a_event->target) {

                auto attacking_weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);
                if (!defender || !defender->GetActorRuntimeData().currentProcess || !defender->GetActorRuntimeData().currentProcess->high
                    || !defender->Get3D())
                {
                    dlog("arrow event, first continue");
                    return continueEvent;
                }

                if (!aggressor || !aggressor->GetActorRuntimeData().currentProcess || !aggressor->GetActorRuntimeData().currentProcess->high) {
                    dlog("Arrow Attack Actor Not Found!");
                    return continueEvent;
                }                
                if (Settings::debug_logging) {
                    if (a_event->source) {
                        auto source = RE::TESForm::LookupByID(a_event->source);
                        auto type = source->GetFormType();
                        auto name = source->GetName();
                        dlog("event source is {} with a type of {}", name, type);
                    }
                }                
                if (a_event->flags.any(HitFlag::kHitBlocked) && a_event->target && !a_event->projectile) {
                    logger::debug("entered block event");
                    attacking_weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);
                    if (!defender || !attacking_weap || !defender->GetActorRuntimeData().currentProcess || !defender->GetActorRuntimeData().currentProcess->high
                        || !defender->Get3D() && attacking_weap->IsMelee())
                    {
                        dlog("block event, first continue");
                        return continueEvent;
                    }                    

                    if (!aggressor || !aggressor->GetActorRuntimeData().currentProcess || !aggressor->GetActorRuntimeData().currentProcess->high) {
                        dlog("Attack Actor Not Found!");
                        return continueEvent;
                    }
                    auto data_aggressor = aggressor->GetActorRuntimeData().currentProcess->high->attackData;
                    if (!data_aggressor) {
                        dlog("Attacker Attack Data Not Found!");
                        return continueEvent;
                    }                    

                    auto leftHand = defender->GetEquippedObject(true);
                    auto rightHand = defender->GetEquippedObject(false);

                    if (leftHand && leftHand->IsArmor()) {                        
                        dlog("left hand is shield");
                        if (defender->IsPlayerRef()) {
                            tb->ProcessHitEventForParryShield(defender, aggressor, true);
                            tb->PlaySparks(defender);
                        }
                        else {
                            tb->PlaySparks(defender);
                        }
                    }
                    else if (rightHand && rightHand->IsWeapon()) {
                        dlog("left hand is empty");
                        if (defender->IsPlayerRef()) {
                            dlog("blocker is player");
                            tb->ProcessHitEventForParry(defender, aggressor);
                            tb->PlaySparks(defender);
                        }
                        else {
                            tb->PlaySparks(defender);
                        }
                    }
                }

                if (!a_event || !a_event->cause || !a_event->cause->IsPlayerRef() || a_event->target->IsNot(RE::FormType::ActorCharacter) || !a_event->source || a_event->projectile) {
                    return continueEvent;
                }
                auto defenderProcess = defender->GetActorRuntimeData().currentProcess;
                auto attackingWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);

                if (!defender || !attackingWeapon || !defenderProcess || !defenderProcess->high || !attackingWeapon->IsMelee() || !defender->Get3D()) {
                    return continueEvent;
                }

                auto player = a_event->cause->As<RE::Actor>();
                auto playerAttkData = player->GetActorRuntimeData().currentProcess->high->attackData;

                if (!playerAttkData) {
                    return continueEvent;
                };
                if ((defender->AsActorState()->GetLifeState() != RE::ACTOR_LIFE_STATE::kDead) && !IsBeastRace() && attackingWeapon->IsHandToHandMelee()) {
                    dlog("H2H start to apply hand to hand xp");
                    ApplyHandToHandXP();
                }
            }
        }
        // Block event. Sparks and parries

        return continueEvent;
    }    

    void StartMultiShot(RE::Actor* attacker, RE::Actor* target) {
        auto weap = Conditions::getWieldingWeapon(attacker);
        SKSE::GetTaskInterface()->AddTask([=] {
        Conditions::LaunchExtraArrow(attacker, attacker->GetCurrentAmmo(), weap, "", -1, target, nullptr);
            });
    }

    void LaunchArrowRain(RE::Actor* attacker, RE::Actor* target, float a_area) {
        Settings* settings = Settings::GetSingleton();
        
        if (Conditions::getWieldingWeapon(attacker)->IsBow()) 
        {
            if(attacker->HasPerk(settings->ArrowRainPerk) && !Conditions::ActorHasActiveEffect(attacker, settings->ArrowRainCooldownEffect))
            {
                // separation needed to apply poisons to arrow rain
                RE::PlayerCharacter* player = Cache::GetPlayerSingleton(); 
                if (attacker == player) {
                    Conditions::ApplySpell(attacker, attacker, settings->ArrowRainCooldownSpell);
                    for (int i = 0; i < 75; i++) {
                        SKSE::GetTaskInterface()->AddTask([=] {
                            Conditions::ArrowRain(attacker, attacker->GetCurrentAmmo(), target, target, a_area, 500.0f, player->GetInfoRuntimeData().pendingPoison);
                            });                   
                    }
                }
                else {
                    do_once = true;
                    for (int i = 0; i < 75; i++) {
                        SKSE::GetTaskInterface()->AddTask([=] {
                            Conditions::ArrowRain(attacker, attacker->GetCurrentAmmo(), target, target, a_area, 500.0f, nullptr);
                            });                   
                    }
                }
            }
                        
        }
    }



    void ApplyHandToHandXP()
    {
        auto player = RE::PlayerCharacter::GetSingleton();

        float HandToHandLevel = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLockpicking);

        float baseXP = (Settings::BonusXPPerLevel * HandToHandLevel) + Settings::BaseXP;

        player->AddSkillExperience(RE::ActorValue::kLockpicking, baseXP);
        dlog("[HAND TO HAND XP] granted {} of hand to hand experience", baseXP);
    }

    bool IsBeastRace()
    {
        RE::MenuControls* MenuControls = RE::MenuControls::GetSingleton();
        return MenuControls->InBeastForm();
    }

    bool ShouldSkipHitEvent(RE::Actor* causeActor, RE::Actor* targetActor, std::uint32_t runTime)
    {
        bool skipEvent = false;

        auto matchedHits = recentGeneralHits.equal_range(runTime);
        for (auto it = matchedHits.first; it != matchedHits.second; ++it) {
            if (it->second.cause == causeActor && it->second.target == targetActor) {
                skipEvent = true;
                break;
            }
        }

        auto upper = recentGeneralHits.lower_bound(runTime);
        auto it    = recentGeneralHits.begin();
        while (it != upper) {
            it = recentGeneralHits.erase(it);
        }

        return skipEvent;
    }

    static void Register()
    {
        RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventHolder->AddEventSink(OnHitEventHandler::GetSingleton());
    }
};

class AnimationGraphEventHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent>,
                                   public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
                                   public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent>
{
public:
    static AnimationGraphEventHandler* GetSingleton()
    {
        static AnimationGraphEventHandler singleton;
        return &singleton;
    }

    static void InstallHook()
    {
        logger::info("Installing animation event hook...");
        REL::Relocation<uintptr_t> AnimEventVtbl_NPC{ RE::VTABLE_Character[2] };
        REL::Relocation<uintptr_t> AnimEventVtbl_PC{ RE::VTABLE_PlayerCharacter[2] };

        _ProcessEvent_NPC = AnimEventVtbl_NPC.write_vfunc(0x1, ProcessEvent_NPC);
        _ProcessEvent_PC  = AnimEventVtbl_PC.write_vfunc(0x1, ProcessEvent_PC);
    }

    inline static void StaminaCost(RE::Actor* actor, double cost);
    inline static void RestoreStamina(RE::Actor* actor, double staminaAmount);

    const char* jumpAnimEventString = "JumpUp";

    // Anims

    static bool IsInBeastRace()
    {
        RE::MenuControls* MenuControls = RE::MenuControls::GetSingleton();
        return MenuControls->InBeastForm();
    }

    static void HandleJumpAnim()
    {
        auto settings = Settings::GetSingleton();
        auto player   = RE::PlayerCharacter::GetSingleton();
        if (!player->IsGodMode()) {
            Conditions::ApplySpell(player, player, settings->jumpSpell);
        }
    }

    // Anims
    RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!a_event->tag.empty() && a_event->holder && a_event->holder->As<RE::Actor>()) {
            // dlog("event is {}", a_event->tag.c_str());
            if (std::strcmp(a_event->tag.c_str(), jumpAnimEventString) == 0) {
                if (a_event->holder && a_event->holder->As<RE::Actor>() && !IsInBeastRace()) {
                    HandleJumpAnim();
                    dlog("jump happened and player is a {} and the event is {}", a_event->holder->As<RE::Actor>()->GetRace()->GetName(), a_event->tag.c_str());
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    // Object load
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto actor = RE::TESForm::LookupByID<RE::Actor>(a_event->formID);
        if (!actor || !actor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Register for anim event
        actor->AddAnimationGraphEventSink(AnimationGraphEventHandler::GetSingleton());

        return RE::BSEventNotifyControl::kContinue;
    }

    // Race Switch
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent*                                a_event,
                                          [[maybe_unused]] RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>* a_eventSource) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto actor = a_event->subject->As<RE::Actor>();
        if (!actor || !actor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Register for anim event
        actor->AddAnimationGraphEventSink(AnimationGraphEventHandler::GetSingleton());
        dlog("added animation graph");

        return RE::BSEventNotifyControl::kContinue;
    }

    static void Register()
    {
        // Register for load event, then in the load event register for anims
        RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
        eventHolder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
    }

    static void RegisterAnimHook() { InstallHook(); }

private:
    inline static void ProcessJump(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                   RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

    inline static void ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

    static EventResult ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                        RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);
    static EventResult ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                       RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

    inline static REL::Relocation<decltype(ProcessEvent_NPC)> _ProcessEvent_NPC;
    inline static REL::Relocation<decltype(ProcessEvent_PC)>  _ProcessEvent_PC;
};

class WeaponFireHandler
{
public:
    static void InstallArrowReleaseHook()
    {
        logger::info("Writing arrow release handler hook");

        auto& trampoline = SKSE::GetTrampoline();
        _Weapon_Fire     = trampoline.write_call<5>(Hooks::arrow_release_handler.address(), WeaponFire);

        logger::info("Release arrow hooked");
    }

    static void WeaponFire(RE::TESObjectWEAP* a_weapon, RE::TESObjectREFR* a_source, RE::TESAmmo* a_ammo, RE::EnchantmentItem* a_ammoEnchantment, RE::AlchemyItem* a_poison)
    {
        _Weapon_Fire(a_weapon, a_source, a_ammo, a_ammoEnchantment, a_poison);

        if (!a_source) {
            return;
        }

        auto source = a_source->As<RE::Actor>();

        if (source->IsPlayerRef() && a_weapon->IsCrossbow()) {
            Conditions::ApplySpell(source, source, Settings::GetSingleton()->MAGCrossbowStaminaDrainSpell);
        }
    }

    inline static REL::Relocation<decltype(WeaponFire)> _Weapon_Fire;
};