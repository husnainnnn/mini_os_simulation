#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <termios.h>
#include <sys/select.h>

#define MAX_TASKS 50
#define MAX_NAME_LENGTH 50
#define MAX_PATH_LENGTH 256
#define TIME_QUANTUM 2  // For Round Robin scheduling

typedef enum {
    FCFS,
    ROUND_ROBIN,
    PRIORITY
} SchedulingAlgorithm;

typedef struct {
    pid_t pid;
    char name[MAX_NAME_LENGTH];
    int ram_usage;
    int hdd_usage;
    int cpu_usage;
    int is_running;
    int is_minimized;
    time_t start_time;
    int priority;  // For priority scheduling
    int remaining_time;  // For Round Robin
} Task;

typedef struct {
    int total_ram;
    int total_hdd;
    int total_cores;
    int available_ram;
    int available_hdd;
    int available_cores;
} SystemResources;

Task tasks[MAX_TASKS];
SystemResources system_res;
int task_count = 0;
int current_mode = 0;
sem_t resource_sem;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
SchedulingAlgorithm current_scheduler = FCFS;

void boot_os();
void show_main_menu();
void execute_task(char *task_name);
void create_process(char *task_name, int ram, int hdd, int cpu);
void show_running_tasks();
void close_task(int task_index);
void minimize_task(int task_index);
void restore_task(int task_index);
void switch_mode();
void shutdown_os();
void manage_resources(int ram, int hdd, int cpu, int allocate);
int check_resources(int ram, int hdd, int cpu);
void schedule_tasks();
void set_scheduling_algorithm();
void show_scheduling_info();

void notepad();
void calculator();
void show_time();
void create_file();
void move_file();
void copy_file();
void delete_file();
void file_info();
void minesweeper();
void calendar();
void music_player();
void system_monitor();
void process_manager();
void memory_viewer();
void help_system();
void snake_game();
void end_task_immediately();

void clear_screen();
void print_header();
void print_error(char *message);
void print_success(char *message);
void print_warning(char *message);
void print_info(char *message);
void loading_animation(char *message, int seconds);
void beep_sound(int duration_ms, int frequency);
int kbhit();

int main() {
    sem_init(&resource_sem, 0, 1);
    
    printf("Enter total RAM (MB): ");
    if (scanf("%d", &system_res.total_ram) != 1) {
        print_error("Invalid input for RAM!");
        return 1;
    }
    
    printf("Enter total Hard Drive space (MB): ");
    if (scanf("%d", &system_res.total_hdd) != 1) {
        print_error("Invalid input for HDD!");
        return 1;
    }
    
    printf("Enter number of CPU cores: ");
    if (scanf("%d", &system_res.total_cores) != 1) {
        print_error("Invalid input for CPU cores!");
        return 1;
    }
    
    system_res.available_ram = system_res.total_ram;
    system_res.available_hdd = system_res.total_hdd;
    system_res.available_cores = system_res.total_cores;
    
    boot_os();
    
    int choice;
    do {
        clear_screen();
        print_header();
        show_main_menu();
        
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            print_error("Invalid input!");
            choice = 0; // Reset choice to prevent invalid behavior
        }
        
        switch(choice) {
            case 1: execute_task("Notepad"); break;
            case 2: execute_task("Calculator"); break;
            case 3: execute_task("Time"); break;
            case 4: execute_task("Calendar"); break;
            case 5: execute_task("Create File"); break;
            case 6: execute_task("Move File"); break;
            case 7: execute_task("Copy File"); break;
            case 8: execute_task("Delete File"); break;
            case 9: execute_task("File Info"); break;
            case 10: execute_task("Minesweeper"); break;
            case 11: execute_task("Music Player"); break;
            case 12: execute_task("System Monitor"); break;
            case 13: execute_task("Process Manager"); break;
            case 14: execute_task("Memory Viewer"); break;
            case 15: execute_task("Snake Game"); break;
            case 16: execute_task("Help System"); break;
            case 17: show_running_tasks(); break;
            case 18: end_task_immediately(); break;
            case 19: switch_mode(); break;
            case 20: shutdown_os(); break;
            case 21: set_scheduling_algorithm(); break;  // New option for scheduling
            default: print_error("Invalid choice!"); sleep(1); break;
        }
        
        // Schedule tasks after each operation that might affect the task queue
        if (choice != 20 && choice != 17 && choice != 18 && choice != 19 && choice != 21) {
            schedule_tasks();
        }
    } while(choice != 20);
    
    sem_destroy(&resource_sem);
    pthread_mutex_destroy(&queue_mutex);
    
    return 0;
}

