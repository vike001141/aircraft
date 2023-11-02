// Copyright (c) 2022 FlyByWire Simulations
// SPDX-License-Identifier: GPL-3.0

#ifndef FLYBYWIRE_DATAMANAGER_H
#define FLYBYWIRE_DATAMANAGER_H

#include <vector>
#include <memory>
#include <map>
#include <string>

#include <MSFS/Legacy/gauges.h>
#include <MSFS/MSFS.h>
#include <SimConnect.h>

#include "logging.h"
#include "IDGenerator.h"
#include "Units.h"

#include "DataDefinitionVariable.hpp"
#include "ClientDataAreaVariable.hpp"
#include "ClientDataBufferedAreaVariable.hpp"

// Forward declarations
class MsfsHandler;
class CacheableVariable;
class NamedVariable;
class AircraftVariable;
class SimObjectBase;
struct DataDefinition;
class Event;

// convenience typedefs
typedef std::shared_ptr<CacheableVariable> CacheableVariablePtr;
typedef std::shared_ptr<NamedVariable> NamedVariablePtr;
typedef std::shared_ptr<AircraftVariable> AircraftVariablePtr;
typedef std::shared_ptr<SimObjectBase> SimObjectBasePtr;
typedef std::shared_ptr<Event> EventPtr;

// Used to identify a key event
typedef unsigned int KeyEventID;

// Used for callback registration to allow removal of callbacks
typedef uint64_t KeyEventCallbackID;

/**
 * Defines a callback function for a key event
 * @param number of parameters to use
 * @param parameters 0-4 to pass to the callback function
 */
typedef std::function<void(
  DWORD param0, DWORD param1, DWORD param2, DWORD param3, DWORD param4)> KeyEventCallbackFunction;

/**
 * DataManager is responsible for managing all variables and events.
 * It is used to register variables and events and to update them.
 * It de-duplicates variables and events and only creates one instance of each if multiple modules
 * use the same variable.
 *
 * It is still possible to use the SDK and Simconnect directly but it is recommended to use the
 * DataManager instead as the data manager is able to de-duplicate variables and events and automatically
 * update and write back variables from/to the sim.
 *
 * Currently variables do not use SIMCONNECT_PERIOD from the simconnect API but instead use a more
 * controlled on-demand update mechanism in this class' preUpdate method.
 */
class DataManager {
private:
  // A map of all registered variables.
  std::map<std::string, CacheableVariablePtr> variables{};

  // A map of all registered SimObjects.
  // Map over the request id to quickly find the SimObject.
  std::map<SIMCONNECT_DATA_REQUEST_ID, SimObjectBasePtr> simObjects{};

  // A map of all registered events.
  // Map over the event id to quickly find the event - make creating an event a bit less efficient.
  std::map<SIMCONNECT_CLIENT_EVENT_ID, EventPtr> events{};

  // Map of callback vectors to be called when a key event is triggered in the sim.
  std::map<KeyEventID , std::map<KeyEventCallbackID, KeyEventCallbackFunction>> keyEventCallbacks{};

  // Backreference to the MsfsHandler instance.
  MsfsHandler* msfsHandler;

  // Handle to the simconnect instance.
  HANDLE hSimConnect{};

  // Flag to indicate if the data manager is initialized.
  bool isInitialized = false;

  // Instances of an IDGenerator to generate unique IDs for variables and events.
  IDGenerator dataDefIDGen{};
  IDGenerator dataReqIDGen{};
  IDGenerator clientDataIDGen{};
  IDGenerator eventIDGen{};
  IDGenerator keyEventCallbackIDGen{};

public:

  /**
   * Creates an instance of the DataManager.
   */
  explicit DataManager(MsfsHandler* msfsHdl) : msfsHandler(msfsHdl) {}

  DataManager() = delete; // no default constructor
  DataManager(const DataManager &) = delete; // no copy constructor
  DataManager &operator=(const DataManager &) = delete; // no copy assignment
  ~DataManager() = default;

  /**
   * Initializes the data manager.
   * This method must be called before any other method of the data manager.
   * Usually called in the MsfsHandler initialization.
   * @param hdl Handle to the simconnect instance
   */
  bool initialize(HANDLE hdl);

