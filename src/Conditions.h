#pragma once
#include "Cache.h"
#include "Settings.h"
#include "Hooks.h"
#include "API/TrueHUDAPI.h"
#include <numbers>

// Originally intended to just implement some condition functions but iv been placing extensions/utility here as well
namespace Conditions
{
    class APIuse
    {

    public: 
        TRUEHUD_API::IVTrueHUD3* ersh_TrueHUD = nullptr;
        static APIuse* GetSingleton()
        {
            static APIuse singleton;
            return  std::addressof(singleton);
        }

    };

    inline static bool IsSprinting(RE::Actor* actor) {
        if (actor) {
            return actor->AsActorState()->IsSprinting();
        }
        return false;
    }

    inline static int GetRandomINT(int a_min, int a_max)
    {
        static std::random_device       rd;
        static std::mt19937             gen(rd());
        std::uniform_int_distribution<int> distrib(a_min, a_max);
        return distrib(gen);
    }

    inline static double GetRandomDouble(double a_min, double a_max)
    {
        static std::random_device       rd;
        static std::mt19937             gen(rd());
        std::uniform_real_distribution<> distrib(a_min, a_max);
        return distrib(gen);
    }

    inline static float GetPointInRange(float radius, float start_point) 
    {
        float r = radius * std::sqrt((float)GetRandomDouble(0.0f, 1.0f));
        auto theta = (float)GetRandomDouble(0.0f, 1.0f) * 2.0f * std::numbers::pi_v<float>;
        return start_point + r * std::cos(theta);
    }

    static bool IsAttacking(RE::Actor* actor)
    {
        using func_t = decltype(&Conditions::IsAttacking);
        REL::Relocation<func_t> func{ Cache::IsAttackingAddress };
        return func(actor);
    }

    inline static REL::Relocation<decltype(IsAttacking)> _IsAttacking;

    static bool IsBlocking(RE::Actor* actor)
    {
        using func_t = decltype(&Conditions::IsBlocking);
        REL::Relocation<func_t> func{ Cache::IsBlockingAddress };
        return func(actor);
    }

    inline static REL::Relocation<decltype(IsBlocking)> _IsBlocking;

    static bool HasSpell(RE::Actor* actor, RE::SpellItem* spell)
    {
        using func_t = decltype(&Conditions::HasSpell);

        REL::Relocation<func_t> func{ Cache::HasSpellAddress };

        return func(actor, spell);
    }

    inline static REL::Relocation<decltype(HasSpell)> _HasSpell;

    inline static bool IsMoving(RE::PlayerCharacter* player)
    {
        auto playerState = player->AsActorState();
        return (static_cast<bool>(playerState->actorState1.movingForward) || static_cast<bool>(playerState->actorState1.movingBack)
                || static_cast<bool>(playerState->actorState1.movingLeft) || static_cast<bool>(playerState->actorState1.movingRight));
    }

    inline static RE::TESObjectWEAP* GetUnarmedWeapon()
    {
        auto** singleton{ reinterpret_cast<RE::TESObjectWEAP**>(Cache::getUnarmedWeaponAddress) };
        return *singleton;
    }

