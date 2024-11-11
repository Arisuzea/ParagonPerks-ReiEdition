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

    FileName = "ParagonPerks.esp";

    if (debug_logging) {
        spdlog::get("Global")->set_level(spdlog::level::level_enum::debug);
        logger::debug("Debug logging enabled");
    };
    logger::info("... finished");
}

void Settings::LoadMCMSettings()
{
    logger::info("loading MCM settings...");

    constexpr auto defaultSettingsPath = R"(.\Data\MCM\Config\ParagonPerks\settings.ini)";
    constexpr auto mcmPath = R"(.\Data\MCM\Settings\ParagonPerks.ini)";

    const auto readMCM = [&](std::filesystem::path path) {
        CSimpleIniA mcm;
        mcm.SetUnicode();
        mcm.LoadFile(path.string().c_str());

        ReadColorStringSetting(mcm, "Colors", "sColorCodeStaminaPenalty", uColorCodeStamBar);        
        ReadColorStringSetting(mcm, "Colors", "sColorCodeStaminaFlash", uColorCodeStamFlash);
        ReadColorStringSetting(mcm, "Colors", "sColorCodeStaminaPhantom", uColorCodePhantom);
        dualBlockKey = std::stoi(mcm.GetValue("General", "iDualBlockKey", "48"));
        dlog("dual block key is {}", dualBlockKey);
        };
    logger::info("Reading MCM .ini...");

    readMCM(defaultSettingsPath); // read the default ini first
    readMCM(mcmPath);

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

void Settings::GetIngameData() noexcept // hard coded FormIDs to keep the ini file simpler for users
{
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
    const int magic_parry_perk = 0x58f69;
    const int arrow_parry_perk = 0x58f68;
    //Not implemented (yet?)
 /*   const int multi_shot_perk = 0x0;
    const int multi_shot_cd_spell = 0x0;
    const int multi_shot_cd_effect = 0x0;
    const int arrow_rain_perk = 0x0;
    const int arrow_rain_cd_spell = 0x0;
    const int arrow_rain_cd_effect = 0x0; */
    const int stam_pen_effect = 0x6D;
    const int npc_stam_pen_effect = 0xCD8;
    const int pit_fighter_perk = 0xC5;

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
    PitFighterPerk = dataHandler->LookupForm(pit_fighter_perk, FileName)->As<RE::BGSPerk>();
    dummyPerkDodge = dataHandler->LookupForm(DodgePerk, FileName)->As<RE::BGSPerk>();
    MagicParryPerk = dataHandler->LookupForm(magic_parry_perk, "Skyrim.esm")->As<RE::BGSPerk>();
    ArrowParryPerk = dataHandler->LookupForm(arrow_parry_perk, "Skyrim.esm")->As<RE::BGSPerk>();

    // Effects:
    MAG_ParryWindowEffect = dataHandler->LookupForm(ParryWindowEffect, FileName)->As<RE::EffectSetting>();
    StaminaPenaltyEffect = dataHandler->LookupForm(stam_pen_effect, FileName)->As<RE::EffectSetting>();
    StaminaPenEffectNPC = dataHandler->LookupForm(npc_stam_pen_effect, FileName)->As<RE::EffectSetting>();

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

    // form logging
    if (debug_logging) {
        LogForm("Stamina Cost Global", StaminaCostGlobal);
        LogForm("NPCStaminaCostGlobal", NPCStaminaCostGlobal);
        LogForm("DualBlockKey", DualBlockKey);
        LogForm("BashStaminaPerk", BashStaminaPerk);
        LogForm("BlockStaminaPerk", BlockStaminaPerk);
        LogForm("PitFighterPerk", PitFighterPerk);
        LogForm("dummyPerkDodge", dummyPerkDodge);
        LogForm("MagicParryPerk", MagicParryPerk);
        LogForm("ArrowParryPerk", ArrowParryPerk);
        LogForm("MAG_ParryWindowEffect", MAG_ParryWindowEffect);
        LogForm("StaminaPenaltyEffect", StaminaPenaltyEffect);
        LogForm("StaminaPenEffectNPC", StaminaPenEffectNPC);
        LogForm("IsBlockingSpell", IsBlockingSpell);
        LogForm("PowerAttackStopSpell", PowerAttackStopSpell);
        LogForm("jumpSpell", jumpSpell);
        LogForm("IsAttackingSpell", IsAttackingSpell);
        LogForm("IsSneakingSpell", IsSneakingSpell);
        LogForm("IsSprintingSpell", IsSprintingSpell);
        LogForm("MountSprintingSpell", MountSprintingSpell);
        LogForm("BowStaminaSpell", BowStaminaSpell);
        LogForm("XbowStaminaSpell", XbowStaminaSpell);
        LogForm("IsCastingSpell", IsCastingSpell);
        LogForm("MAGParryControllerSpell", MAGParryControllerSpell);
        LogForm("MAGParryStaggerSpell", MAGParryStaggerSpell);
        LogForm("APOParryBuffSPell", APOParryBuffSPell);
        LogForm("MAGCrossbowStaminaDrainSpell", MAGCrossbowStaminaDrainSpell);
        LogForm("DodgeRuneSpell", DodgeRuneSpell);
        LogForm("APOSparksShieldFlash", APOSparksShieldFlash);
        LogForm("APOSparksFlash", APOSparksFlash);
        LogForm("APOSparksPhysics", APOSparksPhysics);
        LogForm("APOSparks", APOSparks);
    }
    
    logger::debug("ingame forms loaded");
}



void Settings::LoadForms() 
{    
    GetIngameData();
    SetGlobalsAndGameSettings();

    auto gameSettings = RE::GameSettingCollection::GetSingleton();
    blockAngleSetting = gameSettings->GetSetting("fCombatHitConeAngle")->GetFloat();

    dualBlockKey = (std::uint32_t)DualBlockKey->value;

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
        return;
    }
    else {
        logger::info("Setting max armor rating to 75");
        maxRatingSetting->data.f = 75.0f;
        return;
    } 
}
// TrueHUD 
void Settings::ReadColorStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
    constexpr std::string_view prefix1 = "0x";
    constexpr std::string_view prefix2 = "#";
    constexpr std::string_view cset = "0123456789ABCDEFabcdef";
    const char* value = a_ini.GetValue(a_sectionName, a_settingName);
    if (value) {
        std::string_view str = value;
        if (str.contains(value)) {
        }

        if (str.starts_with(prefix1)) {
            str.remove_prefix(prefix1.size());
        }
        if (str.starts_with(prefix2)) {
            str.remove_prefix(prefix2.size());
        }
        bool bMatches = std::strspn(str.data(), cset.data()) == str.size();

        if (bMatches) {
            a_setting = std::stoi(str.data(), 0, 16);
        }
        else {
            const auto skyrimVM = RE::SkyrimVM::GetSingleton();
            auto vm = skyrimVM ? skyrimVM->impl : nullptr;
            if (vm) {
                RE::BSFixedString modName{ "ParagonPerks" };
                std::string settingStr = a_settingName;
                settingStr.append(":");
                settingStr.append(a_sectionName);
                RE::BSFixedString setting = settingStr;
                std::string settingValue = "0xFFFFFF";
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                auto vmargs = RE::MakeFunctionArguments(std::move(modName), std::move(setting), std::move(settingValue));
                vm->DispatchStaticCall("MCM", "SetModSettingString", vmargs, callback);
                delete vmargs;
            }
        }
    }
}

void Settings::logFormLoad(std::string form_name, RE::TESForm* a_form)
{
    logger::debug("{} loaded it is: {}", form_name, a_form->GetName());
}


