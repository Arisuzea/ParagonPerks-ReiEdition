#include "Settings.h"
#include "Cache.h"
#include "Conditions.h"
#include <SimpleIni.h>
#include <sstream>

Settings* Settings::GetSingleton()
{
    static Settings settings;
    return &settings;
}

void Settings::LoadSettings()
{
    logger::info("loading settings...");
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\paragon-perks.ini)");

    enableSneakStaminaCost = ini.GetBoolValue("", "bEnableSneakStaminaCost", true);
    zeroAllWeapStagger     = ini.GetBoolValue("", "bZeroAllWeaponStagger", true);
    armorScalingEnabled    = ini.GetBoolValue("", "bArmorRatingScalingEnabled", true);
    debug_logging          = ini.GetBoolValue("", "Debug");

    surroundingActorsRange = (float)ini.GetDoubleValue("", "fRangeActors", 16.0);
    auto bonusXP           = (float)ini.GetDoubleValue("", "fBonusXPPerLevel", 0.15);
    auto baseXP            = (float)ini.GetDoubleValue("", "fBaseXPHerHit", 3.0);

    (bonusXP < 0.0 || bonusXP > 100.0) ? BonusXPPerLevel = 0.15f : BonusXPPerLevel = bonusXP;
    baseXP < 0.0 ? BaseXP = 3.0f : BaseXP = baseXP;

    FileName = "ValorPerks.esp";

    if (debug_logging) {
        spdlog::get("Global")->set_level(spdlog::level::level_enum::debug);
        logger::debug("Debug logging enabled");
    };
    logger::info("... finished");
}

RE::FormID Settings::ParseFormID(const std::string& str)
{
    RE::FormID         result;
    std::istringstream ss{ str };
    ss >> std::hex >> result;
    return result;
}

void Settings::AdjustWeaponStaggerVals()
{
    if (zeroAllWeapStagger) {
        logger::info("Adjusting weapon stagger values");
        int16_t totalWeaps = 0;

        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler) {
            for (const auto& foundWeap : dataHandler->GetFormArray<RE::TESObjectWEAP>()) {
                if (foundWeap && !foundWeap->weaponData.flags.any(RE::TESObjectWEAP::Data::Flag::kNonPlayable)) {
                    foundWeap->weaponData.staggerValue = 0.0f;
                    totalWeaps++;
                }
            }
        }

        logger::info(FMT_STRING("Stagger values adjusted: {} weapons"), totalWeaps);
    }
}

