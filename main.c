#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sqlite3.h>

#include "sha256.h"

struct BankUser
{
    uint64_t Id;
    char Name[256];
    char CartID[256];
    int64_t Money;
};

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

#define EXIT 3
#define NOT_FOUND_USER 4

#define DATABASE_ERROR 5
#define CANT_MAKE_DATA 6
#define CANT_OPEN_DATABASE 7

#define NOT_ENOUGH_MONEY 8
#define VALUE_IS_NOT_VALID 9
#define ERROR_SELF_SEND_FOR_SELF 10

#define FEE 2500

sqlite3 *db;

// init database for first time run program
static int initDB();

// insert temp data in database if database is empty!
static int insertData(sqlite3 *db);

// setup login user
// get cardId and password
// check on database
// and if exist setup value to ( user )
static int makeLoginUser(struct BankUser *user);

// get money from atm
static int getMoney(struct BankUser *user);

// send money for another with card id
static int sendMoney(struct BankUser *user);

// show info account
void accountInfo(struct BankUser *user);

// get string from stdin with size of char
static int getLine(char *prmpt, char *buff, size_t sz);

// safe close sqlite data base if get signal attention
void safeClose(int _);

int main()
{
    if (initDB())
        return 1;

    signal(SIGINT, safeClose);
    for (;;)
    {
        struct BankUser user;
        int rc = makeLoginUser(&user);
        if (rc == EXIT)
        {
            break;
        }

        if (rc != OK)
        {
            continue;
        }

        for (;;)
        {
            int choose;
            fprintf(stderr, "Choose One Action!\n");
            fprintf(stderr, "1 - Take Money From ATM\n");
            fprintf(stderr, "2 - Show Info Account\n");
            fprintf(stderr, "3 - Send Money For Another\n");
            fprintf(stderr, "4 - Exit From ATM\n");
            fprintf(stderr, "Enter Number: ");
            scanf("%d", &choose);
            fprintf(stderr, "\033c");

            if (choose == 1)
            {
                getMoney(&user);
            }
            else if (choose == 2)
            {
                accountInfo(&user);
            }
            else if (choose == 3)
            {
                sendMoney(&user);
            }
            else if (choose == 4)
            {
                fprintf(stderr, "See You!");
                sleep(3);
                fprintf(stderr, "\033c");
                break;
            }
            else
            {
                fprintf(stderr, "Choose Not Valid!\n");
            }

            sleep(3);
            fprintf(stderr, "\033c");
        }
    }

    sqlite3_close(db);
    return OK;
}

static int initDB()
{
    sqlite3_stmt *res;
    fprintf(stderr, "Create or Open Database File!\n");
    int rc = sqlite3_open("./user.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return CANT_OPEN_DATABASE;
    }

    fprintf(stderr, "Check Database is not empty!\n");
    char *sql = "SELECT Name, Id FROM User WHERE Name='Alex';";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK || sqlite3_step(res) != SQLITE_ROW)
    {
        fprintf(stderr, "Database is empty!\n");
        rc = insertData(db);
    }

    fprintf(stderr, "init Database Done!\n");
    sleep(5);
    fprintf(stderr, "\033c");
    return rc;
}

static int insertData(sqlite3 *db)
{
    char *err_msg = 0;
    char *sql = "DROP TABLE IF EXISTS User;"
                "CREATE TABLE User(Id INT, Name TEXT, CardID TEXT, Password TEXT, Money INT);"
                "INSERT INTO User VALUES(1, 'Alex', '111111111', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(2, 'Musk', '111111112', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(3, 'Tusk', '111111113', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(4, 'Loser', '111111114', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(5, 'Better', '111111115', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(6, 'IronMan', '111111116', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(7, 'Man', '111111117', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);"
                "INSERT INTO User VALUES(8, 'WoLu', '111111118', '0ffe1abd1a08215353c233d6e009613e95eec4253832a761af28ff37ac5a150c', 10000000);";

    fprintf(stderr, "Create Data on Database!\n");
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return CANT_MAKE_DATA;
    }

    sqlite3_free(err_msg);
    return OK;
}

