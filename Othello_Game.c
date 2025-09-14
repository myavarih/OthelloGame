#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define SIZE 8
#define GAMESCOUNT 2000


// Finish fast:
// 3 2
// 2 4
// 3 5
// 4 2
// 1 3
// 2 2
// 3 1
// 2 3
// 5 3

enum GameMode
{
    NOT_TIMED,
    TIMED
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct GameInfo
{
    int ID;
    char status[20];
    char user1[50];
    char user2[50];
    enum GameMode mode;
    char board[SIZE][SIZE];
    int Xseconds, Oseconds, Xminutes, Ominutes;
    int blackScores[50], whiteScores[50], blackScoresSize, whiteScoresSize;
    char player;
} GamesInfo[GAMESCOUNT];

struct user
{
    char username[50];
    int score;
} users[300];

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Global variables
int blackCount,
    whiteCount;
int blackScoresSize = 0, whiteScoresSize = 0;
enum GameMode gameMode;
char firstUsername[50] = "", secondUsername[50] = "", oneTimeInput[20];
char board[SIZE][SIZE];
char board_copyX[SIZE][SIZE];
char board_copyO[SIZE][SIZE];
int Xminutes = 31, Xseconds = 0;
int Ominutes = 10, Oseconds = 0;
int blackScores[50] = {0}, whiteScores[50] = {0};
int XbackCounter = 0;
int ObackCounter = 0;
char player;
int ID_Holder = -1;

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function prototypes
void typePrint(char text[]);
void SaveAndMenu(char status[]);
void menu();
void initializeBoard();
void printBoard();
bool isValidMove(int row, int col);
bool isAnyValidMove();
void makeMove(int row, int col);
bool isGameOver();
void countDiscs();
void calculateScore();
void TheGame();
bool isTimeLeft(time_t t, struct tm tm, int nowTime);
void ZeroScores();
void bblSort(struct user users[], int size);
void Encryption();
void Decryption();
void printScores();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// custom function to print with typing effect
void typePrint(char text[])
{
    for (int i = 0; i < strlen(text); i++)
    {
        printf("\033[1;34m%c\033[0;37m", text[i]);
        fflush(stdout);
        usleep(20000);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// called when player wants to exit to menu
void SaveAndMenu(char status[])
{
    Decryption();
    // Saving Ended Games:
    if (!strcmp("Finished", status))
    {
        FILE *IDHolder = fopen("IDHolder.txt", "r+");
        if (ID_Holder == -1)
        {

            fscanf(IDHolder, "%d", &ID_Holder);
            fclose(IDHolder);
            FILE *IDHolder = fopen("IDHolder.txt", "w+");
            fprintf(IDHolder, "%d", ID_Holder + 1);
        }
        fclose(IDHolder);
        FILE *pastGames = fopen("past_games.txt", "w+");

        strcpy(GamesInfo[ID_Holder].status, status);
        strcpy(GamesInfo[ID_Holder].user1, firstUsername);
        strcpy(GamesInfo[ID_Holder].user2, secondUsername);
        GamesInfo[ID_Holder].mode = gameMode;
        memcpy(GamesInfo[ID_Holder].board, board, sizeof(board));
        GamesInfo[ID_Holder].Xseconds = Xseconds;
        GamesInfo[ID_Holder].Xminutes = Xminutes;
        GamesInfo[ID_Holder].Oseconds = Oseconds;
        GamesInfo[ID_Holder].Ominutes = Ominutes;
        GamesInfo[ID_Holder].blackScoresSize = blackScoresSize;
        memcpy(GamesInfo[ID_Holder].blackScores, blackScores, sizeof(blackScores));
        GamesInfo[ID_Holder].whiteScoresSize = whiteScoresSize;
        memcpy(GamesInfo[ID_Holder].whiteScores, whiteScores, sizeof(whiteScores));
        GamesInfo[ID_Holder].player = player;

        fwrite(GamesInfo, ID_Holder + 1, sizeof(struct GameInfo), pastGames);

        fclose(pastGames);
        Encryption();
        menu();
    }

    // Saving Unfinished Games:
    while (true)
    {
        typePrint("Do you want to Save The Game? (y/n)\n");
        char save;
        scanf("\n%c", &save);

        if (save == 'y')
        {
            FILE *IDHolder = fopen("IDHolder.txt", "r+");
            if (ID_Holder == -1) // Not raed from file yet:
            {

                fscanf(IDHolder, "%d", &ID_Holder);
                fclose(IDHolder);
                FILE *IDHolder = fopen("IDHolder.txt", "w+");
                fprintf(IDHolder, "%d", ID_Holder + 1);
            }
            fclose(IDHolder);
            FILE *pastGames = fopen("past_games.txt", "w+");

            // Save to The struct Array:
            strcpy(GamesInfo[ID_Holder].status, status);
            strcpy(GamesInfo[ID_Holder].user1, firstUsername);
            strcpy(GamesInfo[ID_Holder].user2, secondUsername);
            GamesInfo[ID_Holder].mode = gameMode;
            memcpy(GamesInfo[ID_Holder].board, board, sizeof(board));
            GamesInfo[ID_Holder].Xseconds = Xseconds;
            GamesInfo[ID_Holder].Xminutes = Xminutes;
            GamesInfo[ID_Holder].Oseconds = Oseconds;
            GamesInfo[ID_Holder].Ominutes = Ominutes;
            GamesInfo[ID_Holder].blackScoresSize = blackScoresSize;
            memcpy(GamesInfo[ID_Holder].blackScores, blackScores, sizeof(blackScores));
            GamesInfo[ID_Holder].whiteScoresSize = whiteScoresSize;
            memcpy(GamesInfo[ID_Holder].whiteScores, whiteScores, sizeof(whiteScores));
            GamesInfo[ID_Holder].player = player;

            fwrite(GamesInfo, ID_Holder + 1, sizeof(struct GameInfo), pastGames);
            fclose(pastGames);

            Encryption();
            menu();
        }
        else if (save == 'n')
        {
            Encryption();
            menu();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// The Main Menu of The Game:
void menu()
{
    // system("cls");
    printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
    typePrint("Welcome to Othello!\n");
    typePrint("Input player one's username (X): ");
    scanf("%s", firstUsername);
    typePrint("Input player two's username (O): ");
    scanf("%s", secondUsername);

    // search for unfinished games:
    Decryption();
    FILE *pastGames = fopen("past_games.txt", "r+");
    FILE *IDHolder = fopen("IDHolder.txt", "r+");
    if (ID_Holder == -1)
    {

        fscanf(IDHolder, "%d", &ID_Holder);
        fclose(IDHolder);
    }
    else
        fclose(IDHolder);
    fread(GamesInfo, ID_Holder + 1, sizeof(struct GameInfo), pastGames);
    fclose(pastGames);
    Encryption();
    bool wasThereAGame = false;
    for (int i = 0; i < ID_Holder + 1; i++)
    {
        if (!strcmp(GamesInfo[i].status, "Unfinished") && !strcmp(GamesInfo[i].user1, firstUsername) && !strcmp(GamesInfo[i].user2, secondUsername))
        {
            wasThereAGame = true;
            char thisgamemode[20];
            if (GamesInfo[i].mode)
                strcpy(thisgamemode, "Timed");
            else
                strcpy(thisgamemode, "Not Timed");
            printf("ID: %d | Mode: %s | X's Turns: %d | O's Turns: %d\n", i, thisgamemode, GamesInfo[i].blackScoresSize, GamesInfo[i].whiteScoresSize);
        }
    }
    ID_Holder = -1;
    if (wasThereAGame)
        typePrint("Enter the Game ID to Resum it\n");
    typePrint("new game? (n)\n");
    typePrint("exit? (e)\n");
    typePrint("print Scores? (s)\n");
    while (true)
    {
        scanf("\n%s", oneTimeInput);
        int command = -1;
        if (isdigit(oneTimeInput[0]))
            command = atoi(oneTimeInput);
        if (oneTimeInput[0] == 'n')
        {
            typePrint("Input Game Mode: \nTimed (t)\nNot Timed (n)\n");
            while (true)
            {
                scanf("\n%s", oneTimeInput);

                if (oneTimeInput[0] == 't')
                {
                    gameMode = TIMED;
                    typePrint("Enter the Game Time in minutes: ");
                    while(Xminutes > 30){
                    scanf("%d", &Xminutes);
                    if(Xminutes > 30)
                        typePrint("Please enter Less than (or equal to) 30 minutes: ");
                    }
                    Ominutes = Xminutes;
                    TheGame();
                }
                else if (oneTimeInput[0] == 'n')
                {
                    gameMode = NOT_TIMED;
                    TheGame();
                }
            }
        }
        else if (oneTimeInput[0] == 'e')
        {
            exit(0);
        }
        else if (command != -1)
        {
            ID_Holder = command;
            strcpy(firstUsername, GamesInfo[ID_Holder].user1);
            strcpy(secondUsername, GamesInfo[ID_Holder].user2);
            gameMode = GamesInfo[ID_Holder].mode;
            memcpy(board, GamesInfo[ID_Holder].board, sizeof(board));
            Xseconds = GamesInfo[ID_Holder].Xseconds;
            Xminutes = GamesInfo[ID_Holder].Xminutes;
            Oseconds = GamesInfo[ID_Holder].Oseconds;
            Ominutes = GamesInfo[ID_Holder].Ominutes;
            blackScoresSize = GamesInfo[ID_Holder].blackScoresSize;
            memcpy(blackScores, GamesInfo[ID_Holder].blackScores, sizeof(blackScores));
            whiteScoresSize = GamesInfo[ID_Holder].whiteScoresSize;
            memcpy(whiteScores, GamesInfo[ID_Holder].whiteScores, sizeof(whiteScores));
            player = GamesInfo[ID_Holder].player;
            TheGame();
        }
        else if (oneTimeInput[0] = 's')
        {
            printScores();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to initialize the board
void initializeBoard()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            board[i][j] = ' ';
        }
    }
    board[SIZE / 2 - 1][SIZE / 2 - 1] = 'O';
    board[SIZE / 2 - 1][SIZE / 2] = 'X';
    board[SIZE / 2][SIZE / 2 - 1] = 'X';
    board[SIZE / 2][SIZE / 2] = 'O';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to print the board
void printBoard()
{
    printf("    ");
    for (int i = 0; i < SIZE; i++)
    {
        printf("\033[0;36m%2d  ", i);
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++)
    {
        printf("\033[0;33m   +");
        for (int j = 0; j < SIZE; j++)
        {
            printf("---+");
        }
        printf("\n");

        printf("\033[0;36m%2d \033[0;33m|", i);
        for (int j = 0; j < SIZE; j++)
        {
            printf(" \033[0;37m%c \033[0;33m|", board[i][j]);
        }
        printf("\n");
    }

    printf("\033[0;33m   +");
    for (int i = 0; i < SIZE; i++)
    {
        printf("---+");
    }
    printf("\n\n\033[0;37m");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to check if a move is valid
bool isValidMove(int row, int col)
{
    if (board[row][col] != ' ')
    {
        return false;
    }

    // ofsets:
    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; i++)
    {
        int r = row + dr[i];
        int c = col + dc[i];

        if (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] != ' ' && board[r][c] != player)
        {
            while (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] != ' ' && board[r][c] != player)
            {
                r += dr[i];
                c += dc[i];
            }

            if (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] == player)
            {
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to check if the player has any valid moves
bool isAnyValidMove()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (isValidMove(i, j))
            {
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to make a move
void makeMove(int row, int col)
{
    board[row][col] = player;

    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; i++)
    {
        int r = row + dr[i];
        int c = col + dc[i];

        if (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] != ' ' && board[r][c] != player)
        {
            while (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] != ' ' && board[r][c] != player)
            {
                r += dr[i];
                c += dc[i];
            }

            if (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] == player)
            {
                while (r != row || c != col)
                {
                    r -= dr[i];
                    c -= dc[i];
                    board[r][c] = player;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to check if the game is over
bool isGameOver()
{
    if (isAnyValidMove())
        return false;
    player = (player == 'X') ? 'O' : 'X';
    if (isAnyValidMove())
    {
        player = (player == 'X') ? 'O' : 'X';
        return false;
    }
    player = (player == 'X') ? 'O' : 'X';
    return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to count the number of discs for each player
void countDiscs()
{
    blackCount = 0;
    whiteCount = 0;

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 'X')
            {
                blackCount++;
            }
            else if (board[i][j] == 'O')
            {
                whiteCount++;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to Calculate the score
void calculateScore()
{
    int stage1, stage2;
    if (player == 'X')
    {
        stage1 = blackCount;
        countDiscs();
        stage2 = blackCount;

        blackScores[blackScoresSize++] = stage2 - stage1 - 1;
    }
    else if (player == 'O')
    {
        stage1 = whiteCount;
        countDiscs();
        stage2 = whiteCount;

        whiteScores[whiteScoresSize++] = stage2 - stage1 - 1;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to check if there's time left
bool isTimeLeft(time_t t, struct tm tm, int nowTime)
{
    t = time(NULL);
    tm = *localtime(&t);
    if (player == 'X')
    {

        if (Xminutes * 60 + Xseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime) < 0)
            return false;

        return true;
    }
    else if (player == 'O')
    {
        if (Ominutes * 60 + Oseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime) < 0)
            return false;

        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to set losing player's scores to zero on timout
void ZeroScores()
{
    if (player == 'X')
    {
        for (int i = 0; i < blackScoresSize; i++)
        {
            blackScores[i] = 0;
        }
    }
    else if (player == 'O')
    {
        for (int i = 0; i < whiteScoresSize; i++)
        {
            whiteScores[i] = 0;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to Sort the scores
void bblSort(struct user users[], int size)
{
    struct user hold;
    for (int pass = 0; pass < size - 1; pass++)
    {
        for (int i = 0; i < size - pass - 1; i++)
        {
            if (users[i].score > users[i + 1].score)
            {
                hold = users[i];
                users[i] = users[i + 1];
                users[i + 1] = hold;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to Encrypt files
void Encryption()
{
    // past_games.txt:
    int i = 0;
    char *buffer1 = malloc(2480 * 1000 * sizeof(char));
    FILE *File1 = fopen("past_games.txt", "r+");
    char c = fgetc(File1);
    while (c != EOF)
    {
        if (c != 0)
        {
            buffer1[i] = (c * 5 + 1) % 128;
        }
        else
        {
            buffer1[i] = c;
        }
        i++;
        c = fgetc(File1);
    }
    fclose(File1);
    FILE *File2 = fopen("past_games.txt", "w+");
    fwrite(buffer1, i, sizeof(char), File2);
    fclose(File2);
    free(buffer1);

    // IDHolder.txt:
    FILE *File3 = fopen("IDHolder.txt", "r+");
    c = fgetc(File3);
    c = (c * 5 + 1) % 128;
    fclose(File3);
    FILE *File4 = fopen("IDHolder.txt", "w+");
    fprintf(File4, "%c", c);
    fclose(File4);

    // userscors.txt:
    FILE *File5 = fopen("userscores.txt", "r+");
    i = 0;
    char *buffer2 = malloc(300 * 600 * sizeof(char));
    c = fgetc(File5);
    while (c != EOF)
    {
        if (c != 0)
        {
            buffer2[i] = (c * 5 + 1) % 128;
        }
        else
        {
            buffer2[i] = c;
        }
        i++;
        c = fgetc(File1);
    }
    fclose(File5);
    FILE *File6 = fopen("userscores.txt", "w+");
    fwrite(buffer2, i, sizeof(char), File6);
    fclose(File6);
    free(buffer2);

    // usercount.txt:
    FILE *File7 = fopen("usercount.txt", "r+");
    c = fgetc(File7);
    c = (c * 5 + 1) % 128;
    fclose(File7);
    FILE *File8 = fopen("usercount.txt", "w+");
    fprintf(File8, "%c", c);
    fclose(File8);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to Decrypt Files
void Decryption()
{
    // past_games.txt:
    int i = 0;
    char *buffer1 = malloc(2480 * 1000 * sizeof(char));
    FILE *File1 = fopen("past_games.txt", "r+");
    char c = fgetc(File1);
    while (c != EOF)
    {
        if (c != 0)
        {
            buffer1[i] = (77 * c + 51) % 128;
        }
        else
        {
            buffer1[i] = c;
        }
        i++;
        c = fgetc(File1);
    }
    fclose(File1);
    FILE *File2 = fopen("past_games.txt", "w+");
    fwrite(buffer1, i, sizeof(char), File2);
    fclose(File2);
    free(buffer1);

    // IDHolder.txt:
    FILE *File3 = fopen("IDHolder.txt", "r+");
    c = fgetc(File3);
    c = (77 * c + 51) % 128;
    fclose(File3);
    FILE *File4 = fopen("IDHolder.txt", "w+");
    fprintf(File4, "%c", c);
    fclose(File4);

    // userscores.txt:
    FILE *File5 = fopen("userscores.txt", "r+");
    i = 0;
    char *buffer2 = malloc(300 * 600 * sizeof(char));
    c = fgetc(File5);
    while (c != EOF)
    {
        if (c != 0)
        {
            buffer2[i] = (c * 77 + 51) % 128;
        }
        else
        {
            buffer2[i] = c;
        }
        i++;
        c = fgetc(File1);
    }
    fclose(File5);
    FILE *File6 = fopen("userscores.txt", "w+");
    fwrite(buffer2, i, sizeof(char), File6);
    fclose(File6);
    free(buffer2);

    // usercount.txt:
    FILE *File7 = fopen("usercount.txt", "r+");
    c = fgetc(File7);
    c = (c * 77 + 51) % 128;
    fclose(File7);
    FILE *File8 = fopen("usercount.txt", "w+");
    fprintf(File8, "%c", c);
    fclose(File8);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to Save user's scores
void SaveUserScore()
{
    Decryption();
    // first:
    FILE *UserCount = fopen("usercount.txt", "r");
    int users_count;
    fscanf(UserCount, "%d", &users_count);
    fclose(UserCount);
    int firstScore = 0;
    for (int i = 0; i < blackScoresSize; i++)
        firstScore += blackScores[i];
    FILE *UserScores = fopen("userscores.txt", "r");
    fread(users, sizeof(struct user), users_count, UserScores);
    int found = 0;
    for (int i = 0; i < users_count; i++)
    {
        if (!strcmp(users[i].username, firstUsername))
        {
            users[i].score += firstScore;
            found++;
            break;
        }
    }
    if (!found)
    {
        users[users_count].score = firstScore;
        strcpy(users[users_count++].username, firstUsername);
    }
    fclose(UserScores);

    // second:
    int secondScore = 0;
    for (int i = 0; i < whiteScoresSize; i++)
        secondScore += whiteScores[i];
    found = 0;
    for (int i = 0; i < users_count; i++)
    {
        if (!strcmp(users[i].username, secondUsername))
        {
            users[i].score += secondScore;
            found++;
            break;
        }
    }
    if (!found)
    {
        users[users_count].score = secondScore;
        strcpy(users[users_count++].username, secondUsername);
    }

    // replace:
    FILE *UserScores2 = fopen("userscores.txt", "w");
    fwrite(users, sizeof(struct user), users_count, UserScores2);
    fclose(UserScores2);

    FILE *UserCount2 = fopen("usercount.txt", "w");
    fprintf(UserCount2, "%d", users_count);
    fclose(UserCount2);
    Encryption();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to print the scores
void printScores()
{
    Decryption();
    FILE *scores = fopen("userscores.txt", "r");
    FILE *Count = fopen("usercount.txt", "r");
    int users_count;
    fscanf(Count, "%d", &users_count);
    fread(users, sizeof(struct user), users_count, scores);
    bblSort(users, users_count);
    for (int i = 0; i < users_count; i++)
    {
        printf("%s: %d\n", users[i].username, users[i].score);
    }
    Encryption();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Function to play the game
void TheGame()
{
    // refresh time:
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int nowTime = tm.tm_min * 60 + tm.tm_sec;
    int XTurnTimeSec = tm.tm_min * 60 + tm.tm_sec, OTurnTimeSec = tm.tm_min * 60 + tm.tm_sec;
    int OTTime, XTTime;
    int row, col;
    char oneTimeInput;
    bool refreshed = false;

    // system("cls");
    printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
    if (ID_Holder == -1)    // game not loaded
    {
        player = 'X';
        initializeBoard();
    }
    // srand(time(0)); // for fast finishing
    while (!isGameOver())
    {
        if (gameMode == TIMED)
        {
            t = time(NULL);
            tm = *localtime(&t);
            if (refreshed)
            {
                if (player == 'X')
                {
                    Xseconds = Xminutes * 60 + Xseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                    Xminutes = Xseconds / 60;
                    Xseconds = Xseconds % 60;
                }
                else if (player == 'O')
                {
                    Oseconds = Ominutes * 60 + Oseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                    Ominutes = Oseconds / 60;
                    Oseconds = Oseconds % 60;
                }
                printf("%-50s%2d:%02d\n", "Player X's Time Remaining: ", Xminutes, Xseconds);
                printf("%-50s%2d:%02d\n", "Player O's Time Remaining: ", Ominutes, Oseconds);
                nowTime = tm.tm_min * 60 + tm.tm_sec;
                refreshed = false;
            }
            else
            {
                if (player == 'X')
                {
                    Oseconds = Ominutes * 60 + Oseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                    Ominutes = Oseconds / 60;
                    Oseconds = Oseconds % 60;

                    OTTime = tm.tm_min * 60 + tm.tm_sec - OTurnTimeSec;
                    XTurnTimeSec = tm.tm_min * 60 + tm.tm_sec;
                }
                else if (player == 'O')
                {
                    Xseconds = Xminutes * 60 + Xseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                    Xminutes = Xseconds / 60;
                    Xseconds = Xseconds % 60;

                    XTTime = tm.tm_min * 60 + tm.tm_sec - XTurnTimeSec;
                    OTurnTimeSec = tm.tm_min * 60 + tm.tm_sec;
                }

                if (Xseconds < 0)
                {
                    Xseconds = Xminutes * 60 + Xseconds;
                    Xminutes = Xseconds / 60;
                    Xseconds = Xseconds % 60;
                }
                if (Oseconds < 0)
                {
                    Oseconds = Ominutes * 60 + Oseconds;
                    Ominutes = Oseconds / 60;
                    Oseconds = Oseconds % 60;
                }
                printf("%-50s%2d:%02d\n", "Player X's Time Remaining: ", Xminutes, Xseconds);
                printf("%-50s%2d:%02d\n", "Player O's Time Remaining: ", Ominutes, Oseconds);
                nowTime = tm.tm_min * 60 + tm.tm_sec;
            }
        }
        printf("To Exit to the menu (Save Or Abort), Enter -1 -1\n");
        if (gameMode == TIMED)
            printf("Player X: To Undo a Move Enter -1 -2\nPlayer O: To Undo a Move Enter -1 -3\nTo Refresh Remaning Time Enter -2 -2\n");
        printBoard();
        countDiscs();
        printf("Black: %d  White: %d\n", blackCount, whiteCount);
        if (!isAnyValidMove())
            player = (player == 'X') ? 'O' : 'X';
        printf("Player %c's turn. Enter row and column (e.g., 2 3): ", player);

        scanf("%d %d", &row, &col);
        // row = rand() % 8; // for fast finishing
        // col = rand() % 8; // for fast finishing
        if (!isTimeLeft(t, tm, nowTime))
        {
            typePrint("your time is up\nyou lose :(\n");
            if (player == 'X')
                typePrint("player O Won\n");
            else if (player == 'O')
                typePrint("player X Won\n");

            ZeroScores();
            typePrint("To Display The Scores Enter (s) or anything else otherwise\n");
            scanf("\n%c", &oneTimeInput);
            if (oneTimeInput == 's')
            {
                printScores();
            }
            typePrint("\nEnter Anything To Return To Main Menu \n");
            scanf("\n%c", &oneTimeInput);
            SaveAndMenu("Finished");
        }
        if (row == -1)
        {
            if (col == -1)
            {
                if (gameMode == TIMED)
                {
                    t = time(NULL);
                    tm = *localtime(&t);
                    if (player == 'X')
                    {
                        Xseconds = Xminutes * 60 + Xseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                        Xminutes = Xseconds / 60;
                        Xseconds = Xseconds % 60;
                    }
                    else if (player == 'O')
                    {
                        Oseconds = Ominutes * 60 + Oseconds - (tm.tm_min * 60 + tm.tm_sec - nowTime);
                        Ominutes = Oseconds / 60;
                        Oseconds = Oseconds % 60;
                    }
                }
                SaveAndMenu("Unfinished");
            }
            else if (col == -2 && gameMode == TIMED && XbackCounter < 2)
            {
                memcpy(board, board_copyX, sizeof(board));
                blackScores[--blackScoresSize] = 0;
                if (player == 'X')
                    whiteScores[--whiteScoresSize] = 0;
                if (XbackCounter == 0)
                {
                    Xseconds -= 30;
                    XbackCounter++;
                }
                else if (XbackCounter == 1)
                {
                    Xseconds -= 60;
                    XbackCounter++;
                    t = time(NULL);
                    tm = *localtime(&t);
                    if (player == 'X')
                        Oseconds += OTTime + XTTime;
                    else
                        Oseconds += XTTime;
                }
                // system("cls");
                printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
                player = 'X';
                continue;
            }
            else if (col == -3 && gameMode == TIMED && ObackCounter < 2)
            {
                memcpy(board, board_copyO, sizeof(board));
                whiteScores[--whiteScoresSize] = 0;
                if (player == 'O')
                    blackScores[--blackScoresSize] = 0;
                if (ObackCounter == 0)
                {
                    Oseconds -= 30;
                    ObackCounter++;
                }
                else if (ObackCounter == 1)
                {
                    Oseconds -= 60;
                    ObackCounter++;
                    t = time(NULL);
                    tm = *localtime(&t);
                    if (player == 'O')
                        Xseconds += OTTime + XTTime;
                    else
                        Xseconds += OTTime;
                }
                // system("cls");
                printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
                player = 'O';
                continue;
            }
        }
        else if (row == -2 && col == -2 && gameMode == TIMED)
        {
            // system("cls");
            printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
            refreshed = true;
            continue;
        }
        // system("cls");
        printf("\e[1;1H\e[2J"); // Same as Above (To clear the terminal)
        if (isValidMove(row, col))
        {
            if (gameMode == TIMED)
            {
                if (player == 'X')
                    memcpy(board_copyX, board, sizeof(board_copyX));
                if (player == 'O')
                    memcpy(board_copyO, board, sizeof(board_copyO));
            }
            makeMove(row, col);
            calculateScore();
            player = (player == 'X') ? 'O' : 'X';
        }
        else
        {
            printf("Invalid move. Try again.\n");
        }
    }
    printBoard();
    countDiscs();
    printf("Black: %d  White: %d\n", blackCount, whiteCount);

    if (blackCount > whiteCount)
    {
        printf("Black wins!\n");
    }
    else if (blackCount < whiteCount)
    {
        printf("White wins!\n");
    }
    else
    {
        printf("It's a tie!\n\n\n");
    }
    SaveUserScore();
    typePrint("To Display The Scores Enter (s) or anything else otherwise\n");
    scanf("\n%c", &oneTimeInput);
    if (oneTimeInput == 's')
    {
        printScores();
    }
    typePrint("\nEnter Anything To Return To Main Menu \n");
    scanf("\n%c", &oneTimeInput);
    SaveAndMenu("Finished");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Just main!
int main()
{
    menu();
    return 0;
}