    inline static bool ActorHasActiveEffect(RE::Actor* a_actor, RE::EffectSetting* a_effect) {

        auto               activeEffects = a_actor->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting* setting       = nullptr;
        if (!activeEffects->empty()) {
            for (RE::ActiveEffect* effect : *activeEffects) {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive)) {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting) {
                        if (setting == a_effect) {
                            return true;
                        }
                    }
                }
            }
        }        
        return false;
    }

    inline static bool PlayerHasActiveMagicEffect(RE::EffectSetting* a_effect)
    {
        auto player = RE::PlayerCharacter::GetSingleton();

        auto               activeEffects = player->AsMagicTarget()->GetActiveEffectList();
        RE::EffectSetting* setting       = nullptr;
        if (!activeEffects->empty()) {
            for (RE::ActiveEffect* effect : *activeEffects) {
                if (effect; !effect->flags.any(RE::ActiveEffect::Flag::kInactive)) {
                    setting = effect ? effect->GetBaseObject() : nullptr;
                    if (setting) {
                        if (setting == a_effect) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    inline static float GetMaxHealth()
    {
        auto player = RE::PlayerCharacter::GetSingleton();

        return player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth)
               + player->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kHealth);
    }

    inline static bool IsPowerAttacking(RE::Actor* actor)
    {
        if (auto high = actor->GetHighProcess()) {
            if (const auto attackData = high->attackData) {
                auto flags = attackData->data.flags;

                if (flags && flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Credit: D7ry for getWieldingWeapon in ValhallaCombat
    // https://github.com/D7ry/valhallaCombat/blob/48fb4c3b9bb6bbaa691ce41dbd33f096b74c07e3/src/include/Utils.cpp#L10
    inline static RE::TESObjectWEAP* getWieldingWeapon(RE::Actor* a_actor)
    {
        bool dual_wielding = false;
        auto weapon        = a_actor->GetAttackingWeapon();
        if (weapon) {
            dual_wielding = false;
            return weapon->object->As<RE::TESObjectWEAP>();
        }
        auto rhs = a_actor->GetEquippedObject(false);
        if (rhs && rhs->IsWeapon()) {
            dual_wielding = false;
            return rhs->As<RE::TESObjectWEAP>();
        }
        auto lhs = a_actor->GetEquippedObject(true);
        if (lhs && lhs->IsWeapon()) {
            dual_wielding = false;
            return lhs->As<RE::TESObjectWEAP>();
        }

        return nullptr;
    }

    inline static bool IsDualWielding(RE::Actor* a_actor)
    {
        auto weapon = a_actor->GetAttackingWeapon();
        auto rhs    = a_actor->GetEquippedObject(false);
        auto lhs    = a_actor->GetEquippedObject(true);
        if (weapon && rhs && lhs && lhs->IsWeapon() && rhs->IsWeapon()) {
            logger::debug("dual wielding is active");
            return true;
        }
        else
            return false;
    }

    inline static std::vector<RE::Actor*> GetNearbyActors(RE::TESObjectREFR* a_ref, float a_radius, bool a_ignorePlayer)
    {
        {
            std::vector<RE::Actor*> result;
            if (const auto processLists = RE::ProcessLists::GetSingleton(); processLists) {
                if (a_ignorePlayer && processLists->numberHighActors == 0) {
                    dlog("no process list");
                    return result;
                }

                const auto squaredRadius = a_radius * a_radius;
                const auto originPos     = a_ref->GetPosition();

                result.reserve(processLists->numberHighActors);

                const auto get_actor_within_radius = [&](RE::Actor* a_actor) {
                    if (a_actor && a_actor != a_ref && originPos.GetSquaredDistance(a_actor->GetPosition()) <= squaredRadius) {
                        result.emplace_back(a_actor);
                    }
                };
                for (auto& actorHandle : processLists->highActorHandles) {
                    const auto actor = actorHandle.get();
                    get_actor_within_radius(actor.get());
                }

                if (!a_ignorePlayer) {
                    get_actor_within_radius(Cache::GetPlayerSingleton());
                }

                if (!result.empty()) {
                    dlog("vector is not empty");
                    return result;
                }
            }
            return result;
        }
    }

    inline static std::int32_t NumNearbyActors(RE::TESObjectREFR* a_ref, float a_radius, bool a_ignorePlayer)
    {
        {
            std::int32_t num_result = 0;
            
            if (const auto processLists = RE::ProcessLists::GetSingleton(); processLists) {
                if (a_ignorePlayer && processLists->numberHighActors == 0) {
                    dlog("no process list");
                    return num_result;
                }

                const auto squaredRadius = a_radius * a_radius;
                const auto originPos     = a_ref->GetPosition();

                std::vector<RE::Actor*> result;
                result.reserve(processLists->numberHighActors);

                const auto get_actor_within_radius = [&](RE::Actor* a_actor) {
                    if (a_actor && a_actor != a_ref && originPos.GetSquaredDistance(a_actor->GetPosition()) <= squaredRadius) {
                        result.emplace_back(a_actor);
                    }
                    };
                for (auto& actorHandle : processLists->highActorHandles) {
                    const auto actor = actorHandle.get();
                    get_actor_within_radius(actor.get());
                }

                if (!a_ignorePlayer) {
                    get_actor_within_radius(Cache::GetPlayerSingleton());
                }

                if (!result.empty()) {
                    for (const auto& enemy : result) {
                        if (enemy && !enemy->IsDead()) {
                            num_result++;
                        }
                    }
                    return num_result;
                }
            }
            return num_result;
        }
    }

    // credits: https://github.com/Sacralletius/ANDR_SKSEFunctions currently unused though
    struct ProjectileRot
    {
        float x, z;
    };

    inline float SkyrimSE_c51f70(RE::NiPoint3* dir)
    {
        using func_t = decltype(SkyrimSE_c51f70);
        REL::Relocation<func_t> func{ REL::RelocationID(68820, 70172) };
        return func(dir);
    }

    inline ProjectileRot rot_at(RE::NiPoint3 dir)
    {
        ProjectileRot rot;
        auto          len = dir.Unitize();
        if (len == 0) {
            rot = { 0, 0 };
        }
        else {
            float polar_angle = SkyrimSE_c51f70(&dir);
            rot               = { -asin(dir.z), polar_angle };
        }
        return rot;
    }

    inline ProjectileRot rot_at(const RE::NiPoint3& from, const RE::NiPoint3& to)
    {
        return rot_at(to - from);
    }

    inline static void LaunchExtraArrow(RE::Actor* a_actor, RE::TESAmmo* a_ammo, RE::TESObjectWEAP* a_weapon, RE::BSFixedString a_nodeName, std::int32_t a_source, RE::TESObjectREFR* a_target, RE::AlchemyItem* a_poison)
    {     
        SKSE::GetTaskInterface()->AddTask([a_actor, a_ammo, a_weapon, a_nodeName, a_source, a_target, a_poison]() {
            RE::NiAVObject* fireNode = nullptr;
            auto            root = a_actor->IsPlayerRef() ? a_actor->GetCurrent3D() : a_actor->Get3D2();
            switch (a_source) {
            case -1:
            {
                if (!a_nodeName.empty()) {
                    if (root) {
                        fireNode = root->GetObjectByName(a_nodeName);
                    }
                }
                else {
                    if (const auto currentProcess = a_actor->GetActorRuntimeData().currentProcess) {
                        const auto& biped = a_actor->GetBiped2();
                        fireNode = a_weapon->IsCrossbow() ? currentProcess->GetMagicNode(biped) : currentProcess->GetWeaponNode(biped);
                    }
                    else {
                        fireNode = a_weapon->GetFireNode(root);
                    }
                }
            }
            break;
            case 0:
                fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcLMagicNode) : nullptr;
                break;
            case 1:
                fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcRMagicNode) : nullptr;
                break;
            case 2:
                fireNode = root ? root->GetObjectByName(RE::FixedStrings::GetSingleton()->npcHeadMagicNode) : nullptr;
                break;
            default:
                break;
            }
            RE::NiPoint3                  origin;
            RE::Projectile::ProjectileRot angles{};
            if (fireNode) {
                origin = fireNode->world.translate;
                a_actor->Unk_A0(fireNode, angles.x, angles.z, origin);
            }
            else {
                origin = a_actor->GetPosition();
                origin.z += 96.0f;

                angles.x = a_actor->GetAimAngle();
                angles.z = a_actor->GetAimHeading();
            }

            RE::ProjectileHandle       handle{};
            RE::Projectile::LaunchData launchData(a_actor, origin, angles, a_ammo, a_weapon);
            launchData.desiredTarget = a_target;
            launchData.poison = a_poison;
            launchData.enchantItem = a_weapon->formEnchanting;
            auto projHandle = RE::Projectile::Launch(&handle, launchData);
            });

    }

    inline static void ArrowRain(RE::Actor* a_Source, RE::TESAmmo* a_arrow, RE::Actor* start_actor, RE::Actor* target, float range, float extra_height, RE::AlchemyItem* a_poison)
    {
        RE::NiPoint3 StartPos;
        StartPos.x = start_actor->GetPositionX();
        StartPos.y = start_actor->GetPositionY();
        StartPos.z = start_actor->GetPositionZ() + extra_height;

        RE::NiPoint3 EndPos;

        EndPos.x = GetPointInRange(range, target->GetPositionX());
        EndPos.y = GetPointInRange(range, target->GetPositionY());
        EndPos.z = target->GetPositionZ();
        //logger::debug("end position is: \ X:{} \n Y:{} \n Z:{}", EndPos.x, EndPos.y, EndPos.z);

        auto rot = rot_at(StartPos, EndPos);

        RE::Projectile::LaunchData ldata;
        ldata.origin                = StartPos;
        ldata.contactNormal         = { 0.0f, 0.0f, 0.0f };
        ldata.projectileBase        = a_arrow->GetRuntimeData().data.projectile;
        ldata.shooter               = a_Source;
        ldata.combatController      = a_Source->GetActorRuntimeData().combatController;
        ldata.weaponSource          = getWieldingWeapon(a_Source);
        ldata.ammoSource            = a_arrow;
        ldata.angleZ                = rot.z;
        ldata.angleX                = rot.x;
        ldata.unk50                 = nullptr;
        ldata.desiredTarget         = target;
        ldata.unk60                 = 0.0f;
        ldata.unk64                 = 0.0f;
        ldata.parentCell            = a_Source->GetParentCell();
        ldata.spell                 = nullptr;
        ldata.castingSource         = RE::MagicSystem::CastingSource::kOther;
        ldata.pad7C                 = 0;
        ldata.enchantItem           = nullptr;
        ldata.poison                = a_poison;
        ldata.area                  = 0;
        ldata.power                 = 1.0f;
        ldata.scale                 = 1.0f;
        ldata.alwaysHit             = false;
        ldata.noDamageOutsideCombat = false;
        ldata.autoAim               = false;
        ldata.chainShatter          = false;
        ldata.useOrigin             = true;
        ldata.deferInitialization   = false;
        ldata.forceConeOfFire       = false;
        RE::BSPointerHandle<RE::Projectile> handle;
        RE::Projectile::Launch(&handle, ldata);
            
        
    }

    inline static void CastSpellFromPointToPoint(RE::Actor* akSource, RE::SpellItem* akSpell, float StartPoint_X, float StartPoint_Y, float StartPoint_Z, float EndPoint_X,
                                                 float EndPoint_Y, float EndPoint_Z)
    {
        RE::NiPoint3 NodePosition;

        NodePosition.x = StartPoint_X;
        NodePosition.y = StartPoint_Y;
        NodePosition.z = StartPoint_Z;

        logger::debug("NodePosition: X = {}, Y = {}, Z = {}.", NodePosition.x, NodePosition.y, NodePosition.z);

        RE::NiPoint3 DestinationPosition;

        DestinationPosition.x = EndPoint_X;
        DestinationPosition.y = EndPoint_Y;
        DestinationPosition.z = EndPoint_Z;

        logger::debug("DestinationPosition: X = {}, Y = {}, Z = {}.", DestinationPosition.x, DestinationPosition.y, DestinationPosition.z);

        auto rot = rot_at(NodePosition, DestinationPosition);

        auto eff = akSpell->GetCostliestEffectItem();

        auto mgef = akSpell->GetAVEffect();

        RE::Projectile::LaunchData ldata;

        ldata.origin                = NodePosition;
        ldata.contactNormal         = { 0.0f, 0.0f, 0.0f };
        ldata.projectileBase        = mgef->data.projectileBase;
        ldata.shooter               = akSource;
        ldata.combatController      = akSource->GetActorRuntimeData().combatController;
        ldata.weaponSource          = nullptr;
        ldata.ammoSource            = nullptr;
        ldata.angleZ                = rot.z;
        ldata.angleX                = rot.x;
        ldata.unk50                 = nullptr;
        ldata.desiredTarget         = nullptr;
        ldata.unk60                 = 0.0f;
        ldata.unk64                 = 0.0f;
        ldata.parentCell            = akSource->GetParentCell();
        ldata.spell                 = akSpell;
        ldata.castingSource         = RE::MagicSystem::CastingSource::kOther;
        ldata.pad7C                 = 0;
        ldata.enchantItem           = nullptr;
        ldata.poison                = nullptr;
        ldata.area                  = eff->GetArea();
        ldata.power                 = 1.0f;
        ldata.scale                 = 1.0f;
        ldata.alwaysHit             = false;
        ldata.noDamageOutsideCombat = false;
        ldata.autoAim               = false;
        ldata.chainShatter          = false;
        ldata.useOrigin             = true;
        ldata.deferInitialization   = false;
        ldata.forceConeOfFire       = false;
        RE::BSPointerHandle<RE::Projectile> handle;
        RE::Projectile::Launch(&handle, ldata);
    }
    
    // Credit: KernalsEgg for ApplySpell and IsPermanent
    // extensions
    static bool IsPermanent(RE::MagicItem* item)
    {
        switch (item->GetSpellType()) {
        case RE::MagicSystem::SpellType::kDisease:
        case RE::MagicSystem::SpellType::kAbility:
        case RE::MagicSystem::SpellType::kAddiction: {
            return true;
        }
        default: {
            return false;
        }
        }
    }

    inline static bool isInBlockAngle(RE::Actor* blocker, RE::TESObjectREFR* a_obj) 
    {
        auto angle = blocker->GetHeadingAngle(a_obj->GetPosition(), false);
        return (angle <= Settings::blockAngleSetting && angle >= -Settings::blockAngleSetting);
    }


    inline static void ApplySpell(RE::Actor* caster, RE::Actor* target, RE::SpellItem* spell)
    {
        if (IsPermanent(spell)) {
            target->AddSpell(spell);
        }
        else {
            caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, false, target, 1.0F, false, 0.0F, nullptr);
        }
    }

    inline static bool tryDamageAV(RE::Actor* actor, RE::ActorValue av, float damage_val) {
        if (actor->AsActorValueOwner()->GetActorValue(av) > damage_val + 1) {
            actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -damage_val);
            return true;
        }
        return false;
    }

    inline static float projectileBlockCost(RE::Actor* actor, float a_cost) {
        float new_cost = a_cost;
        if (actor->HasPerk(Settings::BlockStaminaPerk)) {
            return new_cost / 2;
        }
        else 
            return new_cost;
    }


    inline void PlayerGreyoutAvMeter(RE::Actor* a_actor, RE::ActorValue actorValue) {
        if (!Settings::TrueHudAPI_Obtained) {
            return;
        }
        auto ersh = APIuse::GetSingleton()->ersh_TrueHUD;
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::FlashColor, Settings::uColorCodeStamFlash);
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::BarColor, Settings::uColorCodeStamBar);
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::PhantomColor, Settings::uColorCodePhantom);
    }

    inline void PlayerRevertAvMeter(RE::Actor* a_actor, RE::ActorValue actorValue) {
        if (!Settings::TrueHudAPI_Obtained) {
            return;
        }
        auto ersh = APIuse::GetSingleton()->ersh_TrueHUD;
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::FlashColor);
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::BarColor);
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::PhantomColor);
    }


    inline void greyoutAvMeter(RE::Actor* a_actor, RE::ActorValue actorValue) {
        if (!Settings::TrueHudAPI_Obtained) {
            return;
        }        
        auto ersh = APIuse::GetSingleton()->ersh_TrueHUD;
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::FlashColor, Settings::uColorCodeStamFlash);
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::BarColor, Settings::uColorCodeStamBar);
        ersh->OverrideBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::PhantomColor, Settings::uColorCodePhantom);
        dlog("gray Out");
    }

    inline void revertAvMeter(RE::Actor* a_actor, RE::ActorValue actorValue) {
        if (!Settings::TrueHudAPI_Obtained) {
            return;
        }
        auto ersh = APIuse::GetSingleton()->ersh_TrueHUD;
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::FlashColor);
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::BarColor);
        ersh->RevertBarColor(a_actor->GetHandle(), actorValue, TRUEHUD_API::BarColorType::PhantomColor);
    }

    
    

}; // namespace Conditions
