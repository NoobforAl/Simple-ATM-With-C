#ifndef ATM_H
#define ATM_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#include "../SHA-256/sha256.h"

struct BankUser
{
    uint64_t Id;
    char Name[256];
    char CartID[256];
    int64_t Money;
};

//* All Status Program
#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

#define NOT_FOUND_USER 3

#define DATABASE_ERROR 4
#define CANT_MAKE_DATA 5
#define CANT_OPEN_DATABASE 6

#define NOT_ENOUGH_MONEY 7
#define NOT_VALID_CARD_ID 8
#define VALUE_IS_NOT_VALID 9
#define ERROR_SELF_SEND_FOR_SELF 10

//* how much fee for each pay
#define FEE 2500

//* init database for first time run program
int initDB(void);

//* Close Database
void closeDB(void);

//* insert temp data in database if database is empty!
int insertData(void);

/*
 * setup login user
 * get cardId and password
 * check on database
 * and if exist setup value to ( user )
 *
 * Args :
 *   struct BankUser *user
 *   char *CardID
 *   char *password
 *
 * Return Status Code
 */
int makeLoginUser(struct BankUser *user, char *CardID, char *password);

/*
 * get money from atm
 *
 * Args :
 *   struct BankUser *user
 *   int value
 *
 * Return Status Code
 */
int getMoney(struct BankUser *user, int64_t value);

/*
 * send money for another with card id
 *
 * Args :
 *   struct BankUser *user
 *   char *CardID
 *   int value
 *
 * Return Status Code
 */
int sendMoney(struct BankUser *user, char *CardID, int64_t value);

/*
 * check status code
 * if code is error code
 * print error and return again code
 *
 * Args :
 *   int code
 *
 * Return Status Code
 */
int handelError(int code);

/*
 * show info account
 *
 * Args :
 *   struct BankUser *user
 *
 * Return char (string)
 */
char *accountInfo(struct BankUser *user);

/*
 * get string from stdin with size of char
 *
 * Args :
 *   char *prmpt
 *   char *buff
 *   size_t sz
 *
 * Return Status Code
 */
int getLine(char *prmpt, char *buff, size_t sz);

/*
 * first clear stdout
 * and print message
 * after 3s clear stdout
 *
 * Args :
 *   char *t
 *
 */
void clearPrint(char *t);

#endif