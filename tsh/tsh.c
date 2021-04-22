/* 
 * tsh - A tiny shell program with job control
 * <The line above is not a sufficient documentation.
 *  You will need to write your program documentation.>
 */

#include "tsh_helper.h"

/*
 * If DEBUG is defined, enable contracts and printing on dbg_printf.
 */
#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif

/* Function prototypes */
void eval(const char *cmdline);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void sigquit_handler(int sig);
void restore_signal_handlers();
void block_signals();
void unblock_signals();
int  update_jobs(pid_t pid, int status);
void foreground_wait();



/*
 * <Write main's function header documentation. What does main do?>
 * "Each function should be prefaced with a comment describing the purpose
 *  of the function (in a sentence or two), the function's arguments and
 *  return value, any error cases that are relevant to the caller,
 *  any pertinent side effects, and any assumptions that the function makes."
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE_TSH];  // Cmdline for fgets
    bool emit_prompt = true;    // Emit prompt (default)

    // Redirect stderr to stdout (so that driver will get all output
    // on the pipe connected to stdout)
    Dup2(STDOUT_FILENO, STDERR_FILENO);

    // Parse the command line
    while ((c = getopt(argc, argv, "hvp")) != EOF)
    {
        switch (c)
        {
        case 'h':                   // Prints help message
            usage();
            break;
        case 'v':                   // Emits additional diagnostic info
            verbose = true;
            break;
        case 'p':                   // Disables prompt printing
            emit_prompt = false;  
            break;
        default:
            usage();
        }
    }

    // Install the signal handlers
    Signal(SIGINT,  sigint_handler);   // Handles ctrl-c
    Signal(SIGTSTP, sigtstp_handler);  // Handles ctrl-z
    Signal(SIGCHLD, sigchld_handler);  // Handles terminated or stopped child

    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    Signal(SIGQUIT, sigquit_handler); 

    // Initialize the job list
    initjobs(job_list);

    // Execute the shell's read/eval loop
    while (true)
    {
        if (emit_prompt)
        {
            printf("%s", prompt);
            fflush(stdout);
        }

        if ((fgets(cmdline, MAXLINE_TSH, stdin) == NULL) && ferror(stdin))
        {
            app_error("fgets error");
        }

        if (feof(stdin))
        { 
            // End of file (ctrl-d)
            printf ("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }
        
        // Remove the trailing newline
        cmdline[strlen(cmdline)-1] = '\0';
        
        // Evaluate the command line
        eval(cmdline);
        
        fflush(stdout);
    } 
    
    return -1; // control never reaches here
}


/* Handy guide for eval:
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg),
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.
 * Note: each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */

/* 
 * <What does eval do?>
 */
