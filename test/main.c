#include "../ATM/atm.h"

static int makeUserLoginTest(struct BankUser *user);
static int getMoneyTest(struct BankUser *user);
static int sendMoneyTest(struct BankUser *user);

int main(void)
{
    if (initDB())
        return 1;

    struct BankUser user;

    fprintf(stderr, "Start Test make User Login.\n");
    if (makeUserLoginTest(&user))
        return 1;
    fprintf(stderr, "Test make User Login Pass!\n");

    fprintf(stderr, "Start Test get Money.\n");
    if (getMoneyTest(&user))
        return 1;
    fprintf(stderr, "Test get Money Pass!\n");

    fprintf(stderr, "Start Test send Money.\n");
    if (sendMoneyTest(&user))
        return 1;
    fprintf(stderr, "Test send Money Pass!\n");

    fprintf(stderr, "All Test Pass!\n");
    closeDB();
    return 0;
}

static int makeUserLoginTest(struct BankUser *user)
{
    int rc = makeLoginUser(user, "111", "oh may");
    if (rc != NOT_FOUND_USER)
    {
        fprintf(stderr, "for Not Valid User Not work!\n");
        return 1;
    }

    rc = makeLoginUser(user, "111111112", "1111");
    if (rc != OK)
    {
        fprintf(stderr, "for Valid User Not work!\n");
        return 1;
    }

    return OK;
}

static int getMoneyTest(struct BankUser *user)
{
    int rc = getMoney(user, -1);
    if (rc != VALUE_IS_NOT_VALID)
    {
        fprintf(stderr, "for Not Valid VALUE IS Valid!\n");
        return 1;
    }

    rc = getMoney(user, 10000000000);
    if (rc != NOT_ENOUGH_MONEY)
    {
        fprintf(stderr, "for NOT ENOUGH MONEY Not work!\n");
        return 1;
    }

    return OK;
}

static int sendMoneyTest(struct BankUser *user)
{
    int rc = sendMoney(user, "", -1);
    if (rc != VALUE_IS_NOT_VALID)
    {
        fprintf(stderr, "for Not Valid VALUE IS Valid!\n");
        return 1;
    }

    rc = sendMoney(user, "", 10000000000);
    if (rc != NOT_ENOUGH_MONEY)
    {
        fprintf(stderr, "for NOT ENOUGH MONEY Not work!\n");
        return 1;
    }

    rc = sendMoney(user, "1245", 1000);
    if (rc != NOT_FOUND_USER)
    {
        fprintf(stderr, "for NOT Valid User work!\n");
        return 1;
    }

    rc = sendMoney(user, user->CartID, 1000);
    if (rc != ERROR_SELF_SEND_FOR_SELF)
    {
        fprintf(stderr, "for self cart id work!\n");
        return 1;
    }

    return OK;
}