void boot_os() {
    clear_screen();
    printf("\n\n");
         printf("	    ██╗  ██╗ ██████╗  \n");
	 printf(" 	    ██║  ██║ ██╔════╝ \n");
	 printf("	    ███████║ ██████╗  \n");
	 printf(" 	    ██╔══██║ ╚═══██╗  \n");
	 printf(" 	    ██║  ██║ ██████╔╝ \n");
	 printf("   	    ╚═╝  ╚═╝ ╚═════╝  \n");
         printf("      Operating System Simulator\n");
    loading_animation("Booting OS", 3);
    create_process("Calendar", 15, 2, 1);
}

void show_main_menu() {
    printf("\n=== Main Menu ===\n");
    printf("1. Notepad\n");
    printf("2. Calculator\n");
    printf("3. Time\n");
    printf("4. Calendar\n");
    printf("5. Create File\n");
    printf("6. Move File\n");
    printf("7. Copy File\n");
    printf("8. Delete File\n");
    printf("9. File Info\n");
    printf("10. Minesweeper\n");
    printf("11. Music Player\n");
    printf("12. System Monitor\n");
    printf("13. Process Manager\n");
    printf("14. Memory Viewer\n");
    printf("15. Snake Game\n");
    printf("16. Help System\n");
    printf("17. Show Running Tasks\n");
    printf("18. End Task Immediately\n");
    printf("19. Switch Mode (%s)\n", current_mode ? "Kernel" : "User");
    printf("20. Shutdown\n");
    printf("21. Set CPU Scheduling (%s)\n", 
           current_scheduler == FCFS ? "FCFS" : 
           current_scheduler == ROUND_ROBIN ? "Round Robin" : "Priority");
}

void execute_task(char *task_name) {
    int ram = 0, hdd = 0, cpu = 0;
    
    if (strcmp(task_name, "Notepad") == 0) {
        ram = 50; hdd = 5; cpu = 1;
    } else if (strcmp(task_name, "Calculator") == 0) {
        ram = 20; hdd = 1; cpu = 1;
    } else if (strcmp(task_name, "Time") == 0) {
        ram = 10; hdd = 1; cpu = 1;
    } else if (strcmp(task_name, "Calendar") == 0) {
        ram = 15; hdd = 2; cpu = 1;
    } else if (strcmp(task_name, "Create File") == 0) {
        ram = 30; hdd = 10; cpu = 1;
    } else if (strcmp(task_name, "Move File") == 0) {
        ram = 40; hdd = 10; cpu = 1;
    } else if (strcmp(task_name, "Copy File") == 0) {
        ram = 40; hdd = 10; cpu = 1;
    } else if (strcmp(task_name, "Delete File") == 0) {
        ram = 30; hdd = 1; cpu = 1;
    } else if (strcmp(task_name, "File Info") == 0) {
        ram = 25; hdd = 1; cpu = 1;
    } else if (strcmp(task_name, "Minesweeper") == 0) {
        ram = 60; hdd = 10; cpu = 2;
    } else if (strcmp(task_name, "Music Player") == 0) {
        ram = 40; hdd = 20; cpu = 1;
    } else if (strcmp(task_name, "System Monitor") == 0) {
        ram = 50; hdd = 5; cpu = 2;
    } else if (strcmp(task_name, "Process Manager") == 0) {
        ram = 45; hdd = 5; cpu = 2;
    } else if (strcmp(task_name, "Memory Viewer") == 0) {
        ram = 35; hdd = 5; cpu = 1;
    } else if (strcmp(task_name, "Snake Game") == 0) {
        ram = 55; hdd = 10; cpu = 2;
    } else if (strcmp(task_name, "Help System") == 0) {
        ram = 30; hdd = 5; cpu = 1;
    }
    
    create_process(task_name, ram, hdd, cpu);
}

