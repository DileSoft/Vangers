import * as net from 'net';
import { EVENTS, GameType } from './constants.js';
import { XBuffer, XReader } from './utils.js';

// Protocol constants (mirrors multiplayer.h)
const AUXILIARY_EVENT = 0x80;
const ECHO_EVENT = 0x20;
const NID_VANGER = 9 << 16; // 0x90000

function getObjectType(id: number): number {
    return id & (63 << 16); // bits 16-21
}

interface StoredObject {
    id: number;
    clientId: number;
    time: number;
    x: number;
    y: number;
    body: Buffer;
}

interface Player {
    id: number;
    name: string;
    socket: net.Socket;
    world: number;
    x: number;
    y: number;
    status: number;
    gameId: number | null;
    identified: boolean;
    clientVersion: number;
    body: Buffer | null;
}

interface Game {
    id: number;
    name: string;
    type: GameType;
    players: Map<number, Player>;
    startTime: number;
    configured: boolean;
    serverData: Buffer | null; // raw 44-byte ServerData from SET_GAME_DATA
    objects: Map<number, StoredObject>; // object store for relay
}

class VangersServer {
    private players: Map<number, Player> = new Map();
    private games: Map<number, Game> = new Map();
    private nextPlayerId = 1;
    private readonly MIN_SERVER_VERSION = 3;
    private readonly MAX_SERVER_VERSION = 3;
    private serverStartTime = Date.now();

    constructor(private port: number) {}

    private getGlobalClock(): number {
        // C++: (round(SDL_GetTicks() * (256. / 1000)))
        const ticks = Date.now() - this.serverStartTime;
        return Math.floor(ticks * (256 / 1000));
    }

    start() {
        const server = net.createServer((socket: net.Socket) => {
            const playerId = this.nextPlayerId++;
            const player: Player = {
                id: playerId,
                name: `Player_${playerId}`,
                socket,
                world: 0,
                x: 0,
                y: 0,
                status: 0,
                gameId: null,
                identified: false,
                clientVersion: 0,
                body: null
            };

            this.players.set(playerId, player);
            console.log(`Player connected: ${player.name} (${socket.remoteAddress})`);

            socket.on('data', (data: Buffer) => this.handleData(player, data));
            socket.on('end', () => this.handleDisconnect(player));
            socket.on('error', (err: Error) => {
                console.error(`Socket error for ${player.name}:`, err);
                this.handleDisconnect(player);
            });
        });

        server.listen(this.port, () => {
            console.log(`Vangers TypeScript Server listening on port ${this.port}`);
        });
    }

