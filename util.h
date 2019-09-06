#define TRUE 1
#define FALSE !TRUE

// Shells process id, pgid, terminal mode
static pid_t bash_pid;
static pid_t grbash_pid;

static int is_interact;
static struct termios bash_mode;

static char* currentDirectory;

extern char** environ;

struct sigaction act_child;
struct sigaction act_int;

int no_reprint_prmpt;

pid_t pid;



void signalHandler_child(int p);

void signalHandler_int(int p);


int changeDirectory(char * args[]);