void create_process(char *task_name, int ram, int hdd, int cpu) {
    if (!check_resources(ram, hdd, cpu)) {
        print_error("Not enough resources to start this task!");
        sleep(1);
        return;
    }

    printf("Run in background? (y/n): ");
    char bg;
    scanf(" %c", &bg);
    int run_in_background = (bg == 'y' || bg == 'Y');

    if (run_in_background) {
        pid_t pid = fork();
        
        if (pid == 0) {
            if (strcmp(task_name, "Calendar") == 0) {
                while(1) { sleep(60); }
            } else if (strcmp(task_name, "Time") == 0) {
                while(1) { sleep(1); }
            }
            exit(0);
        } else if (pid > 0) {
            pthread_mutex_lock(&queue_mutex);
            
            if (task_count < MAX_TASKS) {
                tasks[task_count].pid = pid;
                strcpy(tasks[task_count].name, task_name);
                tasks[task_count].ram_usage = ram;
                tasks[task_count].hdd_usage = hdd;
                tasks[task_count].cpu_usage = cpu;
                tasks[task_count].is_running = 1;
                tasks[task_count].is_minimized = 0;
                tasks[task_count].start_time = time(NULL);
                tasks[task_count].priority = rand() % 5 + 1;  // Random priority 1-5
                tasks[task_count].remaining_time = (rand() % 10) + 1;  // Random burst time 1-10
                
                manage_resources(ram, hdd, cpu, 1);
                task_count++;
                
                print_success("Task started in background!");
            } else {
                print_error("Maximum number of tasks reached!");
                kill(pid, SIGTERM);
            }
            
            pthread_mutex_unlock(&queue_mutex);
            sleep(1);
        }
    } else {
        if (strcmp(task_name, "Notepad") == 0) {
            notepad();
        } else if (strcmp(task_name, "Calculator") == 0) {
            calculator();
        } else if (strcmp(task_name, "Time") == 0) {
            show_time();
        } else if (strcmp(task_name, "Calendar") == 0) {
            calendar();
        } else if (strcmp(task_name, "Create File") == 0) {
            create_file();
        } else if (strcmp(task_name, "Move File") == 0) {
            move_file();
        } else if (strcmp(task_name, "Copy File") == 0) {
            copy_file();
        } else if (strcmp(task_name, "Delete File") == 0) {
            delete_file();
        } else if (strcmp(task_name, "File Info") == 0) {
            file_info();
        } else if (strcmp(task_name, "Minesweeper") == 0) {
            minesweeper();
        } else if (strcmp(task_name, "Music Player") == 0) {
            music_player();
        } else if (strcmp(task_name, "System Monitor") == 0) {
            system_monitor();
        } else if (strcmp(task_name, "Process Manager") == 0) {
            process_manager();
        } else if (strcmp(task_name, "Memory Viewer") == 0) {
            memory_viewer();
        } else if (strcmp(task_name, "Snake Game") == 0) {
            snake_game();
        } else if (strcmp(task_name, "Help System") == 0) {
            help_system();
        }
    }
}

