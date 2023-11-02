// Copyright (c) 2023 FlyByWire Simulations
// SPDX-License-Identifier: GPL-3.0

#include "LightingPresets.h"
#include "ScopedTimer.hpp"

bool LightingPresets::update([[maybe_unused]] sGaugeDrawData* pData) {
  if (!_isInitialized) {
    LOG_ERROR("LightingPresets_A32NX::update() - not initialized");
    return false;
  }

  // only run when aircraft is powered
  if (!msfsHandler.getAircraftIsReadyVar() || !elecAC1Powered->getAsBool()) {
    return true;
  }

  // load becomes priority in case both vars are set.
  if (loadLightingPresetRequest->getAsBool()) {
    const INT64 presetRequest = loadLightingPresetRequest->getAsInt64();
    if (loadLightingPreset(presetRequest)) {
      readIniFile = true;
      loadLightingPresetRequest->setAsInt64(0);
      LOG_INFO("LightingPresets_A32NX: Lighting Preset: " + std::to_string(presetRequest) + " successfully loaded.");
    }
  } else if (saveLightingPresetRequest->getAsBool()) {
    saveLightingPreset(saveLightingPresetRequest->getAsInt64());
    saveLightingPresetRequest->setAsInt64(0);
  }

  return true;
}

bool LightingPresets::shutdown() {
  _isInitialized = false;
  LOG_INFO("LightingPresets::shutdown()");
  return true;
}

bool LightingPresets::loadLightingPreset(INT64 loadPresetRequest) {
  // throttle the load process so animation can keep up
  if (msfsHandler.getTickCounter() % 2 != 0) return false;

  // Read current values to be able to calculate intermediate values which are then applied to the aircraft
  // Once the intermediate values are identical to the target values then the load is finished
  readFromAircraft();
  if (readFromStore(loadPresetRequest)) {
    bool finished = calculateIntermediateValues();
    applyToAircraft();
    return finished;
  }
  LOG_WARN("LightingPresets_A32NX: Loading Lighting Preset: " + std::to_string(loadPresetRequest) + " failed.");
  return true;
}

void LightingPresets::saveLightingPreset(INT64 savePresetRequest) {
  std::cout << "LightingPresets_A32NX: Save to Lighting Preset: " << savePresetRequest << std::endl;
  readFromAircraft();
  if (saveToStore(savePresetRequest)) {
    LOG_INFO("LightingPresets_A32NX: Lighting Preset: " + std::to_string(savePresetRequest) + " successfully saved.");
    return;
  }
  LOG_WARN("LightingPresets_A32NX: Saving Lighting Preset: " + std::to_string(savePresetRequest) + " failed.");
}

bool LightingPresets::readFromStore(INT64 presetNr) {
  // only read ini file from disk if we load a new preset
  if (readIniFile) {
    if (!iniFile.read(ini)) {
      LOG_ERROR("LightingPresets_A32NX: Could not read ini file");
      return false;
    };
    readIniFile = false;
  }
  loadFromIni(ini, "preset " + std::to_string(presetNr));
  return true;
}

bool LightingPresets::saveToStore(INT64 presetNr) {
  // add/update iniSectionName
  const std::string iniSectionName = "preset " + std::to_string(presetNr);
  saveToIni(ini, iniSectionName);
  return iniFile.write(ini, true);
}

std::shared_ptr<AircraftVariable> LightingPresets::getLightPotentiometerVar(int index) const {
  return dataManager->make_aircraft_var("LIGHT POTENTIOMETER", index, "", lightPotentiometerSetEvent, UNITS.Percent, false, false, 0.0, 0);
}

FLOAT64 LightingPresets::iniGetOrDefault(const mINI::INIStructure& ini,
                                         const std::string& section,
                                         const std::string& key,
                                         const double defaultValue) {
  if (ini.get(section).has(key)) {
    // As MSFS wasm does not support exceptions (try/catch) we can't use
    // std::stof here. Workaround with std::stringstreams.
    std::stringstream input(ini.get(section).get(key));
    double value = defaultValue;
    if (input >> value) {
      return value;
    } else {
      LOG_WARN("LightingPresets: reading ini value for [" + section + "] " + key + " = " + ini.get(section).get(key) + " failed.");
    }
  }
  return defaultValue;
}

FLOAT64 LightingPresets::convergeValue(FLOAT64 momentary, FLOAT64 target) {
  if (momentary < target) {
    momentary += STEP_SIZE;
    if (momentary > target) {
      momentary = target;
    }
  } else if (momentary > target) {
    momentary -= STEP_SIZE;
    if (momentary < target) {
      momentary = target;
    }
  }
  return momentary;
}
