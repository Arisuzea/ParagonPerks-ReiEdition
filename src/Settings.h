#pragma once

class Settings
{
public:
    static Settings* GetSingleton();

    void LoadSettings();
    void LoadForms() ;
    void AdjustWeaponStaggerVals();
    void GetIngameData() noexcept;
    void SetGlobalsAndGameSettings();  
    void LoadMCMSettings();
    static void ReadColorStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
    void logFormLoad(std::string form_name, RE::TESForm* a_form);

    // Spells
    RE::SpellItem* IsAttackingSpell;
    RE::SpellItem* IsBlockingSpell;
    RE::SpellItem* IsSneakingSpell;
    RE::SpellItem* IsSprintingSpell;
    RE::SpellItem* MountSprintingSpell;
    RE::SpellItem* BowStaminaSpell;
    RE::SpellItem* XbowStaminaSpell;
    RE::SpellItem* IsCastingSpell;
    RE::SpellItem* MAGParryStaggerSpell;
    RE::SpellItem* MAGParryControllerSpell;
    RE::SpellItem* MAGCrossbowStaminaDrainSpell;
    RE::SpellItem* APOParryBuffSPell;
    RE::SpellItem* jumpSpell;
    RE::SpellItem* PowerAttackStopSpell;
    RE::SpellItem* ArrowRainCooldownSpell;
    RE::SpellItem* MultiShotCooldownSpell;

    RE::SpellItem* DodgeRuneSpell;
    // Perks
    RE::BGSPerk* BashStaminaPerk;
    static inline RE::BGSPerk* BlockStaminaPerk;
    RE::BGSPerk* ArrowRainPerk;
    RE::BGSPerk* MultiShotPerk;
    RE::BGSPerk* PitFighterPerk;
    static inline RE::BGSPerk* MagicParryPerk;
    static inline RE::BGSPerk* ArrowParryPerk;
    // Explosions (Sparks)
    RE::BGSExplosion* APOSparks;
    RE::BGSExplosion* APOSparksPhysics;
    RE::BGSExplosion* APOSparksFlash;
    RE::BGSExplosion* APOSparksShieldFlash;
    // Globals
    RE::TESGlobal* StaminaCostGlobal;
    RE::TESGlobal* NPCStaminaCostGlobal;
    RE::TESGlobal* DualBlockKey;
    // Effects
    static inline RE::EffectSetting* MAG_ParryWindowEffect;
    static inline RE::EffectSetting* ArrowRainCooldownEffect;
    static inline RE::EffectSetting* MultiShotCooldownEffect;
    static inline RE::EffectSetting* StaminaPenaltyEffect;
    static inline RE::EffectSetting* StaminaPenEffectNPC;
    // Conditions
    RE::TESCondition* IsPowerAttacking;

    //tests
    RE::SpellItem* fireBolt;

    // bools
    bool               enableSneakStaminaCost;
    bool               enableLevelDifficulty;
    bool               zeroAllWeapStagger;
    bool               armorScalingEnabled;
    bool               IsBlockingWeaponSpellCasted = false;
    bool               wasPowerAttacking           = false;
    inline static bool               debug_logging{};
    static inline float fCombatHitConeAngle;
    // floats
    inline static float BonusXPPerLevel;
    inline static float BaseXP;
    static inline float               blockAngleSetting;
    float               surroundingActorsRange;
    // int
    inline static uint32_t blockingKey[RE::INPUT_DEVICE::kFlatTotal] = { 0xFF, 0xFF, 0xFF };
    inline static uint32_t blockKeyMouse{ 0xFF };
    inline static uint32_t blockKeyKeyboard{ 0xFF };
    inline static uint32_t blockKeyGamePad{ 0xFF };
    int                    maxFrameCheck = 6;
    static inline uint32_t               dualBlockKey;
    static inline std::string sColorCodeStaminaPenalty;
    static inline std::string sColorCodeStaminaFlash;
    static inline std::string sColorCodeStaminaPhantom;
    static inline uint32_t uColorCodeStamBar = 0x7d7e7d;
    static inline uint32_t uColorCodeStamFlash = 0xd72a2a;
    static inline uint32_t uColorCodePhantom = 0xb30d10;
    static inline float dmgModifierMaxEnemy = 1.5f;
    static inline float dmgModifierMidEnemy = 1.3f;
    static inline float dmgModifierMinEnemy = 1.15f;
    static inline bool TrueHudAPI_Obtained;
    static void LogForm(std::string name, RE::TESForm* a_form) {
        dlog("found {} it is {}", name, a_form->GetName());
    }
    // tests
    RE::BGSPerk* dummyPerkDodge;

    static RE::FormID ParseFormID(const std::string& str);

    std::string FileName;
};