void schedule_tasks() {
    if (task_count == 0) return;

    pthread_mutex_lock(&queue_mutex);

    switch(current_scheduler) {
        case FCFS:
            // First-Come-First-Serve - no reordering needed
            break;
            
        case ROUND_ROBIN: {
            // Move the first task to the end of the queue
            Task temp = tasks[0];
            for (int i = 0; i < task_count - 1; i++) {
                tasks[i] = tasks[i + 1];
            }
            tasks[task_count - 1] = temp;
            
            // Decrease remaining time for current task
            tasks[task_count - 1].remaining_time -= TIME_QUANTUM;
            
            // If task is done, remove it
            if (tasks[task_count - 1].remaining_time <= 0) {
                manage_resources(tasks[task_count - 1].ram_usage, 
                                tasks[task_count - 1].hdd_usage, 
                                tasks[task_count - 1].cpu_usage, 0);
                kill(tasks[task_count - 1].pid, SIGTERM);
                waitpid(tasks[task_count - 1].pid, NULL, 0);
                task_count--;
            }
            break;
        }
            
        case PRIORITY: {
            // Sort tasks by priority (higher priority first)
            for (int i = 0; i < task_count - 1; i++) {
                for (int j = 0; j < task_count - i - 1; j++) {
                    if (tasks[j].priority < tasks[j + 1].priority) {
                        Task temp = tasks[j];
                        tasks[j] = tasks[j + 1];
                        tasks[j + 1] = temp;
                    }
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&queue_mutex);
}

void set_scheduling_algorithm() {
    clear_screen();
    printf("=== CPU Scheduling Algorithm ===\n");
    printf("Current algorithm: %s\n", 
           current_scheduler == FCFS ? "First-Come-First-Serve" : 
           current_scheduler == ROUND_ROBIN ? "Round Robin" : "Priority");
    
    printf("\nSelect new algorithm:\n");
    printf("1. First-Come-First-Serve (FCFS)\n");
    printf("2. Round Robin\n");
    printf("3. Priority Scheduling\n");
    printf("4. Back to Main Menu\n");
    
    int choice;
    printf("\nEnter your choice: ");
    if (scanf("%d", &choice) != 1) {
        print_error("Invalid input!");
        return;
    }
    
    switch(choice) {
        case 1: current_scheduler = FCFS; break;
        case 2: current_scheduler = ROUND_ROBIN; break;
        case 3: current_scheduler = PRIORITY; break;
        case 4: return;
        default: print_error("Invalid choice!"); sleep(1); return;
    }
    
    print_success("Scheduling algorithm changed!");
    sleep(1);
}

void show_scheduling_info() {
    printf("\n=== CPU Scheduling Information ===\n");
    printf("Current algorithm: %s\n", 
           current_scheduler == FCFS ? "First-Come-First-Serve" : 
           current_scheduler == ROUND_ROBIN ? "Round Robin" : "Priority");
    
    if (current_scheduler == ROUND_ROBIN) {
        printf("Time Quantum: %d seconds\n", TIME_QUANTUM);
    }
    
    printf("\nTask Queue:\n");
    printf("%-5s %-20s %-10s %-10s %-10s\n", 
           "ID", "Name", "Priority", "Rem Time", "Status");
    
    for (int i = 0; i < task_count; i++) {
        printf("%-5d %-20s %-10d %-10d %-10s\n", 
               i, 
               tasks[i].name, 
               tasks[i].priority,
               tasks[i].remaining_time,
               tasks[i].is_minimized ? "Minimized" : "Running");
    }
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

void end_task_immediately() {
    clear_screen();
    printf("=== End Task Immediately ===\n");
    
    if (task_count == 0) {
        printf("No tasks are currently running.\n");
        sleep(1);
        return;
    }
    
    printf("Running Tasks:\n");
    for (int i = 0; i < task_count; i++) {
        printf("%d. %s(PID: %d)\n", i, tasks[i].name, tasks[i].pid);
    }
    
    printf("\nEnter task number to end (or -1 to cancel): ");
    int task_num;
    if (scanf("%d", &task_num) != 1) {
        print_error("Invalid input!");
        return;
    }
    
    if (task_num >= 0 && task_num < task_count) {
        close_task(task_num);
    }
}

void show_running_tasks() {
    clear_screen();
    print_header();
    printf("\n=== Running Tasks ===\n");
    
    if (task_count == 0) {
        printf("No tasks are currently running.\n");
    } else {
        printf("%-5s %-20s %-10s %-10s %-10s %-10s %-15s\n", 
               "ID", "Name", "RAM(MB)", "HDD(MB)", "CPU", "Status", "Running Time");
        
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].is_running) {
                time_t now = time(NULL);
                double running_time = difftime(now, tasks[i].start_time);
                
                printf("%-5d %-20s %-10d %-10d %-10d %-10s %.0f seconds\n", 
                       i, 
                       tasks[i].name, 
                       tasks[i].ram_usage, 
                       tasks[i].hdd_usage, 
                       tasks[i].cpu_usage,
                       tasks[i].is_minimized ? "Minimized" : "Running",
                       running_time);
            }
        }
    }
    
    if (current_mode == 1) {
        printf("\nKernel Mode Options:\n");
        printf("1. Close Task\n");
        printf("2. Minimize Task\n");
        printf("3. Restore Task\n");
        printf("4. Show Scheduling Info\n");
        printf("5. Back to Main Menu\n");
        
        int choice, task_id;
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            print_error("Invalid input!");
            return;
        }
        
        if (choice >= 1 && choice <= 3) {
            printf("Enter Task ID: ");
            if (scanf("%d", &task_id) != 1) {
                print_error("Invalid input!");
                return;
            }
            
            if (task_id >= 0 && task_id < task_count && tasks[task_id].is_running) {
                switch(choice) {
                    case 1: close_task(task_id); break;
                    case 2: minimize_task(task_id); break;
                    case 3: restore_task(task_id); break;
                }
            } else {
                print_error("Invalid Task ID!");
                sleep(1);
            }
        } else if (choice == 4) {
            show_scheduling_info();
        }
    } else {
        printf("\nPress any key to continue...");
        getchar(); getchar();
    }
}

void close_task(int task_index) {
    if (task_index < 0 || task_index >= task_count || !tasks[task_index].is_running) {
        print_error("Invalid task index!");
        return;
    }
    
    kill(tasks[task_index].pid, SIGTERM);
    waitpid(tasks[task_index].pid, NULL, 0);
    
    manage_resources(tasks[task_index].ram_usage, 
                     tasks[task_index].hdd_usage, 
                     tasks[task_index].cpu_usage, 0);
    
    pthread_mutex_lock(&queue_mutex);
    
    for (int i = task_index; i < task_count - 1; i++) {
        tasks[i] = tasks[i + 1];
    }
    task_count--;
    
    pthread_mutex_unlock(&queue_mutex);
    
    print_success("Task closed successfully!");
    sleep(1);
}

void minimize_task(int task_index) {
    if (task_index < 0 || task_index >= task_count || !tasks[task_index].is_running) {
        print_error("Invalid task index!");
        return;
    }
    
    tasks[task_index].is_minimized = 1;
    print_success("Task minimized successfully!");
    sleep(1);
}

