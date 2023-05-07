//  Copyright (c) 2023 FlyByWire Simulations
//  SPDX-License-Identifier: GPL-3.0

import {
    AtisType,
    AtsuStatusCodes,
    CpdlcMessage,
    DclMessage,
    FmgcDataBusTypes,
    FreetextMessage,
    OclMessage,
    WeatherMessage,
    RmpDataBusTypes,
    Conversion,
    FlightPlanMessage,
    NotamMessage,
} from '@datalink/common';
import { Arinc429Word } from '@shared/arinc429';
import { FmgcFlightPhase } from '@shared/flightphase';
import { EventBus, EventSubscriber, Publisher } from '@microsoft/msfs-sdk';
import { AtcAocRouterMessages, DatalinkRouterMessages } from './databus';

export type RouterDigitalInputCallbacks = {
    sendFreetextMessage: (message: FreetextMessage, force: boolean) => Promise<AtsuStatusCodes>;
    sendCpdlcMessage: (message: CpdlcMessage, force: boolean) => Promise<AtsuStatusCodes>;
    sendDclMessage: (message: DclMessage, force: boolean) => Promise<AtsuStatusCodes>;
    sendOclMessage: (message: OclMessage, force: boolean) => Promise<AtsuStatusCodes>;
    requestFlightPlan: (requestSent: () => void) => Promise<[AtsuStatusCodes, FlightPlanMessage]>;
    requestNotams: (requestSent: () => void) => Promise<[AtsuStatusCodes, NotamMessage[]]>;
    requestAtis: (icao: string, type: AtisType, requestSent: () => void) => Promise<[AtsuStatusCodes, WeatherMessage]>;
    requestWeather: (icaos: string[], metar: boolean, requestSent: () => void) => Promise<[AtsuStatusCodes, WeatherMessage]>;
    connect: (callsign: string) => Promise<AtsuStatusCodes>;
    disconnect: () => Promise<AtsuStatusCodes>;
    stationAvailable: (callsign: string) => Promise<AtsuStatusCodes>;
}

export class DigitalInputs {
    private subscriber: EventSubscriber<AtcAocRouterMessages & FmgcDataBusTypes & DatalinkRouterMessages & RmpDataBusTypes> = null;

    private publisher: Publisher<AtcAocRouterMessages & DatalinkRouterMessages>;

    private poweredUp: boolean = false;

    private callbacks: RouterDigitalInputCallbacks = {
        sendFreetextMessage: null,
        sendCpdlcMessage: null,
        sendDclMessage: null,
        sendOclMessage: null,
        requestFlightPlan: null,
        requestNotams: null,
        requestAtis: null,
        requestWeather: null,
        connect: null,
        disconnect: null,
        stationAvailable: null,
    };

    public FlightPhase: FmgcFlightPhase = FmgcFlightPhase.Preflight;

    public Vhf3Powered: boolean = false;

    public Vhf3DataMode: boolean = false;

    private resetData(): void {
        this.FlightPhase = FmgcFlightPhase.Preflight;
        this.Vhf3Powered = false;
        this.Vhf3DataMode = false;
    }

    constructor(private readonly bus: EventBus, private readonly synchronizedAtc: boolean, private readonly synchronizedAoc: boolean) {
        this.resetData();
    }

    public initialize(): void {
        this.subscriber = this.bus.getSubscriber<AtcAocRouterMessages & FmgcDataBusTypes & DatalinkRouterMessages & RmpDataBusTypes>();
        this.publisher = this.bus.getPublisher<AtcAocRouterMessages & DatalinkRouterMessages>();
    }