  /**
   * Called by the MsfsHandler update() method.
   * Updates all variables marked for automatic reading.
   * Calls SimConnect_GetNextDispatch to retrieve all messages from the simconnect queue.
   * @param pData Pointer to the data structure of gauge pre-draw event
   * @return true if successful, false otherwise
   */
  bool preUpdate([[maybe_unused]] sGaugeDrawData* pData);

  /**
 * Called by the MsfsHandler update() method.
 * @param pData Pointer to the data structure of gauge pre-draw event
 * @return true if successful, false otherwise
 */
  bool update(sGaugeDrawData* pData) const;

  /**
 * Called by the MsfsHandler update() method.
 * Writes all variables marked for automatic writing back to the sim.
 * @param pData Pointer to the data structure of gauge pre-draw event
 * @return true if successful, false otherwise
 */
  bool postUpdate(sGaugeDrawData* pData);

  /**
   * Called by the MsfsHandler shutdown() method.
   * Can be used for any extra cleanup.
   * @return true if successful, false otherwise
   */
  bool shutdown();

  /**
   * Must be called to retrieve requested sim data.
   * It will loop until all requested data has been received for this tick.
   * Will be called at the end of preUpdate() whenever preUpdate() is called.
   * Request data by calling any of the DataDefinitions::request...() methods
   * on the data definition variable.
   */
  void getRequestedData();

  /**
   * Adds a callback function to be called when a key event is triggered in the sim.<br/>
   * OBS: The callback will be called even if the sim is paused.
   * @param keyEventId The ID of the key event to listen to.
   * @param callback
   * @return The ID of the callback required for removing a callback.
   * @see https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm
   * @see #define KEY_events in gauges.h.
   */
  [[nodiscard]]
  KeyEventCallbackID addKeyEventCallback(KeyEventID keyEventId, const KeyEventCallbackFunction &callback);

  /**
   * Removes a callback from a key event.
   * @param keyEventId The ID of the key event to remove the callback from.
   * @param callbackId The ID receive when adding the callback.
   */
  bool removeKeyEventCallback(KeyEventID keyEventId, KeyEventCallbackID callbackId);

  /**
   * Sends a key event to the sim.
   * @param keyEventId The ID of the key event to send.
   * @param param0
   * @param param1
   * @param param2
   * @param param3
   * @param param4
   * @return true if successful, false otherwise
   * @see https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm
   * @see #define KEY_events in gauges.h.
   */
  bool sendKeyEvent(KeyEventID keyEventId, DWORD param0, DWORD param1, DWORD param2, DWORD param3, DWORD param4);

  /**
   * TODO: Implement and document
   *
   * Is called by the MsfsHandler when a key event is triggered in the sim.
   * @param event
   * @param evdata0
   * @param evdata1
   * @param evdata2
   * @param evdata3
   * @param evdata4
   * @param userdata
   */
  [[maybe_unused]]
  void processKeyEvent(KeyEventID event, UINT32 evdata0, UINT32 evdata1, UINT32 evdata2,
                       UINT32 evdata3, UINT32 evdata4);

  /**
   * Creates a new named variable and adds it to the list of managed variables
   * @param varName Name of the variable in the sim
   * @param optional unit Unit of the variable (default=Number)
   * @param autoReading optional flag to indicate if the variable should be read automatically (default=false)
   * @param autoWriting optional flag to indicate if the variable should be written automatically (default=false)
   * @param maxAgeTime optional maximum age of the variable in seconds (default=0)
   * @param maxAgeTicks optional Maximum age of the variable in ticks (default=0)
   * @return A shared pointer to the variable
   * @see Units.h for available units
   */
  NamedVariablePtr make_named_var(
    const std::string varName,
    Unit unit = UNITS.Number,
    bool autoReading = false,
    bool autoWriting = false,
    FLOAT64 maxAgeTime = 0.0,
    UINT64 maxAgeTicks = 0);