void restore_task(int task_index) {
    if (task_index < 0 || task_index >= task_count || !tasks[task_index].is_running) {
        print_error("Invalid task index!");
        return;
    }
    
    tasks[task_index].is_minimized = 0;
    print_success("Task restored successfully!");
    sleep(1);
}

void switch_mode() {
    current_mode = !current_mode;
    print_success(current_mode ? "Switched to Kernel Mode" : "Switched to User Mode");
    sleep(1);
}

void shutdown_os() {
    clear_screen();
    printf("\n\n");
    printf("    ███████╗██╗  ██╗██╗   ██╗████████╗███████╗██████╗ \n");
    printf("    ██╔════╝██║  ██║██║   ██║╚══██╔══╝██╔════╝██╔══██╗\n");
    printf("    ███████╗███████║██║   ██║   ██║   █████╗  ██║  ██║\n");
    printf("    ╚════██║██╔══██║██║   ██║   ██║   ██╔══╝  ██║  ██║\n");
    printf("    ███████║██║  ██║╚██████╔╝   ██║   ███████╗██████╔╝\n");
    printf("    ╚══════╝╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚══════╝╚═════╝ \n");
    
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].is_running) {
            kill(tasks[i].pid, SIGTERM);
            waitpid(tasks[i].pid, NULL, 0);
        }
    }
    
    loading_animation("Shutting down", 3);
    exit(0);
}

void manage_resources(int ram, int hdd, int cpu, int allocate) {
    sem_wait(&resource_sem);
    
    if (allocate) {
        system_res.available_ram -= ram;
        system_res.available_hdd -= hdd;
        system_res.available_cores -= cpu;
    } else {
        system_res.available_ram += ram;
        system_res.available_hdd += hdd;
        system_res.available_cores += cpu;
    }
    
    sem_post(&resource_sem);
}

int check_resources(int ram, int hdd, int cpu) {
    sem_wait(&resource_sem);
    
    int result = (system_res.available_ram >= ram) && 
                 (system_res.available_hdd >= hdd) && 
                 (system_res.available_cores >= cpu);
    
    sem_post(&resource_sem);
    return result;
}

void notepad() {
    clear_screen();
    printf("=== Notepad ===\n");
    printf("Type your text (enter 'SAVE' on a new line to save and exit):\n\n");
    
    char filename[MAX_PATH_LENGTH];
    printf("Enter filename to save: ");
    scanf("%s", filename);
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file!\n");
        return;
    }
    
    char buffer[256];
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        
        if (strcmp(buffer, "SAVE\n") == 0) {
            break;
        }
        
        fputs(buffer, file);
    }
    
    fclose(file);
    printf("File saved successfully as %s\n", filename);
    sleep(2);
}

void calculator() {
    clear_screen();
    printf("=== Calculator ===\n");
    printf("Operations: +, -, *, /, %% (enter 'q' to quit)\n\n");
    
    while (1) {
        double num1, num2;
        char op;
        
        printf("Enter expression (e.g., 5 + 3): ");
        if (scanf("%lf %c %lf", &num1, &op, &num2) != 3) {
            break;
        }
        
        double result;
        switch(op) {
            case '+': result = num1 + num2; break;
            case '-': result = num1 - num2; break;
            case '*': result = num1 * num2; break;
            case '/': 
                if (num2 == 0) {
                    printf("Error: Division by zero!\n");
                    continue;
                }
                result = num1 / num2; 
                break;
            case '%': 
                result = (int)num1 % (int)num2; 
                break;
            default: 
                printf("Invalid operator!\n");
                continue;
        }
        
        printf("Result: %.2lf\n", result);
        
        while ((getchar()) != '\n');
        
        printf("Press q to quit or any other key to continue...");
        if (getchar() == 'q') {
            break;
        }
    }
}

void show_time() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    clear_screen();
    printf("=== Current Time ===\n");
    printf("%02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    printf("%02d/%02d/%04d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

void calendar() {
    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        
        clear_screen();
        printf("=== Calendar ===\n");
        
        printf("     %02d/%04d\n", tm_info->tm_mon + 1, tm_info->tm_year + 1900);
        printf("Su Mo Tu We Th Fr Sa\n");
        
        struct tm first_day = *tm_info;
        first_day.tm_mday = 1;
        mktime(&first_day);
        
        int day_of_week = first_day.tm_wday;
        int days_in_month;
        
        switch(tm_info->tm_mon + 1) {
            case 4: case 6: case 9: case 11: days_in_month = 30; break;
            case 2: 
                days_in_month = ((tm_info->tm_year + 1900) % 4 == 0) ? 29 : 28;
                break;
            default: days_in_month = 31;
        }
        
        for (int i = 0; i < day_of_week; i++) {
            printf("   ");
        }
        
        for (int day = 1; day <= days_in_month; day++) {
            printf("%2d ", day);
            if ((day + day_of_week) % 7 == 0 || day == days_in_month) {
                printf("\n");
            }
        }
        
        printf("\nPress 'q' to quit...");
        if (kbhit()) {
            char ch = getchar();
            if (ch == 'q' || ch == 'Q') {
                break;
            }
        }
        
        sleep(1);
    }
}

