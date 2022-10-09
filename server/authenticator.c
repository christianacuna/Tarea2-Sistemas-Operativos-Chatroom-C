#define USER_COUNT 5
#define BUFFER 32

/* Pair structure */
typedef struct
{
    char first[BUFFER];
    char second[BUFFER];
} Pair;

/* Data base of users */
Pair Credentials[USER_COUNT] = {
    {"atlas", "admin"},
    {"ginebra", "admin"},
    {"guest", "1234"},
    {"client", "4321"},
    {"admin", "admin"},
};

/**
 * Validates if the user password of correct
 *
 * @param user username to match
 * @param password of the supposed user
 * @return boolean value
 */
int authenticate(char *username, char *password)
{
    int result = 0;

    for (int i = 0; i < USER_COUNT; i++)
    {
        if (strcmp(Credentials[i].first, username) == 0)
        {
            result = strcmp(Credentials[i].second, password) == 0;
            break;
        }
    }

    return result;
}