  /**
   * Creates a new AircraftVariable and adds it to the list of managed variables.
   * To create a writable variable, use either the setterEventName or setterEvent parameter.
   * If both are set, the setterEvent will be used.
   * @param varName Name of the variable in the sim
   * @param index Index of the indexed variable in the sim (default=0)
   * @param setterEventName the name of the event to set the variable with an event or calculator code (default="")
   * @param setterEvent an instance of an event variable to set the variable with an event or calculator code (default=nullptr)
   * @param unit Unit of the variable (default=Number)
   * @param autoReading optional flag to indicate if the variable should be read automatically (default=false)
   * @param autoWriting optional flag to indicate if the variable should be written automatically (default=false)
   * @param maxAgeTime optional maximum age of the variable in seconds (default=0)
   * @param maxAgeTicks optional Maximum age of the variable in ticks (default=0)
   * @return A shared pointer to the variable
   * @see Units.h for available units
   */
  AircraftVariablePtr make_aircraft_var(
    const std::string varName,
    int index = 0,
    const std::string setterEventName = "",
    EventPtr setterEvent = nullptr,
    Unit unit = UNITS.Number,
    bool autoReading = false,
    bool autoWriting = false,
    FLOAT64 maxAgeTime = 0.0,
    UINT64 maxAgeTicks = 0);

  /**
   * Creates a new readonly non-indexed AircraftVariable and adds it to the list of managed variables.
   * This is a convenience method for make_aircraft_var() to create a variable that is read-only and
   * does not have an index.
   * @param varName Name of the variable in the sim
   * @param unit Unit of the variable (default=Number)
   * @param autoReading optional flag to indicate if the variable should be read automatically (default=false)
   * @param maxAgeTime optional maximum age of the variable in seconds (default=0)
   * @param maxAgeTicks optional Maximum age of the variable in ticks (default=0)
   * @return A shared pointer to the variable
   * @see Units.h for available units
   */
  AircraftVariablePtr make_simple_aircraft_var(
    const std::string varName,
    Unit unit = UNITS.Number,
    bool autoReading = false,
    FLOAT64 maxAgeTime = 0.0,
    UINT64 maxAgeTicks = 0);

  /**
   * Creates a new data definition variable and adds it to the list of managed variables.
   * @typename T Type of the data structure to use to store the data
   * @param name An arbitrary name for the data definition variable for debugging purposes
   * @param dataDefinitions A vector of data definitions for the data definition variable
   * @param autoReading optional flag to indicate if the variable should be read automatically (default=false)
   * @param autoWriting optional flag to indicate if the variable should be written automatically (default=false)
   * @param maxAgeTime optional maximum age of the variable in seconds (default=0)
   * @param maxAgeTicks optional Maximum age of the variable in ticks (default=0)
   * @return A shared pointer to the variable
   */
  template<typename T>
  std::shared_ptr<DataDefinitionVariable<T>> make_datadefinition_var(
    const std::string name,
    std::vector<DataDefinition>&dataDefinitions,
    bool autoReading = false,
    bool autoWriting = false, FLOAT64
    maxAgeTime = 0.0, UINT64
    maxAgeTicks = 0) {

    std::shared_ptr<DataDefinitionVariable<T>>
      var = std::make_shared<DataDefinitionVariable<T>> (
        hSimConnect,
        std::move(name),
        dataDefinitions,
        dataDefIDGen.getNextId(),
        dataReqIDGen.getNextId(),
        autoReading,
        autoWriting,
        maxAgeTime,
        maxAgeTicks);

    LOG_DEBUG("DataManager::make_datadefinition_var(): " + name);
    simObjects.insert({var->getRequestId(), var});
    return var;
  }

