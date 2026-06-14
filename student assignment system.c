#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 300

// ===== COLORS =====
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// ===== STRUCTURES =====
struct Room {
    int id, capacity, remaining, type;
};

struct Group {
    int id, size, vip, allocatedRoom;
};

// ===== GLOBAL =====
struct Room rooms[MAX];
struct Group groups[MAX];

int roomCount = 7;
int groupCount = 0;
int nextGroupID = 1;

// ===== UI =====
void printLine() {
    printf(CYAN "========================================\n" RESET);
}

void printBoxTitle(char title[]) {
    printLine();
    printf(BOLD BLUE "|| %-36s ||\n" RESET, title);
    printLine();
}

void printSuccess(char msg[]) {
    printf(GREEN ">> %s\n" RESET, msg);
}

void printError(char msg[]) {
    printf(RED ">> %s\n" RESET, msg);
}

// ===== SAFE INPUT (FINAL FIX) =====
int safePositive() {
    int x;
    char input[100];

    while (1) {
        printf(">> Enter a valid positive number: ");

        if (fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = '\0';

            if (input[0] == '\0') continue;

            if (sscanf(input, "%d", &x) == 1 && x > 0) {
                return x;
            }
        }

        printError("Invalid input!");
    }
}

int safeZeroOne() {
    int x;
    char input[100];

    while (1) {
        printf("Enter 1 for VIP, 0 for Normal: ");

        if (fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = '\0';

            if (sscanf(input, "%d", &x) == 1 && (x == 0 || x == 1)) {
                return x;
            }
        }

        printError("Only 1 or 0 allowed!");
    }
}

// ===== LOAD ROOMS =====
void loadRooms() {
    rooms[0]=(struct Room){101,10,10,1};
    rooms[1]=(struct Room){102,8,8,2};
    rooms[2]=(struct Room){103,5,5,1};
    rooms[3]=(struct Room){104,12,12,1};
    rooms[4]=(struct Room){105,6,6,2};
    rooms[5]=(struct Room){106,15,15,1};
    rooms[6]=(struct Room){107,20,20,2};
}

// ===== DISPLAY =====
void showRooms() {
    printBoxTitle("ROOM LIST");

    for(int i=0;i<roomCount;i++) {
        printf("|| Room %-3d | Cap:%-3d | Rem:%-3d | ",
               rooms[i].id, rooms[i].capacity, rooms[i].remaining);

        if(rooms[i].type==2)
            printf(YELLOW "VIP" RESET "   ||\n");
        else
            printf(BLUE "Normal" RESET "||\n");
    }
    printLine();
}

void showAvailableRooms() {
    printBoxTitle("AVAILABLE ROOMS");

    for(int i=0;i<roomCount;i++)
        if(rooms[i].remaining>0)
            printf("|| Room %-3d -> %-3d free            ||\n",
                   rooms[i].id, rooms[i].remaining);

    printLine();
}

// ===== ADD GROUP =====
void addGroup() {
    if(groupCount>=MAX) {
        printError("Limit reached");
        return;
    }

    groups[groupCount].id = nextGroupID++;

    printf("\nGroup ID %d\n", groups[groupCount].id);
    groups[groupCount].size = safePositive();

    groups[groupCount].vip = safeZeroOne();
    groups[groupCount].allocatedRoom = -1;

    groupCount++;
}

// ===== SEARCH =====
int findGroup(int id) {
    for(int i=0;i<groupCount;i++)
        if(groups[i].id==id) return i;
    return -1;
}

void searchGroup() {
    printf("Enter Group ID: ");
    int id = safePositive();

    int i = findGroup(id);

    if(i==-1)
        printError("Not found");
    else {
        printBoxTitle("GROUP INFO");
        printf("|| ID:%d Size:%d %s Room:%d        ||\n",
               groups[i].id, groups[i].size,
               groups[i].vip?"VIP":"Normal",
               groups[i].allocatedRoom);
        printLine();
    }
}

// ===== SORT =====
void sortVIP() {
    for(int i=0;i<groupCount-1;i++)
        for(int j=i+1;j<groupCount;j++)
            if(groups[j].vip > groups[i].vip) {
                struct Group t=groups[i];
                groups[i]=groups[j];
                groups[j]=t;
            }
    printSuccess("Sorted by VIP");
}