    private handleData(player: Player, data: Buffer) {
        if (!player.identified) {
            this.handleIdentification(player, data);
            return;
        }

        // Suppress noisy per-frame hex dump (UPDATE/DELETE object streams)
        if (data[2] !== EVENTS.UPDATE_OBJECT && data[2] !== EVENTS.DELETE_OBJECT) {
            console.log(`[RECV] << ${data.toString('hex').toUpperCase().match(/.{1,2}/g)?.join(' ')}`);
        }        const reader = new XReader(data);
        while (!reader.isEOF) {
            if (data.length - reader.tell() < 2) break;
            const eventSize = reader.readInt16LE();
            const startOffset = reader.tell();
            
            if (data.length - reader.tell() < 1) break; 
            
            const rawCode = reader.readUInt8();
            // Strip ECHO_EVENT flag for non-auxiliary events (mirrors C++ server logic)
            const eventCode = (rawCode & AUXILIARY_EVENT) ? rawCode : (rawCode & ~ECHO_EVENT);
            const eventName = Object.keys(EVENTS).find(key => (EVENTS as any)[key] === eventCode) || `0x${eventCode.toString(16)}`;
            if (eventCode !== EVENTS.SERVER_TIME_QUERY && eventCode !== EVENTS.UPDATE_OBJECT && eventCode !== EVENTS.DELETE_OBJECT) {
                console.log(`[EVENT] RECV: ${eventName} (0x${eventCode.toString(16)}) [size=${eventSize}] from ${player.name}`);
            }

            switch (eventCode) {
                case EVENTS.CREATE_PERMANENT_OBJECT: {
                    if (player.gameId === null) break;
                    const game = this.games.get(player.gameId);
                    if (!game) break;
                    // Client sends: obj_ID(4), time(4), x(2), y(2), radius(2),
                    //   [y_half_size(2) if NID_VANGER], body
                    const objId = reader.readInt32LE();
                    const time = reader.readInt32LE();
                    const x = reader.readInt16LE();
                    const y = reader.readInt16LE();
                    reader.readInt16LE(); // radius - not stored/relayed
                    const isVanger = getObjectType(objId) === NID_VANGER;
                    if (isVanger) {
                        reader.readUInt16LE(); // y_half_size_of_screen - stripped from relay
                    }
                    const bodySize = isVanger ? eventSize - 17 : eventSize - 15;
                    const body = bodySize > 0 ? reader.readBytes(bodySize) : Buffer.alloc(0);
                    const obj: StoredObject = { id: objId, clientId: player.id, time, x, y, body };
                    game.objects.set(objId, obj);
                    this.relayObjectUpdate(player, game, obj);
                    break;
                }
                case EVENTS.UPDATE_OBJECT: {
                    if (player.gameId === null) break;
                    const game = this.games.get(player.gameId);
                    if (!game) break;
                    // Client sends: obj_ID(4), time(4), x(2), y(2),
                    //   [y_half_size(2) if NID_VANGER], body
                    const objId = reader.readInt32LE();
                    const time = reader.readInt32LE();
                    const x = reader.readInt16LE();
                    const y = reader.readInt16LE();
                    const isVanger = getObjectType(objId) === NID_VANGER;
                    if (isVanger) {
                        reader.readUInt16LE(); // y_half_size_of_screen - stripped from relay
                    }
                    const bodySize = isVanger ? eventSize - 15 : eventSize - 13;
                    const body = bodySize > 0 ? reader.readBytes(bodySize) : Buffer.alloc(0);
                    const existing = game.objects.get(objId);
                    if (existing) {
                        existing.clientId = player.id;
                        existing.time = time;
                        existing.x = x;
                        existing.y = y;
                        existing.body = body;
                        this.relayObjectUpdate(player, game, existing);
                    }
                    // If object not found, ignore (mirrors C++ SKIP_UPDATE_OBJECT)
                    break;
                }
                case EVENTS.DELETE_OBJECT: {
                    if (player.gameId === null) break;
                    const game = this.games.get(player.gameId);
                    if (!game) break;
                    const objId = reader.readInt32LE();
                    const time = reader.readInt32LE();
                    const bodySize = eventSize - 9; // 1(code) + 4(id) + 4(time)
                    const body = bodySize > 0 ? reader.readBytes(bodySize) : Buffer.alloc(0);
                    game.objects.delete(objId);
                    this.relayObjectDelete(player, game, objId, time, body);
                    break;
                }
                case EVENTS.GAMES_LIST_QUERY:
                    this.sendGamesList(player);
                    break;
                case EVENTS.REGISTER_NAME: {
                    const name = reader.readString();
                    const password = reader.readString();
                    player.name = name;
                    console.log(`[LOG] Player ${player.id} registered: name="${name}", pass="${password}"`);
                    this.broadcastName(player);
                    break;
                }
                case EVENTS.SERVER_TIME_QUERY:
                    this.sendServerTime(player);
                    break;
                case EVENTS.ATTACH_TO_GAME: {
                    const gameId = reader.readInt32LE(); 
                    console.log(`[LOG] ${player.name} requests attachment to gameId: ${gameId}`);
                    this.attachToGame(player, gameId);
                    break;
                }
                case EVENTS.TOTAL_PLAYERS_DATA_QUERY: {
                    this.sendTotalPlayersData(player);
                    break;
                }
                case EVENTS.SET_GAME_DATA: {
                    this.handleSetGameData(player, reader);
                    break;
                }
                case EVENTS.GET_GAME_DATA: {
                    this.sendGameData(player);
                    break;
                }
                case EVENTS.SET_PLAYER_DATA: {
                    if (player.gameId === null) break;
                    const game = this.games.get(player.gameId);
                    if (!game) break;
                    const SIZEOF_PLAYER_BODY = 52;
                    const bodyBytes = eventSize - 1; // eventSize includes the code byte
                    if (bodyBytes >= SIZEOF_PLAYER_BODY) {
                        player.body = reader.readBytes(SIZEOF_PLAYER_BODY);
                        console.log(`[LOG] ${player.name} SET_PLAYER_DATA: NetID=0x${player.body.readInt32LE(24).toString(16)}, world=${player.body[3]}, CarIndex=${player.body[12]}`);
                        const res = new XBuffer();
                        res.beginEvent(EVENTS.PLAYERS_DATA);
                        res.writeUInt8(player.id);
                        res.writeBytes(player.body);
                        for (const p of game.players.values()) {
                            if (p.id !== player.id) {
                                this.send(p, res, "PLAYERS_DATA");
                            }
                        }
                    }
                    break;
                }
                case EVENTS.LEAVE_WORLD: {
                    console.log(`[LOG] ${player.name} left world ${player.world}`);
                    player.world = 0;
                    break;
                }
                case EVENTS.DIRECT_SENDING: {
                    if (player.gameId === null) break;
                    const game = this.games.get(player.gameId);
                    if (!game) break;
                    const mask = reader.readUInt32LE();
                    const bodySize = eventSize - 5; // 1(code) + 4(mask)
                    const body = bodySize > 0 ? reader.readBytes(bodySize) : Buffer.alloc(0);
                    for (const p of game.players.values()) {
                        if (mask & (1 << (p.id - 1))) {
                            const res = new XBuffer();
                            res.beginEvent(EVENTS.DIRECT_RECEIVING);
                            res.writeUInt8(player.id);
                            res.writeBytes(body);
                            this.send(p, res, "DIRECT_RECEIVING");
                        }
                    }
                    break;
                }
                case EVENTS.SET_POSITION: {
                    player.x = reader.readInt16LE();
                    player.y = reader.readInt16LE();
                    const yHalfSize = reader.readInt16LE(); 
                    console.log(`[LOG] ${player.name} position: x=${player.x}, y=${player.y}, yHalf=${yHalfSize}`);
                    this.broadcastPosition(player);
                    break;
                }
                case EVENTS.SET_WORLD: {
                    const worldId = reader.readUInt8();
                    const worldYSize = reader.readInt16LE();
                    console.log(`[LOG] ${player.name} sets world: worldId=${worldId}, ySize=${worldYSize}`);
                    this.handleSetWorld(player, worldId, worldYSize);
                    break;
                }
                default:
                    console.log(`[WARN] Unhandled event 0x${eventCode.toString(16)}`);
            }

            // Always seek to the end of the event to handle potential partial reads
            const bytesRead = reader.tell() - startOffset;
            if (bytesRead < eventSize) {
                console.log(`[LOG] Skipping ${eventSize - bytesRead} unread bytes of event 0x${eventCode.toString(16)}`);
                for (let i = 0; i < eventSize - bytesRead; i++) {
                    if (!reader.isEOF) reader.readUInt8();
                }
            }
        }
    }