void eval(const char *cmdline) 
{
    parseline_return parse_result;     
    struct cmdline_tokens token;
    
    // Parse command line
    parse_result = parseline(cmdline, &token);

    if (parse_result == PARSELINE_ERROR || parse_result == PARSELINE_EMPTY)
    {
        return;
    }
    else
    {
        // Block signals
		block_signals();
        
        int jid, use_jid = 0;
        pid_t pid;
        struct job_t *j;
        
		if (parse_result == PARSELINE_FG) {
			// If the command is a builtin, execute without forking
			if (token.builtin != BUILTIN_NONE) {
				// Choose which builtin to execute
				switch(token.builtin) {
					case BUILTIN_QUIT:
                        // Unblock signals
                        unblock_signals();
                        raise(SIGQUIT);
						return; // should not get here
					case BUILTIN_JOBS:
						// List jobs in shell
						listjobs(job_list, 1);
						// Unblock signals now
						unblock_signals();
						return;
                    case BUILTIN_BG:
                        // Parse command for '%'
                        if (token.argv[1][0] == '%') use_jid = 1;
                        // Get job id and process id
                        if (use_jid) {
                            jid = atoi(token.argv[1]+1);
                            j = getjobjid(job_list, jid);
                            pid = j->pid;
                        } else {
                            pid = atoi(token.argv[1]);
                            j = getjobpid(job_list, pid);
                            jid = j->jid;
                        }
                        // Unblock signals
                        unblock_signals();
                        // Send SIGCONT signal
                        Kill(-pid, SIGCONT);
                        // Change status based on transitions
                        printf("[%d] (%d) %s\n", jid, pid, j->cmdline);
                        j->state = BG;
						return;
					case BUILTIN_FG:
                        // Parse command for '%'
                        if (token.argv[1][0] == '%') use_jid = 1;
                        // Get job id and process id
                        if (use_jid) {
                            jid = atoi(token.argv[1]+1);
                            j = getjobjid(job_list, jid);
                            pid = j->pid;
                        } else {
                            pid = atoi(token.argv[1]);
                            j = getjobpid(job_list, pid);
                            jid = j->jid;
                        }
                        // Unblock signals
                        unblock_signals();
                        // Send SIGCONT signal
                        Kill(-pid, SIGCONT);
                        // Change status based on transitions
                        j->state = FG;
                        // Wait in the foreground
                        foreground_wait();
						return;
					default:
						break;
				} 
			}
			
			// Fork into child process
			pid_t child_pid = Fork();
			// Inside child process
			if (child_pid == 0) {
                // Unblock signals
				unblock_signals();
                // Restore default handlers in child
                restore_signal_handlers();
				// Set different process group
				Setpgid(0, 0);
				// Execute the command and exit
				Execve(token.argv[0], token.argv, environ);
                exit(0);
			}
            // Inside parent process
			else {
                addjob(job_list, child_pid, FG, cmdline);
                // Unblock signals
				unblock_signals();
                // Wait in the foreground
                foreground_wait();
                return;
			}
		}
		else if (parse_result == PARSELINE_BG) {
			// Fork into child process
			pid_t bg_pid = Fork();
			// Inside child process
			if (bg_pid == 0) {
                // Unblock signals
                unblock_signals();
                // Restore default handlers in child
                restore_signal_handlers();
				// Set process group id
				Setpgid(0,0);
				// Execute the command
				Execve(token.argv[0], token.argv, environ);
                exit(0);
			}
			else {
				// Add job to job list
				addjob(job_list, bg_pid, BG, cmdline);
				// Display job that was just created
				struct job_t *newjob = getjobpid(job_list, bg_pid);
				printf("[%d] (%d) %s\n", newjob->jid, newjob->pid, cmdline);
                // Unblock signals
                unblock_signals();
                return;
			}
		}
	}

    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * <What does sigchld_handler do?>
 */
void sigchld_handler(int sig) 
{
    int status;
    pid_t pid;
    do
       pid = waitpid(WAIT_ANY, &status, WNOHANG | WUNTRACED);
    while(!update_jobs(pid, status));
    return;
}

/* 
 * <What does sigint_handler do?>
 */
void sigint_handler(int sig) 
{
    block_signals();
    pid_t pid = fgpid(job_list);  
    pid_t gpid = __getpgid(pid);
    
    //check for valid process group id
    if (gpid != getpid()) {     
        Kill(-gpid, sig);
    }
    unblock_signals();
    return;
}

/*
 * <What does sigtstp_handler do?>
 */
void sigtstp_handler(int sig) 
{
    block_signals();
    pid_t pid = fgpid(job_list);  
    pid_t gpid = __getpgid(pid);
    
    //check for valid process group id
    if (gpid != getpid()) {     
        Kill(-gpid, sig);
    }
    unblock_signals();
    return;
}

void restore_signal_handlers()
{
    Signal(SIGINT, SIG_DFL);
    Signal(SIGTSTP, SIG_DFL);
    Signal(SIGCHLD, SIG_DFL);
}

int update_jobs(pid_t pid, int status) {
    // Check if job exists
    if(pid < 1) return -1;

    // Check status of jobs and update job list
    block_signals();
    struct job_t *job = getjobpid(job_list, pid);
    if (WIFSIGNALED(status)) {
        printf("Job [%d] (%d) terminated by signal %d\n", 
               job->jid, job->pid, WTERMSIG(status));
        // Delete job since it has been terminated
        deletejob(job_list, job->pid);
    }
    else if (WIFSTOPPED(status)) {
        printf("Job [%d] (%d) stopped by signal %d\n", 
               job->jid, job->pid, WSTOPSIG(status));
        // Set job status to stopped
        job->state = ST;
    }
    else {
        deletejob(job_list, job->pid);
    }
    unblock_signals();
    return 0;
}

void block_signals() {
    sigset_t mask;
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGTSTP);
    Sigaddset(&mask, SIGINT);
    Sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signals() {
    sigset_t mask;
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGTSTP);
    Sigaddset(&mask, SIGINT);
    Sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void foreground_wait() {
    sigset_t mask, oldmask;
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGTSTP);
    Sigaddset(&mask, SIGINT);
    Sigprocmask(SIG_BLOCK, &mask, &oldmask);
    while(fgpid(job_list) != 0) Sigsuspend(&oldmask);
}