void Settings::GetIngameData() // hard coded FormIDs to keep the ini file simpler for users
{
    /*CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\paragon-form-lookup.ini)");*/


    //forms for ini:
    const int stam_cost_global = 0x25C;
    const int npc_stam_cost = 0x25D;
    const int dual_block_key = 0x2BD;
    const int bashStamPerk = 0xADA510;
    const int blockStamPerk = 0xADA509;
    const int DodgePerk = 0x2E9;
    const int ParryWindowEffect = 0xD9E;
    const int isBlockSpell = 0xDAC;
    const int power_attack_stop= 0x228;
    const int jump_spell = 0x225;
    const int isAttackSpel = 0xDA9;
    const int isSneakSpel= 0xDB4;
    const int sprint_spel = 0xDB6;
    const int mount_sprint = 0xDB1;
    const int bow_stam_spel = 0xDAE;
    const int x_bow_stam_spel = 0xDB0;
    const int casting_spel = 0xDB5;
    const int parry_control_spel = 0xDB2;
    const int parry_stagger_spel = 0xDB3;
    const int parry_buff_spel= 0xAE;
    const int crossbow_stam_drain = 0xDAF ;
    const int dodge_spell = 0x2C8;
    const int shield_sparks= 0xAC;
    const int weapon_sparks= 0xAB;
    const int physic_sparks= 0xA9;
    const int normal_sparks= 0xAA;

    const int multi_shot_perk = 0x0;
    const int multi_shot_cd_spell = 0x0;
    const int multi_shot_cd_effect = 0x0;
    const int arrow_rain_perk = 0x0;
    const int arrow_rain_cd_spell = 0x0;
    const int arrow_rain_cd_effect = 0x0; 
    const int stam_pen_effect = 0x06D;

    auto dataHandler = RE::TESDataHandler::GetSingleton();

    //ArrowRainPerk = dataHandler->LookupForm(arrow_rain_perk, FileName)->As<RE::BGSPerk>();
    //ArrowRainCooldownSpell = dataHandler->LookupForm(arrow_rain_cd_spell, FileName)->As<RE::SpellItem>();
    //ArrowRainCooldownEffect = dataHandler->LookupForm(arrow_rain_cd_effect, FileName)->As<RE::EffectSetting>();

    //MultiShotPerk = dataHandler->LookupForm(multi_shot_perk, FileName)->As<RE::BGSPerk>();
    //MultiShotCooldownSpell = dataHandler->LookupForm(multi_shot_cd_spell, FileName)->As<RE::SpellItem>();
    //MultiShotCooldownEffect = dataHandler->LookupForm(multi_shot_cd_effect, FileName)->As<RE::EffectSetting>();

    // Globals:
    StaminaCostGlobal    = dataHandler->LookupForm(stam_cost_global, FileName)->As<RE::TESGlobal>();
    NPCStaminaCostGlobal = dataHandler->LookupForm(npc_stam_cost, FileName)->As<RE::TESGlobal>();
    DualBlockKey         = dataHandler->LookupForm(dual_block_key, FileName)->As<RE::TESGlobal>();
    // Perks:
    BashStaminaPerk  = dataHandler->LookupForm(bashStamPerk, "Update.esm")->As<RE::BGSPerk>();
    BlockStaminaPerk = dataHandler->LookupForm(blockStamPerk, "Update.esm")->As<RE::BGSPerk>();
    //dummyPerkDodge = dataHandler->LookupForm(0x111211, FileName)->As<RE::BGSPerk>(); 
    dummyPerkDodge = dataHandler->LookupForm(DodgePerk, FileName)->As<RE::BGSPerk>();
    // Effects:
    MAG_ParryWindowEffect = dataHandler->LookupForm(ParryWindowEffect, FileName)->As<RE::EffectSetting>();
    StaminaPenaltyEffect = dataHandler->LookupForm(stam_pen_effect, FileName)->As<RE::EffectSetting>();
    // Spells:
    IsBlockingSpell              = dataHandler->LookupForm(isBlockSpell, FileName)->As<RE::SpellItem>();
    PowerAttackStopSpell         = dataHandler->LookupForm(power_attack_stop, FileName)->As<RE::SpellItem>();
    jumpSpell                    = dataHandler->LookupForm(jump_spell, FileName)->As<RE::SpellItem>();
    IsAttackingSpell             = dataHandler->LookupForm(isAttackSpel, FileName)->As<RE::SpellItem>();
    IsSneakingSpell              = dataHandler->LookupForm(isSneakSpel, FileName)->As<RE::SpellItem>();
    IsSprintingSpell             = dataHandler->LookupForm(sprint_spel, FileName)->As<RE::SpellItem>();
    MountSprintingSpell          = dataHandler->LookupForm(mount_sprint, FileName)->As<RE::SpellItem>();
    BowStaminaSpell              = dataHandler->LookupForm(bow_stam_spel, FileName)->As<RE::SpellItem>();
    XbowStaminaSpell             = dataHandler->LookupForm(x_bow_stam_spel, FileName)->As<RE::SpellItem>();
    IsCastingSpell               = dataHandler->LookupForm(casting_spel, FileName)->As<RE::SpellItem>();
    MAGParryControllerSpell      = dataHandler->LookupForm(parry_control_spel, FileName)->As<RE::SpellItem>();
    MAGParryStaggerSpell         = dataHandler->LookupForm(parry_stagger_spel, FileName)->As<RE::SpellItem>();
    APOParryBuffSPell            = dataHandler->LookupForm(parry_buff_spel, FileName)->As<RE::SpellItem>();
    MAGCrossbowStaminaDrainSpell = dataHandler->LookupForm(crossbow_stam_drain, FileName)->As<RE::SpellItem>();
    DodgeRuneSpell = dataHandler->LookupForm(dodge_spell, FileName)->As<RE::SpellItem>();
    // Explosions:
    APOSparksShieldFlash = dataHandler->LookupForm(shield_sparks, FileName)->As<RE::BGSExplosion>();
    APOSparksFlash       = dataHandler->LookupForm(weapon_sparks, FileName)->As<RE::BGSExplosion>();
    APOSparksPhysics     = dataHandler->LookupForm(physic_sparks, FileName)->As<RE::BGSExplosion>();
    APOSparks            = dataHandler->LookupForm(normal_sparks, FileName)->As<RE::BGSExplosion>();
    
    // test stuff
    fireBolt = dataHandler->LookupForm(0x2dd29, "Skyrim.esm")->As<RE::SpellItem>();

    logger::debug("ingame forms loaded");
}



void Settings::LoadForms()
{
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    
    GetIngameData();
    SetGlobalsAndGameSettings();

    auto gameSettings = RE::GameSettingCollection::GetSingleton();
    blockAngleSetting = gameSettings->GetSetting("fCombatHitConeAngle")->GetFloat();

    dualBlockKey = DualBlockKey->value;

    logger::info("All Forms loaded");
}

void Settings::SetGlobalsAndGameSettings()
{
    // Set fMaxArmorRating game setting
    auto gameSettings     = RE::GameSettingCollection::GetSingleton();    
    auto maxRatingSetting = gameSettings->GetSetting("fMaxArmorRating");

    if (armorScalingEnabled) {
        logger::info("Setting max armor rating from {} to 90", maxRatingSetting->data.f);
        
        maxRatingSetting->data.f = 90.0f;
        logger::debug("donezo");
        return;
    }
    else {
        logger::info("Setting max armor rating to 75");
        maxRatingSetting->data.f = 75.0f;
        return;
    }
}