    private sendTotalPlayersData(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        const res = new XBuffer();
        res.beginEvent(EVENTS.TOTAL_LIST_OF_PLAYERS_DATA);
        res.writeUInt8(game.players.size);
        for (const p of game.players.values()) {
            res.writeUInt8(p.id);
            res.writeUInt8(p.status); // 0 = INITIAL
            res.writeUInt8(p.world);
            res.writeInt16LE(p.x);
            res.writeInt16LE(p.y);
            res.writeString(p.name);
            
            // PlayerBody size is 52 bytes for v3
            if (p.body) {
                res.writeBytes(p.body);
            } else {
                for (let i = 0; i < 52; i++) res.writeUInt8(0);
            }
        }
        this.send(player, res, "TOTAL_LIST_OF_PLAYERS_DATA");
    }

    private handleIdentification(player: Player, data: Buffer) {
        const REQUEST_STR = "Vivat Sicher, Rock'n'Roll forever!!!";
        const RESPONSE_STR = "Enter, my son, please...";
        
        const dataStr = data.toString('utf8');
        if (dataStr.startsWith(REQUEST_STR)) {
            player.identified = true;
            if (data.length > REQUEST_STR.length + 1) {
                player.clientVersion = data[REQUEST_STR.length + 1];
            }
            
            console.log(`[LOG] Player ${player.id} identified. Client version: ${player.clientVersion}`);
            
            // Build response: RESPONSE_STR + \0 + server_version
            const resLen = RESPONSE_STR.length + 2;
            const resBuffer = Buffer.alloc(resLen);
            resBuffer.write(RESPONSE_STR, 0, 'utf8');
            resBuffer[RESPONSE_STR.length] = 0; // Null terminator
            
            // Version logic from C++
            if (player.clientVersion < this.MIN_SERVER_VERSION || player.clientVersion > this.MAX_SERVER_VERSION) {
                resBuffer[RESPONSE_STR.length + 1] = this.MAX_SERVER_VERSION;
            } else {
                resBuffer[RESPONSE_STR.length + 1] = player.clientVersion;
            }
            
            console.log(`[SEND] >> Identification Response to ${player.name}: ${resBuffer.toString('hex').toUpperCase().match(/.{1,2}/g)?.join(' ')}`);
            player.socket.write(resBuffer);
        } else {
            console.log(`[WARN] Player ${player.id} failed identification. Received: ${data.toString('hex')}`);
        }
    }