    public connectedCallback(): void {
        this.subscriber.on('routerSendFreetextMessage').handle(async (request) => {
            if (this.callbacks.sendFreetextMessage !== null) {
                this.callbacks.sendFreetextMessage(Conversion.messageDataToMessage(request.message) as FreetextMessage, request.force).then((status) => {
                    this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status }, this.synchronizedAoc, false);
                });
            } else {
                this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status: AtsuStatusCodes.ComFailed }, this.synchronizedAoc, false);
            }
        });
        this.subscriber.on('routerSendCpdlcMessage').handle(async (request) => {
            if (this.callbacks.sendCpdlcMessage !== null) {
                this.callbacks.sendCpdlcMessage(Conversion.messageDataToMessage(request.message) as CpdlcMessage, request.force).then((status) => {
                    this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status }, this.synchronizedAtc, false);
                });
            } else {
                this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status: AtsuStatusCodes.ComFailed }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerSendDclMessage').handle(async (request) => {
            if (this.callbacks.sendDclMessage !== null) {
                this.callbacks.sendDclMessage(Conversion.messageDataToMessage(request.message) as DclMessage, request.force).then((status) => {
                    this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status }, this.synchronizedAtc, false);
                });
            } else {
                this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status: AtsuStatusCodes.ComFailed }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerSendOclMessage').handle(async (request) => {
            if (this.callbacks.sendOclMessage !== null) {
                this.callbacks.sendOclMessage(Conversion.messageDataToMessage(request.message) as OclMessage, request.force).then((status) => {
                    this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status }, this.synchronizedAtc, false);
                });
            } else {
                this.publisher.pub('routerSendMessageResponse', { requestId: request.requestId, status: AtsuStatusCodes.ComFailed }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerRequestFlightplan').handle(async (request) => {
            if (this.callbacks.requestFlightPlan !== null) {
                this.callbacks.requestFlightPlan(() => this.publisher.pub('routerRequestSent', request.requestId, this.synchronizedAoc, false)).then((response) => {
                    this.publisher.pub('routerReceivedFlightplan', { requestId: request.requestId, response }, this.synchronizedAoc, false);
                });
            } else {
                this.publisher.pub('routerReceivedFlightplan', { requestId: request.requestId, response: [AtsuStatusCodes.ComFailed, null] }, this.synchronizedAoc, false);
            }
        });
        this.subscriber.on('routerRequestNotams').handle(async (request) => {
            if (this.callbacks.requestNotams !== null) {
                this.callbacks.requestNotams(() => this.publisher.pub('routerRequestSent', request.requestId, this.synchronizedAoc, false)).then((response) => {
                    this.publisher.pub('routerReceivedNotams', { requestId: request.requestId, response }, this.synchronizedAoc, false);
                });
            } else {
                this.publisher.pub('routerReceivedNotams', { requestId: request.requestId, response: [AtsuStatusCodes.ComFailed, null] }, this.synchronizedAoc, false);
            }
        });
        this.subscriber.on('routerRequestAtis').handle(async (request) => {
            if (this.callbacks.requestAtis !== null) {
                const synchronized = this.synchronizedAoc || this.synchronizedAtc;
                this.callbacks.requestAtis(request.icao, request.type, () => this.publisher.pub('routerRequestSent', request.requestId, synchronized, false)).then((response) => {
                    this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response }, synchronized, false);
                });
            } else {
                this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response: [AtsuStatusCodes.ComFailed, null] }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerRequestMetar').handle(async (request) => {
            if (this.callbacks.requestAtis !== null) {
                const synchronized = this.synchronizedAoc || this.synchronizedAtc;
                this.callbacks.requestWeather(request.icaos, true, () => this.publisher.pub('routerRequestSent', request.requestId, synchronized, false)).then((response) => {
                    this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response }, synchronized, false);
                });
            } else {
                this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response: [AtsuStatusCodes.ComFailed, null] }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerRequestTaf').handle(async (request) => {
            if (this.callbacks.requestAtis !== null) {
                const synchronized = this.synchronizedAoc || this.synchronizedAtc;
                this.callbacks.requestWeather(request.icaos, false, () => this.publisher.pub('routerRequestSent', request.requestId, synchronized, false)).then((response) => {
                    this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response }, synchronized, false);
                });
            } else {
                this.publisher.pub('routerReceivedWeather', { requestId: request.requestId, response: [AtsuStatusCodes.ComFailed, null] }, this.synchronizedAtc, false);
            }
        });
        this.subscriber.on('routerConnect').handle(async (data) => {
            if (this.callbacks.connect !== null) {
                this.callbacks.connect(data.callsign).then((code) => {
                    this.publisher.pub('routerManagementResponse', { requestId: data.requestId, status: code }, true, false);
                });
            } else {
                this.publisher.pub('routerManagementResponse', { requestId: data.requestId, status: AtsuStatusCodes.ComFailed }, true, false);
            }
        });
        this.subscriber.on('routerDisconnect').handle(async (data) => {
            if (this.callbacks.disconnect !== null) {
                this.callbacks.disconnect().then((code) => {
                    this.publisher.pub('routerManagementResponse', { requestId: data, status: code }, true, false);
                });
            } else {
                this.publisher.pub('routerManagementResponse', { requestId: data, status: AtsuStatusCodes.ComFailed }, true, false);
            }
        });
        this.subscriber.on('routerRequestStationAvailable').handle(async (data) => {
            if (this.callbacks.stationAvailable !== null) {
                this.callbacks.stationAvailable(data.callsign).then((code) => {
                    this.publisher.pub('routerManagementResponse', { requestId: data.requestId, status: code }, true, false);
                });
            } else {
                this.publisher.pub('routerManagementResponse', { requestId: data.requestId, status: AtsuStatusCodes.ComFailed }, true, false);
            }
        });
        this.subscriber.on('flightPhase').handle((phase: Arinc429Word) => {
            if (this.poweredUp) {
                if (phase.isNormalOperation()) {
                    this.FlightPhase = phase.value as FmgcFlightPhase;
                } else {
                    this.FlightPhase = FmgcFlightPhase.Preflight;
                }
            }
        });
        this.subscriber.on('vhf3Powered').handle((powered: boolean) => this.Vhf3Powered = powered);
        this.subscriber.on('vhf3DataMode').handle((dataMode: boolean) => this.Vhf3DataMode = dataMode);
    }

    public powerUp(): void {
        this.poweredUp = true;
    }

    public powerDown(): void {
        this.poweredUp = false;
        this.resetData();
    }

    public addDataCallback<K extends keyof RouterDigitalInputCallbacks>(event: K, callback: RouterDigitalInputCallbacks[K]): void {
        this.callbacks[event] = callback;
    }
}