static int makeLoginUser(struct BankUser *user)
{
    char CardID[256];
    int rc = getLine("Enter Your CardID: ", CardID, sizeof CardID);
    if (rc != OK)
    {
        fprintf(stderr, "\033c");
        fprintf(stderr, "Pleas Enter CardId!");

        sleep(5);
        fprintf(stderr, "\033c");
        return rc;
    }

    if (!strcmp(CardID, "exit"))
    {
        return EXIT;
    }

    char *password = getpass("Password: ");
    char hashPass[SHA256_HEX_SIZE];
    sha256_hex(password, strlen(password), hashPass);

    char sql[512];
    sprintf(sql,
            "SELECT Id, Name, CardID, Money "
            "FROM User "
            "WHERE CardID = '%s' AND Password = '%s';",
            CardID, hashPass);

    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK ||
        sqlite3_step(res) != SQLITE_ROW)
    {
        fprintf(stderr, "\033c");
        fprintf(stderr, "CardId or Password Is Wrong!\n");

        sleep(5);
        fprintf(stderr, "\033c");
        return NOT_FOUND_USER;
    }

    user->Id = sqlite3_column_int(res, 0);
    sprintf(user->Name, "%s", sqlite3_column_text(res, 1));
    sprintf(user->CartID, "%s", sqlite3_column_text(res, 2));
    user->Money = sqlite3_column_int(res, 3);

    fprintf(stderr, "\033c");
    accountInfo(user);

    sleep(5);
    fprintf(stderr, "\033c");

    return OK;
}

static int getMoney(struct BankUser *user)
{
    int value = 0;
    fprintf(stderr, "Enter Value: ");
    int rc = scanf("%d", &value);
    if (1 != rc || value <= 0)
    {
        fprintf(stderr, "Value Is Not Valid!");
        return VALUE_IS_NOT_VALID;
    }

    if ((user->Money - (value + FEE)) < 0)
    {
        fprintf(stderr, "Not Enough Money!");
        return NOT_ENOUGH_MONEY;
    }

    user->Money -= (value + FEE);
    char sql[256];
    sprintf(sql, "UPDATE User SET Money = %ld WHERE Id = %ld;",
            user->Money, user->Id);

    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Database Error: %s", sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return DATABASE_ERROR;
    }

    fprintf(stderr, "Get Your Money (%d)\n"
                    "Fee Is (%d)\n"
                    "Your money now: %ld",
            value, FEE, user->Money);

    return OK;
}

static int sendMoney(struct BankUser *user)
{
    int value = 0;
    fprintf(stderr, "Enter Value: ");
    int rc = scanf("%d", &value);
    if (1 != rc || value <= 0)
    {
        fprintf(stderr, "Value Is Not Valid!");
        return VALUE_IS_NOT_VALID;
    }

    if ((user->Money - (value + FEE)) < 0)
    {
        fprintf(stderr, "Not Enough Money!");
        return NOT_ENOUGH_MONEY;
    }

    char CardID[256];
    rc = getLine("Enter CardID: ", CardID, sizeof CardID);
    if (rc != OK)
    {
        fprintf(stderr, "not valid Card Id!");
        return rc;
    }

    if (strcmp(CardID, user->CartID) == 0)
    {
        fprintf(stderr, "You Can't Send Money for your self!");
        return ERROR_SELF_SEND_FOR_SELF;
    }

    char sqlFindUser[512];
    sprintf(sqlFindUser,
            "SELECT Id, Money, Name "
            "FROM User "
            "WHERE CardID = '%s';",
            CardID);

    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db, sqlFindUser, -1, &res, 0);
    if (rc != SQLITE_OK ||
        sqlite3_step(res) != SQLITE_ROW)
    {
        fprintf(stderr, "Not Found User!\n");
        return NOT_FOUND_USER;
    }

    uint64_t idGetter = sqlite3_column_int(res, 0);
    uint64_t MoneyGetter = sqlite3_column_int(res, 1);
    char *nameGetter = (char *)sqlite3_column_text(res, 2);

    user->Money -= (value + FEE);
    MoneyGetter += value;
    char sql[512];
    sprintf(sql,
            "UPDATE User SET Money = %ld "
            "WHERE Id = %ld;"
            "UPDATE User SET Money = %ld "
            "WHERE Id = %ld;",
            user->Money, user->Id,
            MoneyGetter, idGetter);

    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Database Error: %s", sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return DATABASE_ERROR;
    }

    fprintf(stderr, "Send Money (%d)\n"
                    "For (%s)\n"
                    "Fee Is (%d)\n"
                    "Your money now: %ld",
            value, nameGetter, FEE, user->Money);

    return OK;
}

void accountInfo(struct BankUser *user)
{
    fprintf(stderr,
            "--- Account Info ---\n"
            "- NAME : %s\n"
            "- CartID : %s\n"
            "- Money : %ld",
            user->Name,
            user->CartID,
            user->Money);
}

static int getLine(char *prmpt, char *buff, size_t sz)
{
    int ch, extra;
    if (prmpt != NULL)
    {
        fprintf(stderr, prmpt);
        fflush(stdout);
    }

    if (fgets(buff, sz, stdin) == NULL)
        return NO_INPUT;

    if (buff[0] == '\n')
        fgets(buff, sz, stdin);

    if (buff[strlen(buff) - 1] != '\n')
    {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    buff[strlen(buff) - 1] = '\0';
    return OK;
}

void safeClose(int _)
{
    fprintf(stderr, "\n");
    sqlite3_close(db);
    exit(OK);
}