void create_file() {
    clear_screen();
    printf("=== Create File ===\n");
    
    char filename[MAX_PATH_LENGTH];
    printf("Enter filename: ");
    scanf("%s", filename);
    
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file!\n");
    } else {
        printf("File created successfully: %s\n", filename);
        fclose(file);
    }
    
    sleep(2);
}

void move_file() {
    clear_screen();
    printf("=== Move File ===\n");
    
    char source[MAX_PATH_LENGTH], dest[MAX_PATH_LENGTH];
    printf("Enter source file path: ");
    scanf("%s", source);
    printf("Enter destination path: ");
    scanf("%s", dest);
    
    if (rename(source, dest) == 0) {
        printf("File moved successfully from %s to %s\n", source, dest);
    } else {
        printf("Error moving file!\n");
    }
    
    sleep(2);
}

void copy_file() {
    clear_screen();
    printf("=== Copy File ===\n");
    
    char source[MAX_PATH_LENGTH], dest[MAX_PATH_LENGTH];
    printf("Enter source file path: ");
    scanf("%s", source);
    printf("Enter destination path: ");
    scanf("%s", dest);
    
    FILE *src_file = fopen(source, "rb");
    if (src_file == NULL) {
        printf("Error opening source file!\n");
        sleep(2);
        return;
    }
    
    FILE *dest_file = fopen(dest, "wb");
    if (dest_file == NULL) {
        printf("Error creating destination file!\n");
        fclose(src_file);
        sleep(2);
        return;
    }
    
    char buffer[1024];
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        fwrite(buffer, 1, bytes, dest_file);
    }
    
    fclose(src_file);
    fclose(dest_file);
    
    printf("File copied successfully from %s to %s\n", source, dest);
    sleep(2);
}

void delete_file() {
    clear_screen();
    printf("=== Delete File ===\n");
    
    char filename[MAX_PATH_LENGTH];
    printf("Enter filename to delete: ");
    scanf("%s", filename);
    
    if (remove(filename) == 0) {
        printf("File deleted successfully: %s\n", filename);
    } else {
        printf("Error deleting file!\n");
    }
    
    sleep(2);
}

void file_info() {
    clear_screen();
    printf("=== File Info ===\n");
    
    char filename[MAX_PATH_LENGTH];
    printf("Enter filename: ");
    scanf("%s", filename);
    
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        printf("Error getting file info!\n");
        sleep(2);
        return;
    }
    
    printf("\nFile Information for: %s\n", filename);
    printf("Size: %ld bytes\n", file_stat.st_size);
    printf("Permissions: %o\n", file_stat.st_mode & 0777);
    printf("Last accessed: %s", ctime(&file_stat.st_atime));
    printf("Last modified: %s", ctime(&file_stat.st_mtime));
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

