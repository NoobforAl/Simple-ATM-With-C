#include "atm.h"

sqlite3 *db;

int initDB()
{
    fprintf(stderr, "Create or Open Database File!\n");
    if (sqlite3_open("./user.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return CANT_OPEN_DATABASE;
    }

    sqlite3_stmt *res;
    fprintf(stderr, "Check Database is not empty!\n");
    char *sql = "SELECT Name, Id FROM User WHERE Name='Alex';";
    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK || sqlite3_step(res) != SQLITE_ROW)
    {
        fprintf(stderr, "Database is empty!\n");
        if (insertData() != OK)
            return 1;
    }

    fprintf(stderr, "init Database Done!\n");
    sleep(3);
    fprintf(stderr, "\033c");
    return OK;
}

void closeDB(void)
{
    sqlite3_close(db);
}

int insertData()
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

int makeLoginUser(struct BankUser *user, char *CardID, char *password)
{

    char hashPass[SHA256_HEX_SIZE];
    sha256_hex(password, strlen(password), hashPass);

    char sql[256];
    sprintf(sql,
            "SELECT Id, Name, CardID, Money "
            "FROM User "
            "WHERE CardID = '%s' AND Password = '%s';",
            CardID, hashPass);

    sqlite3_stmt *res;
    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK ||
        sqlite3_step(res) != SQLITE_ROW)
        return NOT_FOUND_USER;

    user->Id = sqlite3_column_int64(res, 0);
    sprintf(user->Name, "%s", sqlite3_column_text(res, 1));
    sprintf(user->CartID, "%s", sqlite3_column_text(res, 2));
    user->Money = sqlite3_column_int64(res, 3);
    return OK;
}

int getMoney(struct BankUser *user, int64_t value)
{
    if (value <= 0)
        return VALUE_IS_NOT_VALID;

    if ((user->Money - (value + FEE)) < 0)
        return NOT_ENOUGH_MONEY;

    user->Money -= (value + FEE);
    char sql[256];
    sprintf(sql, "UPDATE User SET Money = %ld WHERE Id = %ld;",
            user->Money, user->Id);

    char *err_msg = 0;
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "Database Error: %s", sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return DATABASE_ERROR;
    }
    return OK;
}

int sendMoney(struct BankUser *user, char *CardID, int64_t value)
{
    if (value <= 0)
        return VALUE_IS_NOT_VALID;

    if ((user->Money - (value + FEE)) < 0)
        return NOT_ENOUGH_MONEY;

    if (strcmp(CardID, user->CartID) == 0)
        return ERROR_SELF_SEND_FOR_SELF;

    char sqlFindUser[512];
    sprintf(sqlFindUser,
            "SELECT Id, Money, Name "
            "FROM User "
            "WHERE CardID = '%s';",
            CardID);

    sqlite3_stmt *res;
    if (sqlite3_prepare_v2(db, sqlFindUser, -1, &res, 0) != SQLITE_OK ||
        sqlite3_step(res) != SQLITE_ROW)
        return NOT_FOUND_USER;

    uint64_t idGetter = sqlite3_column_int(res, 0);
    uint64_t MoneyGetter = sqlite3_column_int(res, 1);

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
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "Database Error: %s", sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return DATABASE_ERROR;
    }

    return OK;
}

int handelError(int code)
{
    switch (code)
    {
    case NOT_FOUND_USER:
        clearPrint("CardId or Password Is Wrong!\n");
        return code;

    case DATABASE_ERROR:
        clearPrint("");
        return code;

    case VALUE_IS_NOT_VALID:
        clearPrint("Value Is Not Valid!");
        return code;

    case NOT_ENOUGH_MONEY:
        clearPrint("Not Enough Money!");
        return code;

    case ERROR_SELF_SEND_FOR_SELF:
        clearPrint("You Can't Send Money for your self!");
        return code;

    case NOT_VALID_CARD_ID || NO_INPUT || TOO_LONG:
        clearPrint("Pleas Enter Valid CardId!");
        return code;

    default:
        return code;
    }
}

char *accountInfo(struct BankUser *user)
{
    char *info;
    info = (char *)malloc(1024);
    sprintf(info,
            "--- Account Info ---\n"
            "- NAME : %s\n"
            "- CartID : %s\n"
            "- Money : %ld",
            user->Name,
            user->CartID,
            user->Money);
    return info;
}

int getLine(char *prmpt, char *buff, size_t sz)
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

void clearPrint(char *t)
{
    fprintf(stderr, "\033c");
    fprintf(stderr, "%s", t);
    sleep(3);
    fprintf(stderr, "\033c");
}
