#define MAX_PLAYERS 10

#define MULTICAST "226.1.1.1"
#define MULTICAST_PORT 4321

typedef struct Player {
    int x;
    int y;
    int color;
} Player;

typedef struct PlayerAction {
    int type;
    int id;
    int move;
} PlayerAction;

typedef struct GameData {
    int count;
    Player players[MAX_PLAYERS];
} GameData;

typedef struct InitGameData {
    char address[16];
    int id;
    GameData gd;
} InitGameData;