// Copyright (c) 2022 FlyByWire Simulations
// SPDX-License-Identifier: GPL-3.0

#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <MSFS/Legacy/gauges.h>
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_Render.h>

#include "MsfsHandler.h"

#ifdef EXAMPLES
#include "Example/ExampleModule.h"
#endif

#include "AircraftPresets/AircraftPresetProcedures_A380X.h"
#include "AircraftPresets/AircraftPresets.h"
#include "LightingPresets/LightingPresets_A380X.h"
#include "Pushback/Pushback.h"

// FIXME: Using the A32NX_ prefix is a workaround until the A380X_ prefix is used
//  in the A380X EFB
MsfsHandler msfsHandler("Gauge_Extra_Backend", "A32NX_");
#ifdef EXAMPLES
ExampleModule exampleModule(msfsHandler);
#endif

// ADD ADDITIONAL MODULES HERE
// This is the only place these have to be added - everything else is handled automatically
LightingPresets_A380X lightingPresets(msfsHandler);
Pushback pushback(msfsHandler);
AircraftPresets aircraftPresets(msfsHandler, AircraftPresetProcedures_A380X::aircraftProcedureDefinition);

/**
 * Gauge Callback
 * There can by multiple gauges in a single wasm module. Just add another gauge callback function
 * and register it in the panel.cfg file.
 *
 * Avoid putting any logic in the gauge callback function. Instead, create a new class and put
 * the logic there.
 *
 * @see
 * https://docs.flightsimulator.com/html/Content_Configuration/SimObjects/Aircraft_SimO/Instruments/C_C++_Gauges.htm?rhhlterm=_gauge_callback&rhsearch=_gauge_callback
 */
extern "C" {
[[maybe_unused]] MSFS_CALLBACK bool Gauge_Extra_Backend_gauge_callback([[maybe_unused]] FsContext ctx, int svcId, void* pData) {
  switch (svcId) {
    case PANEL_SERVICE_PRE_INSTALL: {
      return msfsHandler.initialize();
    }
    case PANEL_SERVICE_PRE_DRAW: {
      return msfsHandler.update(static_cast<sGaugeDrawData*>(pData));
    }
    case PANEL_SERVICE_PRE_KILL: {
      return msfsHandler.shutdown();
    }
    default:
      break;
  }
  return false;
}
}
