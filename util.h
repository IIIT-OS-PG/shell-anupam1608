#define TRUE 1
#define FALSE !TRUE

// Shells process id, pgid, terminal mode
static pid_t GBSH_PID;
static pid_t GBSH_PGID;

static int GBSH_IS_INTERACTIVE;
static struct termios GBSH_TMODES;

static char* currentDirectory;

extern char** environ;

struct sigaction act_child;
struct sigaction act_int;

int no_reprint_prmpt;

pid_t pid;



void signalHandler_child(int p);

void signalHandler_int(int p);


int changeDirectory(char * args[]);
