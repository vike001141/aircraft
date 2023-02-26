// Copyright (c) 2022 FlyByWire Simulations
// SPDX-License-Identifier: GPL-3.0

#ifndef FLYBYWIRE_SIMCONNECTEXCEPTIONSTRINGS_H
#define FLYBYWIRE_SIMCONNECTEXCEPTIONSTRINGS_H

#include <SimConnect.h>
#include <string>

class SimconnectExceptionStrings {
 public:
  static std::string getSimConnectExceptionString(SIMCONNECT_EXCEPTION exception) {
    switch (exception) {
      case SIMCONNECT_EXCEPTION_NONE:
        return "NONE";
      case SIMCONNECT_EXCEPTION_ERROR:
        return "ERROR";
      case SIMCONNECT_EXCEPTION_SIZE_MISMATCH:
        return "SIZE_MISMATCH";
      case SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID:
        return "UNRECOGNIZED_ID";
      case SIMCONNECT_EXCEPTION_UNOPENED:
        return "UNOPENED";
      case SIMCONNECT_EXCEPTION_VERSION_MISMATCH:
        return "VERSION_MISMATCH";
      case SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS:
        return "TOO_MANY_GROUPS";
      case SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED:
        return "NAME_UNRECOGNIZED";
      case SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES:
        return "TOO_MANY_EVENT_NAMES";
      case SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE:
        return "EVENT_ID_DUPLICATE";
      case SIMCONNECT_EXCEPTION_TOO_MANY_MAPS:
        return "TOO_MANY_MAPS";
      case SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS:
        return "TOO_MANY_OBJECTS";
      case SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS:
        return "TOO_MANY_REQUESTS";
      case SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT:
        return "WEATHER_INVALID_PORT";
      case SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR:
        return "WEATHER_INVALID_METAR";
      case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION:
        return "WEATHER_UNABLE_TO_GET_OBSERVATION";
      case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION:
        return "WEATHER_UNABLE_TO_CREATE_STATION";
      case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION:
        return "WEATHER_UNABLE_TO_REMOVE_STATION";
      case SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE:
        return "INVALID_DATA_TYPE";
      case SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE:
        return "INVALID_DATA_SIZE";
      case SIMCONNECT_EXCEPTION_DATA_ERROR:
        return "DATA_ERROR";
      case SIMCONNECT_EXCEPTION_INVALID_ARRAY:
        return "INVALID_ARRAY";
      case SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED:
        return "CREATE_OBJECT_FAILED";
      case SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED:
        return "LOAD_FLIGHTPLAN_FAILED";
      case SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OBJECT_TYPE:
        return "OPERATION_INVALID_FOR_OBJECT_TYPE";
      case SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION:
        return "ILLEGAL_OPERATION";
      case SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED:
        return "ALREADY_SUBSCRIBED";
      case SIMCONNECT_EXCEPTION_INVALID_ENUM:
        return "INVALID_ENUM";
      case SIMCONNECT_EXCEPTION_DEFINITION_ERROR:
        return "DEFINITION_ERROR";
      case SIMCONNECT_EXCEPTION_DUPLICATE_ID:
        return "DUPLICATE_ID";
      case SIMCONNECT_EXCEPTION_DATUM_ID:
        return "DATUM_ID";
      case SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS:
        return "OUT_OF_BOUNDS";
      case SIMCONNECT_EXCEPTION_ALREADY_CREATED:
        return "ALREADY_CREATED";
      case SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE:
        return "OBJECT_OUTSIDE_REALITY_BUBBLE";
      case SIMCONNECT_EXCEPTION_OBJECT_CONTAINER:
        return "OBJECT_CONTAINER";
      case SIMCONNECT_EXCEPTION_OBJECT_AI:
        return "OBJECT_AI";
      case SIMCONNECT_EXCEPTION_OBJECT_ATC:
        return "OBJECT_ATC";
      case SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE:
        return "OBJECT_SCHEDULE";
      default:
        return "UNKNOWN";
    }
  };
};

#endif  // FLYBYWIRE_SIMCONNECTEXCEPTIONSTRINGS_H
