#pragma once

namespace Hooks
{
    inline static REL::Relocation<std::uintptr_t> GetStaminaDamageHook(REL::RelocationID(25864, 26430)); // 1.5,1.6
    inline static REL::Relocation<std::uintptr_t> GetStaminaBashHook(REL::RelocationID(25863, 26429));   // 1.5,1.6
    inline static REL::Relocation<std::uintptr_t> OnFrame_Update_Hook{ REL::RelocationID(35565, 36564), REL::Relocate(0x1E, 0x6E) };
    inline static REL::Relocation<std::uintptr_t> Scale_Patch_Hook{ REL::RelocationID(37013, 37943), REL::Relocate(0x1A, 0x51) };
    inline static REL::Relocation<std::uintptr_t> Block_GameSetting_Hook{ REL::RelocationID(42842, 44014), REL::Relocate(0x452, 0x438) };
    inline static REL::Relocation<std::uintptr_t> fBlock_GameSetting{ REL::RelocationID(505023, 374158), 0x8 };
    inline static REL::Relocation<std::uintptr_t> SpellCap_Hook{ REL::RelocationID(37792, 38741), REL::Relocate(0x53, 0x55) };
    inline REL::Relocation<uintptr_t>             arrow_release_handler{ REL::RelocationID(41778, 42859), REL::Relocate(0x133, 0x138) };
    static REL::Relocation<std::uintptr_t>        armorRating1(REL::RelocationID(42842, 44014));
    static REL::Relocation<std::uintptr_t>        armorRating2(REL::RelocationID(37605, 38558));
    inline static REL::Relocation<std::uintptr_t> launchArrow(REL::RelocationID(17693, 18102), REL::VariantOffset(0xE82, 0xe60, 0xE82));

    bool InstallHooks();
    bool InstallBashMultHook();

    class ActorUpdateHook
    {
    public: 
        static void InstallUpdateActor();
    private:
        static void ActorUpdate(RE::Character* a_this, float a_delta);
        static inline REL::Relocation<decltype(&ActorUpdate)> _ActorUpdate;
    };
    //Fenix
    class CombatHit {
    public:
        static void Install();

    private:
        static float PitFighter(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow);
        static inline REL::Relocation<decltype(&PitFighter)> _originalCall;
    };
    // RE Discord Server po3/doodlez
    class BowHit {
    public:
        static void Install();
    private:
        static float PitFighterBow(float a1, float a2);
        static inline REL::Relocation<decltype(&PitFighterBow)> _originalCall;
    };
    // po3: https://github.com/powerof3/MagicSneakAttacks/blob/275255b26492115557c7bfa3cb7c4a79e83f2f3d/src/Hooks.cpp#L29
    class AdjustActiveEffect
    {
    public:        
        static void Install();

    private:
        static void AdjustSpells(RE::ActiveEffect* a_this, float a_power, bool a_onlyHostile);
        static inline REL::Relocation<decltype(&AdjustSpells)> func;
    };
} // namespace Hooks