void sortSize() {
    for(int i=0;i<groupCount-1;i++)
        for(int j=i+1;j<groupCount;j++)
            if(groups[j].size > groups[i].size) {
                struct Group t=groups[i];
                groups[i]=groups[j];
                groups[j]=t;
            }
    printSuccess("Sorted by Size");
}

// ===== RESET =====
void reset() {
    for(int i=0;i<roomCount;i++)
        rooms[i].remaining = rooms[i].capacity;

    for(int i=0;i<groupCount;i++)
        groups[i].allocatedRoom = -1;
}

// ===== FIRST FIT =====
void firstFit() {
    reset();

    for(int i=0;i<groupCount;i++)
        for(int j=0;j<roomCount;j++) {
            if(groups[i].vip && rooms[j].type!=2) continue;

            if(rooms[j].remaining>=groups[i].size) {
                groups[i].allocatedRoom=rooms[j].id;
                rooms[j].remaining-=groups[i].size;
                break;
            }
        }

    printSuccess("First Fit Done");
}

// ===== BEST FIT =====
void bestFit() {
    reset();

    for(int i=0;i<groupCount;i++) {
        int best=-1;

        for(int j=0;j<roomCount;j++) {
            if(groups[i].vip && rooms[j].type!=2) continue;

            if(rooms[j].remaining>=groups[i].size) {
                if(best==-1 || rooms[j].remaining<rooms[best].remaining)
                    best=j;
            }
        }

        if(best!=-1) {
            groups[i].allocatedRoom=rooms[best].id;
            rooms[best].remaining-=groups[i].size;
        }
    }

    printSuccess("Best Fit Done");
}

// ===== WORST FIT =====
void worstFit() {
    reset();

    for(int i=0;i<groupCount;i++) {
        int worst=-1;

        for(int j=0;j<roomCount;j++) {
            if(groups[i].vip && rooms[j].type!=2) continue;

            if(rooms[j].remaining>=groups[i].size) {
                if(worst==-1 || rooms[j].remaining>rooms[worst].remaining)
                    worst=j;
            }
        }

        if(worst!=-1) {
            groups[i].allocatedRoom=rooms[worst].id;
            rooms[worst].remaining-=groups[i].size;
        }
    }

    printSuccess("Worst Fit Done");
}

// ===== RESULT =====
void showResult() {
    printBoxTitle("ALLOCATION RESULT");

    for(int i=0;i<groupCount;i++) {
        if(groups[i].allocatedRoom!=-1)
            printf(GREEN "|| G%d -> R%d                     ||\n" RESET,
                   groups[i].id, groups[i].allocatedRoom);
        else
            printf(RED "|| G%d -> Not Placed              ||\n" RESET,
                   groups[i].id);
    }

    printLine();
}

// ===== MAIN =====
int main() {
    int ch;
    loadRooms();

    while(1) {
        printLine();
        printf(BOLD YELLOW "||      ROOM ALLOCATION SYSTEM        ||\n" RESET);
        printLine();

        printf("|| 1. View Rooms                      ||\n");
        printf("|| 2. Available Rooms                 ||\n");
        printf("|| 3. Add Group                       ||\n");
        printf("|| 4. Search Group                    ||\n");
        printf("|| 5. First Fit                       ||\n");
        printf("|| 6. Best Fit                        ||\n");
        printf("|| 7. Worst Fit                       ||\n");
        printf("|| 8. Sort VIP                        ||\n");
        printf("|| 9. Sort Size                       ||\n");
        printf("|| 10. Show Result                    ||\n");
        printf("|| 11. Exit                           ||\n");

        printLine();
        printf("Enter Choice: ");

        ch = safePositive();

        switch(ch) {
            case 1: showRooms(); break;
            case 2: showAvailableRooms(); break;
            case 3: addGroup(); break;
            case 4: searchGroup(); break;
            case 5: firstFit(); break;
            case 6: bestFit(); break;
            case 7: worstFit(); break;
            case 8: sortVIP(); break;
            case 9: sortSize(); break;
            case 10: showResult(); break;
            case 11: exit(0);
            default: printError("Invalid choice");
        }
}