    private handleSetWorld(player: Player, worldId: number, worldYSize: number) {
        player.world = worldId;
        const res = new XBuffer();
        res.beginEvent(EVENTS.SET_WORLD_RESPONSE);
        res.writeUInt8(worldId);
        res.writeUInt8(1); 
        this.send(player, res, "SET_WORLD_RESPONSE");

        if (player.status === 0) { // INITIAL_STATUS
            player.status = 1; // GAMING_STATUS
            this.broadcastStatus(player);
        }
        this.broadcastWorld(player);
    }

    private broadcastStatus(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        const res = new XBuffer();
        res.beginEvent(EVENTS.PLAYERS_STATUS);
        res.writeUInt8(player.id);
        res.writeUInt8(player.status);

        for (const p of game.players.values()) {
            this.send(p, res, "PLAYERS_STATUS");
        }
    }

    private broadcastWorld(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        const res = new XBuffer();
        res.beginEvent(EVENTS.PLAYERS_WORLD);
        res.writeUInt8(player.id);
        res.writeUInt8(player.world);

        for (const p of game.players.values()) {
            if (p.id !== player.id) {
                this.send(p, res, "PLAYERS_WORLD");
            }
        }
    }

    private send(player: Player, buffer: XBuffer, name: string) {
        const data = buffer.data;
        // In Vangers protocol, every event must be preceded by its size (short)
        const wrapped = Buffer.alloc(data.length + 2);
        wrapped.writeInt16LE(data.length, 0);
        data.copy(wrapped, 2);
        
        if (name !== "SERVER_TIME" && name !== "UPDATE_OBJECT" && name !== "DELETE_OBJECT") {
            console.log(`[SEND] >> ${name} to ${player.name}: ${wrapped.toString('hex').toUpperCase().match(/.{1,2}/g)?.join(' ')}`);
        }
        player.socket.write(wrapped);
    }

    private attachToGame(player: Player, gameId: number) {
        let game = this.games.get(gameId);
        if (!game) {
            game = {
                id: gameId,
                name: `Game ${gameId}`,
                type: GameType.UNCONFIGURED,
                players: new Map(),
                startTime: Date.now(),
                configured: false,
                serverData: null,
                objects: new Map()
            };
            this.games.set(game.id, game);
            console.log(`[LOG] Created new game: ${game.name}`);
        }

        player.gameId = game.id;
        game.players.set(player.id, player);

        const res = new XBuffer();
        res.beginEvent(EVENTS.ATTACH_TO_GAME_RESPONSE);
        res.writeInt32LE(game.id);
        res.writeUInt8(game.configured ? 1 : 0); 
        
        // C++: (unsigned int)round((double)game->birth_time * (256. / 1000.))
        // game->birth_time is SDL_GetTicks() at creation.
        const gameBirthTicks = game.startTime - this.serverStartTime;
        const gameBirthClock = Math.floor(gameBirthTicks * (256 / 1000));
        res.writeUInt32LE(gameBirthClock);
        
        res.writeUInt8(player.id);
        for (let i = 0; i < 16; i++) res.writeInt16LE(0); 

        this.send(player, res, "ATTACH_TO_GAME_RESPONSE");
        console.log(`[LOG] ${player.name} attached to game ${game.name} (ID: ${game.id}) as playerID: ${player.id}`);

        if (player.clientVersion >= 2) {
            const zTime = new XBuffer();
            zTime.beginEvent(0xE3); // zTIME_RESPONSE
            zTime.writeUInt32LE(Math.floor(Date.now() / 1000));
            this.send(player, zTime, "zTIME_RESPONSE");
        }
    }

    private sendGamesList(player: Player) {
        const res = new XBuffer();
        res.beginEvent(EVENTS.GAMES_LIST_RESPONSE);
        
        const activeGames = Array.from(this.games.values());//.filter(g => g.type !== GameType.UNCONFIGURED);
        res.writeUInt8(activeGames.length);

        for (const game of activeGames) {
            res.writeInt32LE(game.id); 
            
            const typeStr = game.type === GameType.VAN_WAR ? "V" : (game.type === GameType.MECHOSOMA ? "M" : "P");
            const uptimeSec = Math.floor((Date.now() - game.startTime) / 1000);
            const h = Math.floor(uptimeSec / 3600);
            const m = Math.floor((uptimeSec % 3600) / 60);
            const s = uptimeSec % 60;
            const timeStr = `${h}:${m}:${s}`;
            
            const detailedName = `${game.name}: ${game.players.size} ${typeStr} ${timeStr}`;
            res.writeString(detailedName);
        }
        this.send(player, res, "GAMES_LIST_RESPONSE");
    }

