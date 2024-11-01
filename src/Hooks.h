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



    // Valhalla Combat                               May be used later for a timed dodge cooldown mechanic
    // inline static float*                          g_deltaTime         = (float*)RELOCATION_ID(523660, 410199).address(); // 2F6B948
    // inline static float*                          g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address(); // 2F6B94C

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
    class CombatHit {
    public:
        static void Install();

    private:
        static void PitFighter(RE::Actor* a_this, RE::HitData* a_hitData);
        static void CHit(RE::Actor* a_this, RE::HitData* a_hitData);
        static inline REL::Relocation<decltype(&CHit)> _originalCall;
    };

} // namespace Hooks