  /**
   * Creates a new client data area variable and adds it to the list of managed variables.
   * @typename T Type of the data structure to use to store the data
   * @param clientDataName String containing the client data area name. This is the name that another
   *                       client will use to specify the data area. The name is not case-sensitive.
   *                       If the name requested is already in use by another addon, a error will be
   *                       printed to the console.
   * @param readOnlyForOthers Specify if the data area can only be written to by this client (the
   *                          client creating the data area). By default other clients can write to
   *                          this data area.
   * @param autoReading optional flag to indicate if the variable should be read automatically (default=false)
   * @param autoWriting optional flag to indicate if the variable should be written automatically (default=false)
   * @param maxAgeTime optional maximum age of the variable in seconds (default=0)
   * @param maxAgeTicks optional Maximum age of the variable in ticks (default=0)
   * @return A shared pointer to the variable
   */
  template<typename T>
  std::shared_ptr<ClientDataAreaVariable<T>> make_clientdataarea_var(
    const std::string clientDataName,
    bool autoReading = false,
    bool autoWriting = false,
    FLOAT64 maxAgeTime = 0.0,
    UINT64 maxAgeTicks = 0) {

    std::shared_ptr <ClientDataAreaVariable<T>>
      var = std::make_shared<ClientDataAreaVariable<T>> (
        hSimConnect,
        std::move(clientDataName),
        clientDataIDGen.getNextId(),
        dataDefIDGen.getNextId(),
        dataReqIDGen.getNextId(),
        autoReading,
        autoWriting,
        maxAgeTime,
        maxAgeTicks);

    LOG_DEBUG("DataManager::make_datadefinition_var(): " + clientDataName);
    simObjects.insert({var->getRequestId(), var});
    return var;
  }

  template<typename T, std::size_t ChunkSize>
  std::shared_ptr<ClientDataBufferedAreaVariable<T, ChunkSize>> make_clientdatabufferedarea_var(
    const std::string clientDataName,
    bool autoReading = false,
    bool autoWriting = false,
    FLOAT64 maxAgeTime = 0.0,
    UINT64 maxAgeTicks = 0) {

    std::shared_ptr <ClientDataBufferedAreaVariable<T, ChunkSize>>
      var = std::make_shared<ClientDataBufferedAreaVariable<T, ChunkSize>> (
        hSimConnect,
        std::move(clientDataName),
        clientDataIDGen.getNextId(),
        dataDefIDGen.getNextId(),
        dataReqIDGen.getNextId(),
        autoReading,
        autoWriting,
        maxAgeTime,
        maxAgeTicks);

    LOG_DEBUG("DataManager::make_clientdataarea_buffered_var(): " + clientDataName);
    simObjects.insert({var->getRequestId(), var});
    return var;
  }


  /**
   * Creates a new event and adds it to the list of managed events.
   * Per default does not subscribe to the event. Use the subscribeToSim() method
   * to subscribeToSim to the event.
   * @param eventName Name of the event in the sim
   * @param maksEvent True indicates that the event will be masked by this client and will not be
   *                  transmitted to any more clients, possibly including Microsoft Flight Simulator
   *                  itself (if the priority of the client exceeds that of Flight Simulator).
   *                  False is the default.
   * @return A shared pointer to the event instance
   */
  EventPtr make_event(const std::string &eventName, bool maksEvent = false);

private:

  /**
   * This is called everytime we receive a message from the sim in getRequestedData().
   * @param pRecv
   * @param cbData
   *
   * @see https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/Structures_And_Enumerations/SIMCONNECT_RECV.htm
   */
  void processDispatchMessage(SIMCONNECT_RECV* pRecv, [[maybe_unused]] DWORD* cbData);

  /**
   * Callback for SimConnect_GetNextDispatch events in the MsfsHandler used for data definition
   * variable batches.
   * Must map data request IDs to know IDs of data definition variables and return true if the
   * requestId has been handled.
   * @param data Pointer to the data structure of gauge pre-draw event
   * @return true if request ID has been processed, false otherwise
   */
  void processSimObjectData(SIMCONNECT_RECV* data);

  /**
   * Called from processDispatchMessage() for SIMCONNECT_RECV_ID_EVENT messages.
   * @param pRecv the SIMCONNECT_RECV_EVENT structure
   */
  void processEvent(const SIMCONNECT_RECV_EVENT* pRecv);

  /**
   * Called from processDispatchMessage() for SIMCONNECT_RECV_EVENT_EX1 messages.
   * @param pRecv the SIMCONNECT_RECV_EVENT_EX1 structure
   */
  void processEvent(const SIMCONNECT_RECV_EVENT_EX1* pRecv);
};

#endif // FLYBYWIRE_DATAMANAGER_H