    private sendServerTime(player: Player) {
        const res = new XBuffer();
        res.beginEvent(EVENTS.SERVER_TIME);
        res.writeUInt32LE(this.getGlobalClock());
        this.send(player, res, "SERVER_TIME");
    }

    private broadcastPosition(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        const res = new XBuffer();
        res.beginEvent(EVENTS.PLAYERS_POSITION);
        res.writeUInt8(player.id);
        res.writeInt16LE(player.x);
        res.writeInt16LE(player.y);

        for (const p of game.players.values()) {
            if (p.id !== player.id) {
                this.send(p, res, "PLAYERS_POSITION");
            }
        }
    }

    private sendGameData(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        // C++ sends: name (string + null) + ServerData (44 bytes raw)
        const res = new XBuffer();
        res.beginEvent(EVENTS.GAME_DATA_RESPONSE);
        res.writeString(game.name);
        if (game.serverData) {
            res.writeBytes(game.serverData);
        } else {
            // Fallback: empty ServerData (44 zeros) if not yet configured
            for (let i = 0; i < 44; i++) res.writeUInt8(0);
        }
        this.send(player, res, "GAME_DATA_RESPONSE");
    }

    private handleSetGameData(player: Player, reader: XReader) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        // C++ reads: name (string), then sizeof(ServerData) = 44 bytes raw
        // ServerData layout: InitialRND(4) + GameType(4) + union(36) = 44 bytes
        const SIZEOF_SERVER_DATA = 44;
        const name = reader.readString();
        const serverData = reader.readBytes(SIZEOF_SERVER_DATA);

        game.name = name;
        game.serverData = serverData;
        game.type = serverData.readInt32LE(4); // GameType is at offset 4 in ServerData
        game.configured = true;

        console.log(`[LOG] Game ${game.id} configured: name="${name}", type=${game.type}`);

        // Broadcast current user's name to others
        this.broadcastName(player);
    }

    private broadcastName(player: Player) {
        if (player.gameId === null) return;
        const game = this.games.get(player.gameId);
        if (!game) return;

        const res = new XBuffer();
        res.beginEvent(EVENTS.PLAYERS_NAME);
        res.writeUInt8(player.id);
        res.writeString(player.name);

        for (const p of game.players.values()) {
            if (p.id !== player.id) {
                this.send(p, res, "PLAYERS_NAME");
            }
        }
    }

    private handleDisconnect(player: Player) {
        console.log(`Player disconnected: ${player.name}`);
        this.players.delete(player.id);
        if (player.gameId !== null) {
            const game = this.games.get(player.gameId);
            game?.players.delete(player.id);
            // Remove all objects owned by this player from the game store
            if (game) {
                for (const [id, obj] of game.objects) {
                    if (obj.clientId === player.id) game.objects.delete(id);
                }
            }
        }
    }

    // Relay a stored object as UPDATE_OBJECT to all other players in the same world.
    // Server always sends UPDATE_OBJECT (never CREATE_OBJECT) - mirrors C++ put_object / Player::send().
    // Wire format: event_code(1) + obj_ID(4) + client_ID(1) + time(4) + x(2) + y(2) + body
    private relayObjectUpdate(sender: Player, game: Game, obj: StoredObject) {
        const res = new XBuffer(obj.body.length + 20);
        res.beginEvent(EVENTS.UPDATE_OBJECT);
        res.writeInt32LE(obj.id);
        res.writeUInt8(obj.clientId);
        res.writeInt32LE(obj.time);
        res.writeInt16LE(obj.x);
        res.writeInt16LE(obj.y);
        res.writeBytes(obj.body);
        for (const p of game.players.values()) {
            if (p.id !== sender.id && p.world === sender.world) {
                this.send(p, res, "UPDATE_OBJECT");
            }
        }
    }

    // Relay DELETE_OBJECT to all other players in the same world.
    // Wire format: event_code(1) + obj_ID(4) + client_ID(1) + time(4) + body
    private relayObjectDelete(sender: Player, game: Game, objId: number, time: number, body: Buffer) {
        const res = new XBuffer(body.length + 15);
        res.beginEvent(EVENTS.DELETE_OBJECT);
        res.writeInt32LE(objId);
        res.writeUInt8(sender.id);
        res.writeInt32LE(time);
        res.writeBytes(body);
        for (const p of game.players.values()) {
            if (p.id !== sender.id && p.world === sender.world) {
                this.send(p, res, "DELETE_OBJECT");
            }
        }
    }
}

const server = new VangersServer(2197);
server.start();