void minesweeper() {
    clear_screen();
    printf("=== Minesweeper ===\n");
    printf("A simplified version of Minesweeper\n\n");
    
    const int SIZE = 5;
    const int MINES = 5;
    
    char board[SIZE][SIZE];
    char visible[SIZE][SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = '0';
            visible[i][j] = '.';
        }
    }
    
    srand(time(NULL));
    for (int m = 0; m < MINES; ) {
        int x = rand() % SIZE;
        int y = rand() % SIZE;
        
        if (board[x][y] != '*') {
            board[x][y] = '*';
            m++;
            
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    int nx = x + i;
                    int ny = y + j;
                    
                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] != '*') {
                        board[nx][ny]++;
                    }
                }
            }
        }
    }
    
    int game_over = 0;
    int cells_revealed = 0;
    int total_safe = SIZE * SIZE - MINES;
    
    while (!game_over && cells_revealed < total_safe) {
        printf("   ");
        for (int i = 0; i < SIZE; i++) printf("%d ", i);
        printf("\n");
        
        for (int i = 0; i < SIZE; i++) {
            printf("%d |", i);
            for (int j = 0; j < SIZE; j++) {
                printf("%c ", visible[i][j]);
            }
            printf("\n");
        }
        
        int x, y;
        printf("\nEnter row and column (0-%d): ", SIZE-1);
        scanf("%d %d", &x, &y);
        
        if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
            printf("Invalid coordinates!\n");
            continue;
        }
        
        if (visible[x][y] != '.') {
            printf("Cell already revealed!\n");
            continue;
        }
        
        if (board[x][y] == '*') {
            game_over = 1;
            visible[x][y] = '*';
        } else {
            visible[x][y] = board[x][y];
            cells_revealed++;
        }
    }
    
    if (game_over) {
        printf("\nBOOM! You hit a mine!\n");
    } else {
        printf("\nCongratulations! You cleared the minefield!\n");
    }
    
    printf("\nFinal Board:\n");
    printf("   ");
    for (int i = 0; i < SIZE; i++) printf("%d ", i);
    printf("\n");
    
    for (int i = 0; i < SIZE; i++) {
        printf("%d |", i);
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

void music_player() {
    clear_screen();
    printf("=== Music Player ===\n");
    printf("Playing background music...\n");
    
    for (int i = 0; i < 5; i++) {
        if (task_count == 0 || !tasks[task_count-1].is_running) break;
        
        printf("Playing note %d/5...\n", i+1);
        beep_sound(500, 440 + i * 100);
        sleep(1);
    }
    
    printf("Music finished playing.\n");
    sleep(2);
}

void system_monitor() {
    while (1) {
        clear_screen();
        printf("=== System Monitor ===\n");
        
        printf("\nSystem Resources:\n");
        printf("RAM: %d/%d MB (%.1f%% used)\n", 
               system_res.total_ram - system_res.available_ram, 
               system_res.total_ram,
               ((float)(system_res.total_ram - system_res.available_ram) / system_res.total_ram * 100));
        printf("HDD: %d/%d MB (%.1f%% used)\n", 
               system_res.total_hdd - system_res.available_hdd, 
               system_res.total_hdd,
               ((float)(system_res.total_hdd - system_res.available_hdd) / system_res.total_hdd * 100));
        printf("CPU Cores: %d/%d in use\n", 
               system_res.total_cores - system_res.available_cores, 
               system_res.total_cores);
        
        printf("\nPress q to quit or any other key to refresh...");
        char ch = getchar();
        if (ch == 'q') {
            break;
        }
        while ((getchar()) != '\n'); // Clear input buffer
    }
}

void process_manager() {
    while (1) {
        clear_screen();
        printf("=== Process Manager ===\n");
        
        if (task_count == 0) {
            printf("No processes running.\n");
        } else {
            printf("%-5s %-20s %-10s %-10s %-10s %-10s\n", 
                   "ID", "Name", "RAM(MB)", "HDD(MB)", "CPU", "Status");
            
            for (int i = 0; i < task_count; i++) {
                printf("%-5d %-20s %-10d %-10d %-10d %-10s\n", 
                       i, 
                       tasks[i].name, 
                       tasks[i].ram_usage, 
                       tasks[i].hdd_usage, 
                       tasks[i].cpu_usage,
                       tasks[i].is_minimized ? "Minimized" : "Running");
            }
        }
        
        printf("\nPress q to quit or any other key to refresh...");
        char ch = getchar();
        if (ch == 'q') {
            break;
        }
        while ((getchar()) != '\n'); // Clear input buffer
    }
}

void memory_viewer() {
    clear_screen();
    printf("=== Memory Viewer ===\n");
    
    printf("\nMemory Allocation Map:\n");
    printf("Total RAM: %d MB\n", system_res.total_ram);
    printf("Used RAM: %d MB\n", system_res.total_ram - system_res.available_ram);
    printf("Free RAM: %d MB\n", system_res.available_ram);
    
    printf("\nProcess Memory Usage:\n");
    for (int i = 0; i < task_count; i++) {
        printf("%-20s: %4d MB\n", tasks[i].name, tasks[i].ram_usage);
    }
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

void snake_game() {
    clear_screen();
    printf("=== Snake Game ===\n");
    printf("Use WASD keys to move. Press q to quit.\n");
    
    const int WIDTH = 20;
    const int HEIGHT = 10;
    
    int snakeX[100], snakeY[100];
    int snakeLength = 1;
    snakeX[0] = WIDTH / 2;
    snakeY[0] = HEIGHT / 2;
    
    int foodX = rand() % (WIDTH - 2) + 1;
    int foodY = rand() % (HEIGHT - 2) + 1;
    
    char direction = 'd';
    int game_over = 0;
    struct termios oldt, newt;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    while (!game_over) {
        clear_screen();
        printf("=== Snake Game ===\n");
        
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (i == 0 || i == HEIGHT-1 || j == 0 || j == WIDTH-1) {
                    printf("#");
                } else if (i == foodY && j == foodX) {
                    printf("F");
                } else {
                    int isSnake = 0;
                    for (int k = 0; k < snakeLength; k++) {
                        if (snakeX[k] == j && snakeY[k] == i) {
                            printf("O");
                            isSnake = 1;
                            break;
                        }
                    }
                    if (!isSnake) printf(" ");
                }
            }
            printf("\n");
        }
        
        if (kbhit()) {
            char ch = getchar();
            switch(ch) {
                case 'w': if (direction != 's') direction = 'w'; break;
                case 'a': if (direction != 'd') direction = 'a'; break;
                case 's': if (direction != 'w') direction = 's'; break;
                case 'd': if (direction != 'a') direction = 'd'; break;
                case 'q': game_over = 1; continue;
            }
        }
        
        for (int i = snakeLength - 1; i > 0; i--) {
            snakeX[i] = snakeX[i-1];
            snakeY[i] = snakeY[i-1];
        }
        
        switch(direction) {
            case 'w': snakeY[0]--; break;
            case 'a': snakeX[0]--; break;
            case 's': snakeY[0]++; break;
            case 'd': snakeX[0]++; break;
        }
        
        if (snakeX[0] <= 0 || snakeX[0] >= WIDTH-1 || 
            snakeY[0] <= 0 || snakeY[0] >= HEIGHT-1) {
            game_over = 1;
        }
        
        for (int i = 1; i < snakeLength; i++) {
            if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
                game_over = 1;
            }
        }
        
        if (snakeX[0] == foodX && snakeY[0] == foodY) {
            snakeLength++;
            foodX = rand() % (WIDTH-2) + 1;
            foodY = rand() % (HEIGHT-2) + 1;
        }
        
        usleep(200000);
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    printf("\nGame Over! Your score: %d\n", snakeLength - 1);
    printf("Press any key to continue...");
    getchar();
}

