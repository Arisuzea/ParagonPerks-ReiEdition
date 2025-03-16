#include "Events.h"

void AnimationGraphEventHandler::StaminaCost(RE::Actor* actor, double cost) {
    RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
    if (actor == player && !player->IsGodMode()) {
        logger::debug("attacks costs {} stamina", cost);
    }
    actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -static_cast<float>(cost));
}

void AnimationGraphEventHandler::RestoreStamina(RE::Actor* actor, double staminaAmount) {
    if (actor && staminaAmount > 0) {
        actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, static_cast<float>(staminaAmount));
    }
}

void AnimationGraphEventHandler::ProcessJump(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                            RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
    const char* jumpAnimEventString = "JumpUp";

    if (!a_event) {
        return;
    }
    if (!a_event->tag.empty() && a_event->holder && a_event->holder->As<RE::Actor>()) {
        if (std::strcmp(a_event->tag.c_str(), jumpAnimEventString) == 0) {
            HandleJumpAnim();
        }
    }
}

void AnimationGraphEventHandler::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
    const char* HitString = "HitFrame";
    const char* DodgeString = "TKDR_DodgeStart"; // Test to make stuff while dodging
    Settings* settings = Settings::GetSingleton();

    if (!a_event) {
        return;
    }

    if (!a_event->tag.empty() && a_event->holder && a_event->holder->As<RE::Actor>()) {
        if (std::strcmp(a_event->tag.c_str(), HitString) == 0) {
            RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
            auto actor = const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>();
            auto wieldedWeap = Conditions::getWieldingWeapon(actor);
            RE::TESGlobal* stamGlob = settings->StaminaCostGlobal;
            auto global = stamGlob->value;
            auto npc_glob = settings->NPCStaminaCostGlobal->value;
            double stam_cost = global;
            double dual_wield_mod = global * 1.2;

            if (actor == player) {
                if (wieldedWeap && wieldedWeap->IsWeapon() && wieldedWeap->IsMelee()) {
                    bool dagger = wieldedWeap->IsOneHandedDagger();
                    bool sword = wieldedWeap->IsOneHandedSword();
                    bool mace = wieldedWeap->IsOneHandedMace();
                    bool axe = wieldedWeap->IsOneHandedAxe();
                    bool greatsword = wieldedWeap->IsTwoHandedSword();
                    bool greataxe = wieldedWeap->IsTwoHandedAxe();

                    if (sword || axe || mace) {
                        if (Conditions::IsDualWielding(actor)) {
                            stam_cost = global * dual_wield_mod;
                        } else {
                            stam_cost = global;
                        }
                    } else if (greatsword || greataxe) {
                            stam_cost = global * 1.5;
                    } else if (dagger || wieldedWeap->IsHandToHandMelee()) {
                        if (dagger && Conditions::IsDualWielding(actor)) {
                            stam_cost = global * 0.8 * dual_wield_mod;
                        } else {
                            stam_cost = global * 0.8;
                        }
                    }
                }
                if (player->IsGodMode()) {
                    stam_cost = 0.0;
                }
            } else if (actor != player) {
                if (wieldedWeap && wieldedWeap->IsWeapon() && wieldedWeap->IsMelee()) {
                    bool dagger = wieldedWeap->IsOneHandedDagger();
                    bool sword = wieldedWeap->IsOneHandedSword();
                    bool mace = wieldedWeap->IsOneHandedMace();
                    bool axe = wieldedWeap->IsOneHandedAxe();
                    bool greatsword = wieldedWeap->IsTwoHandedSword();
                    bool greataxe = wieldedWeap->IsTwoHandedAxe();

                    if (sword || axe || mace) {
                        if (Conditions::IsDualWielding(actor)) {
                            stam_cost = npc_glob * dual_wield_mod;
                        } else {
                            stam_cost = npc_glob;
                        }
                    } else if (greatsword || greataxe) {
                        stam_cost = npc_glob * 1.5;
                    } else if (dagger || wieldedWeap->IsHandToHandMelee()) {
                        if (dagger && Conditions::IsDualWielding(actor)) {
                            stam_cost = npc_glob * 0.8 * dual_wield_mod;
                        } else {
                            stam_cost = npc_glob * 0.8;
                        }
                    }
                }
            }

            if (!Conditions::IsPowerAttacking(actor)) {
                StaminaCost(actor, stam_cost);
            }

            // Restore on light attacks and none for power or sprint attacks
            if (actor == player && !Conditions::IsPowerAttacking(actor) && !Conditions::IsSprinting(actor)) {
                double staminaRestore = stam_cost * 2; // Restore 2x of the stamina cost
                RestoreStamina(actor, staminaRestore);
                logger::debug("Attack hit! Stamina restored by {}.", staminaRestore);
            }

        }

        if (std::strcmp(a_event->tag.c_str(), DodgeString) == 0) {
            if (a_event->holder->As<RE::Actor>()->HasPerk(settings->dummyPerkDodge)) {
                logger::debug("Dodge happened");
                RE::PlayerCharacter* player = Cache::GetPlayerSingleton();
                RE::NiPoint3 playerPos;
                playerPos.x = player->GetPositionX();
                playerPos.y = player->GetPositionY();
                playerPos.z = player->GetPositionZ();
                Conditions::CastSpellFromPointToPoint(player, settings->DodgeRuneSpell, playerPos.x, playerPos.y, playerPos.z + 10, playerPos.x, playerPos.y,
                    playerPos.z - 150);
            }
        }
    }
}

EventResult AnimationGraphEventHandler::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                        RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
    ProcessEvent(a_sink, a_event, a_eventSource);
    return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
}

EventResult AnimationGraphEventHandler::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                       RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
    ProcessEvent(a_sink, a_event, a_eventSource);
    return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
}

