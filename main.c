#include "ATM/atm.h"

int main()
{
    if (initDB())
        return 1;

    struct BankUser user;

    char CardID[512], p[512];
    int value, rc;

    for (;;)
    {
        rc = getLine("Enter Your CardID: ", CardID, sizeof CardID);
        if (handelError(rc) ||
            handelError(strlen(CardID) > 10 ? NOT_VALID_CARD_ID : OK))
            continue;

        if (!strcmp(CardID, "exit"))
            break;

        char *password = getpass("Password: ");

        rc = makeLoginUser(&user, CardID, password);
        if (handelError(rc))
            continue;

        clearPrint(accountInfo(&user));

        for (;;)
        {
            int choose;
            fprintf(stderr, "Choose One Action!\n");
            fprintf(stderr, "1 - Take Money From ATM\n");
            fprintf(stderr, "2 - Show Account Info\n");
            fprintf(stderr, "3 - Send Money For Another\n");
            fprintf(stderr, "4 - Exit From ATM\n");
            fprintf(stderr, "Enter Number: ");
            scanf("%d", &choose);
            fprintf(stderr, "\033c");

            if (choose == 1)
            {
                fprintf(stderr, "Enter Value: ");
                rc = scanf("%d", &value);
                rc = getMoney(&user, value);

                if (handelError(rc))
                    continue;

                sprintf(p, "Get Your Money (%d)\n"
                           "Fee Is (%d)\n"
                           "Your money now: %ld",
                        value, FEE, user.Money);
                clearPrint(p);
            }
            else if (choose == 2)
                clearPrint(accountInfo(&user));
            else if (choose == 3)
            {
                fprintf(stderr, "Enter Value: ");
                rc = scanf("%d", &value);

                rc = getLine("Enter Your CardID: ", CardID, sizeof CardID);
                if (handelError(rc) ||
                    handelError(strlen(CardID) > 10 ? NOT_VALID_CARD_ID : OK))
                    continue;

                rc = sendMoney(&user, CardID, value);
                if (handelError(rc))
                    continue;

                sprintf(p, "Send Money (%d)\n"
                           "For (%s)\n"
                           "Fee Is (%d)\n"
                           "Your money now: %ld",
                        value, CardID, FEE, user.Money);
                clearPrint(p);
            }
            else if (choose == 4)
            {
                clearPrint("See You!");
                break;
            }
            else
                clearPrint("Choose Not Valid!\n");
        }
    }

    closeDB();
    return OK;
}