void help_system() {
    clear_screen();
    printf("=== Help System ===\n");
    
    printf("Available Commands:\n");
    printf("1. Notepad - Simple text editor\n");
    printf("2. Calculator - Basic arithmetic operations\n");
    printf("3. Time - Shows current time and date\n");
    printf("4. Calendar - Shows current month calendar\n");
    printf("5. Create File - Creates a new empty file\n");
    printf("6. Move File - Moves a file to new location\n");
    printf("7. Copy File - Copies a file to new location\n");
    printf("8. Delete File - Deletes a file\n");
    printf("9. File Info - Shows information about a file\n");
    printf("10. Minesweeper - Simple minesweeper game\n");
    printf("11. Music Player - Plays simple background music\n");
    printf("12. System Monitor - Shows system resource usage\n");
    printf("13. Process Manager - Shows running processes\n");
    printf("14. Memory Viewer - Shows memory allocation\n");
    printf("15. Snake Game - Classic snake game\n");
    printf("16. Help System - Shows this help message\n");
    printf("17. Show Running Tasks - List all running tasks\n");
    printf("18. Switch Mode - Toggle between User and Kernel mode\n");
    printf("19. Shutdown - Shuts down the OS\n");
    printf("20. Set CPU Scheduling - Change CPU scheduling algorithm\n");
    
    printf("\nIn Kernel Mode, you can:\n");
    printf("- Close running tasks\n");
    printf("- Minimize tasks\n");
    printf("- Restore minimized tasks\n");
    printf("- View scheduling information\n");
    
    printf("\nPress any key to continue...");
    getchar(); getchar();
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void print_header() {
    printf("OS Simulator - RAM: %d/%d MB | HDD: %d/%d MB | Cores: %d/%d | Mode: %s | Scheduler: %s\n\n",
           system_res.total_ram - system_res.available_ram, system_res.total_ram,
           system_res.total_hdd - system_res.available_hdd, system_res.total_hdd,
           system_res.total_cores - system_res.available_cores, system_res.total_cores,
           current_mode ? "Kernel" : "User",
           current_scheduler == FCFS ? "FCFS" : 
           current_scheduler == ROUND_ROBIN ? "Round Robin" : "Priority");
}

void print_error(char *message) {
    printf("\033[1;31m[ERROR] %s\033[0m\n", message);
}

void print_success(char *message) {
    printf("\033[1;32m[SUCCESS] %s\033[0m\n", message);
}

void print_warning(char *message) {
    printf("\033[1;33m[WARNING] %s\033[0m\n", message);
}

void print_info(char *message) {
    printf("\033[1;34m[INFO] %s\033[0m\n", message);
}

void loading_animation(char *message, int seconds) {
    printf("\n%s ", message);
    fflush(stdout);
    
    for (int i = 0; i < seconds; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

void beep_sound(int duration_ms, int frequency) {
    #ifdef _WIN32
        Beep(frequency, duration_ms);
    #else
        char command[50];
        sprintf(command, "beep -f %d -l %d", frequency, duration_ms);
        system(command);
    #endif
}
