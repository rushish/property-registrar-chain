#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <ctype.h>

#define BLOCK_SIZE 256
#define MAX_USERS 10

const char validCharacters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@#$%^-+&~";

struct User
{
    char username[50];
    char password[50];
    int isAdmin;
};

struct Block
{
    int index;
    time_t timestamp;
    char data[BLOCK_SIZE];
    char originalData[BLOCK_SIZE];
    char propertyName[50];
    char address[100];
    char city[50];
    char postalCode[10];
    char rent[20];
    char sellingPrice[20];
    char prevHash[64];
    char hash[64];
    struct Block *next;
};

void calculateHash(struct Block *block)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, block->data, strlen(block->data));
    SHA256_Update(&sha256, block->prevHash, 64);
    SHA256_Update(&sha256, (char *)&(block->timestamp), sizeof(time_t));

    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_Final(digest, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        block->hash[i] = validCharacters[digest[i] % 62];
    }
    block->hash[SHA256_DIGEST_LENGTH] = '\0';
}

struct Block *createBlock(struct Block *prevBlock, char *data, char *propertyName, char *address, char *city, char *postalCode, char *rent, char *sellingPrice)
{
    struct Block *newBlock = (struct Block *)malloc(sizeof(struct Block));
    newBlock->index = prevBlock->index + 1;
    newBlock->timestamp = time(NULL);
    strcpy(newBlock->data, data);
    strcpy(newBlock->originalData, data);
    strcpy(newBlock->propertyName, propertyName);
    strcpy(newBlock->address, address);
    strcpy(newBlock->city, city);
    strcpy(newBlock->postalCode, postalCode);
    strcpy(newBlock->rent, rent);
    strcpy(newBlock->sellingPrice, sellingPrice);
    strcpy(newBlock->prevHash, prevBlock->hash);
    newBlock->next = NULL;
    calculateHash(newBlock);
    return newBlock;
}

void deletePropertyByName(struct Block **blockchain, int *chainLength, char *propertyName)
{
    struct Block *currentBlock = *blockchain;
    struct Block *prevBlock = NULL;
    int propertyFound = 0;
    char propertyNameLowerCase[50];

    // Convert the property name to lowercase for case-insensitive comparison
    for (int j = 0; propertyName[j]; j++)
    {
        propertyNameLowerCase[j] = tolower((unsigned char)propertyName[j]);
    }
    propertyNameLowerCase[strlen(propertyName)] = '\0';

    while (currentBlock)
    {
        char currentPropertyNameLowerCase[50];

        // Convert the property name in the blockchain to lowercase for comparison
        for (int j = 0; currentBlock->propertyName[j]; j++)
        {
            currentPropertyNameLowerCase[j] = tolower((unsigned char)currentBlock->propertyName[j]);
        }
        currentPropertyNameLowerCase[strlen(currentBlock->propertyName)] = '\0';

        if (strstr(currentPropertyNameLowerCase, propertyNameLowerCase) != NULL)
        {
            propertyFound = 1;
            if (prevBlock)
            {
                prevBlock->next = currentBlock->next;
            }
            else
            {
                *blockchain = currentBlock->next;
            }
            free(currentBlock);
            (*chainLength)--;
            printf("---------------------------------------------------------\n");
            printf("Property '%s' deleted.\n", propertyName);
            printf("---------------------------------------------------------\n");
            break;
        }
        prevBlock = currentBlock;
        currentBlock = currentBlock->next;
    }

    if (!propertyFound)
    {
        printf("---------------------------------------------------------\n");
        printf("Property '%s' does not exist.\n");
        printf("---------------------------------------------------------\n");
    }
}

void freeBlockchain(struct Block *head)
{
    while (head)
    {
        struct Block *temp = head;
        head = head->next;
        free(temp);
    }
}

void displayBlockchain(struct Block *blockchain)
{
    struct Block *currentBlock = blockchain;
    int i = 0;
    while (currentBlock)
    {
        printf("---------------------------------------------------------\n");
        printf("Property #%d:\n", i);
        printf("---------------------------------------------------------\n");
        printf("Property Name: %s\n", currentBlock->propertyName);
        printf("---------------------------------------------------------\n");
        printf("Address: %s\n", currentBlock->address);
        printf("City: %s\n", currentBlock->city);
        printf("Postal Code: %s\n", currentBlock->postalCode);
        printf("Rent: %s\n", currentBlock->rent);
        printf("Selling Price: %s\n", currentBlock->sellingPrice);
        printf("Timestamp: %ld\n", currentBlock->timestamp);
        printf("---------------------------------------------------------\n");
        printf("Prev Hash: %s\n", currentBlock->prevHash);
        printf("---------------------------------------------------------\n");
        printf("Hash: %s\n\n", currentBlock->hash);
        currentBlock = currentBlock->next;
        i++;
    }
}

int authenticateUser(struct User *users, int numUsers, char *username, char *password, int *isAdmin)
{
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            *isAdmin = users[i].isAdmin;
            return 1;
        }
    }
    return 0;
}

void addPropertyForAuction(struct Block **blockchain, int *chainLength, int isAdmin)
{
    char propertyData[BLOCK_SIZE];
    char propertyName[50];
    char address[100];
    char city[50];
    char postalCode[10];
    char rent[20];
    char sellingPrice[20];

    printf("Enter Property Name: ");
    inputString("", propertyName, sizeof(propertyName));

    // Check if the property name already exists
    if (propertyExists(*blockchain, propertyName))
    {
        printf("---------------------------------------------------------\n");
        printf("This Property is Already Registered!\n");
        printf("---------------------------------------------------------\n");
        return;
    }

    printf("Enter Address: ");
    inputString("", address, sizeof(address));

    printf("Enter City: ");
    inputString("", city, sizeof(city));

    printf("Enter Postal Code: ");
    inputString("", postalCode, sizeof(postalCode));

    printf("Enter Rent: ");
    inputString("", rent, sizeof(rent));

    printf("Enter Selling Price: ");
    inputString("", sellingPrice, sizeof(sellingPrice));

    snprintf(propertyData, sizeof(propertyData), "Property Name: %s\nAddress: %s\nCity: %s\nPostal Code: %s\nRent: %s\nSelling Price: %s", propertyName, address, city, postalCode, rent, sellingPrice);

    if (*chainLength == 0)
    {
        *blockchain = createBlock(NULL, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    }
    else
    {
        struct Block *currentBlock = *blockchain;
        while (currentBlock->next != NULL)
        {
            currentBlock = currentBlock->next;
        }
        currentBlock->next = createBlock(currentBlock, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    }

    (*chainLength)++;
    printf("---------------------------------------------------------\n");
    printf("Property added for auction.\n");
    printf("---------------------------------------------------------\n");
}

int propertyExists(struct Block *blockchain, const char *propertyName)
{
    struct Block *currentBlock = blockchain;
    while (currentBlock)
    {
        if (strcmp(currentBlock->propertyName, propertyName) == 0)
        {
            return 1; // Property with the same name exists
        }
        currentBlock = currentBlock->next;
    }
    return 0; // Property with the same name doesn't exist
}

void searchPropertyByName(struct Block *blockchain)
{
    char propertyName[BLOCK_SIZE];
    int found = 0;

    printf("---------------------------------------------------------\n");
    printf("Enter Property Name to search: ");
    inputString("", propertyName, sizeof(propertyName));

    // Convert the input property name to lowercase
    for (int i = 0; propertyName[i]; i++)
    {
        propertyName[i] = tolower((unsigned char)propertyName[i]);
    }

    struct Block *currentBlock = blockchain;
    int i = 1;
    while (currentBlock)
    {
        char propertyNameLowerCase[50];
        // Convert the property name in the blockchain to lowercase
        strcpy(propertyNameLowerCase, currentBlock->propertyName);
        for (int j = 0; propertyNameLowerCase[j]; j++)
        {
            propertyNameLowerCase[j] = tolower((unsigned char)propertyNameLowerCase[j]);
        }

        if (strstr(propertyNameLowerCase, propertyName) != NULL)
        {
            found = 1;
            printf("---------------------------------------------------------\n");
            printf("Property #%d:\n", i);
            printf("---------------------------------------------------------\n");
            printf("Property Name: %s\n", currentBlock->propertyName);
            printf("---------------------------------------------------------\n");
            printf("Address: %s\n", currentBlock->address);
            printf("City: %s\n", currentBlock->city);
            printf("Postal Code: %s\n", currentBlock->postalCode);
            printf("Rent: %s\n", currentBlock->rent);
            printf("Selling Price: %s\n", currentBlock->sellingPrice);
            printf("Timestamp: %ld\n", currentBlock->timestamp);
            printf("---------------------------------------------------------\n");
            printf("Prev Hash: %s\n", currentBlock->prevHash);
            printf("---------------------------------------------------------\n");
            printf("Hash: %s\n\n", currentBlock->hash);
        }

        currentBlock = currentBlock->next;
        i++;
    }

    if (!found)
    {
        printf("---------------------------------------------------------\n");
        printf("No matching properties found.\n");
        printf("---------------------------------------------------------\n");
    }
}

void inputString(const char *prompt, char *dest, size_t destSize)
{
    printf("%s", prompt);
    fgets(dest, destSize, stdin);
    dest[strcspn(dest, "\n")] = '\0';
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void saveBlockchainToFile(const char *filename, struct Block *blockchain)
{
    FILE *file = fopen(filename, "wb");
    if (file != NULL)
    {
        struct Block *currentBlock = blockchain;
        while (currentBlock)
        {
            fwrite(currentBlock, sizeof(struct Block), 1, file);
            currentBlock = currentBlock->next;
        }
        fclose(file);
    }
    else
    {
        printf("---------------------------------------------------------\n");
        printf("Error: Unable to open file for writing.\n");
        printf("---------------------------------------------------------\n");
    }
}

struct Block *loadBlockchainFromFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    struct Block *blockchain = NULL;
    if (file != NULL)
    {
        struct Block block;
        while (fread(&block, sizeof(struct Block), 1, file) == 1)
        {
            struct Block *newBlock = (struct Block *)malloc(sizeof(struct Block));
            *newBlock = block;
            newBlock->next = NULL;

            if (blockchain == NULL)
            {
                blockchain = newBlock;
            }
            else
            {
                struct Block *currentBlock = blockchain;
                while (currentBlock->next != NULL)
                {
                    currentBlock = currentBlock->next;
                }
                currentBlock->next = newBlock;
            }
        }
        fclose(file);
    }
    else
    {
        printf("---------------------------------------------------------\n");
        printf("Error: Unable to open file for reading.\n");
        printf("---------------------------------------------------------\n");
    }
    return blockchain;
}

void inputPassword(const char *prompt, char *password, size_t passwordSize)
{
    printf("%s", prompt);

    int index = 0;
    int ch;

    while (1)
    {
        ch = getch(); // Read a character without displaying it
        if (ch == '\n' || ch == '\r')
        {
            break; // Stop if Enter key is pressed
        }
        else if (ch == '\b' || ch == 127)
        {
            // Handle backspace or delete key
            if (index > 0)
            {
                printf("\b \b"); // Clear the character from the screen
                index--;
            }
        }
        else if (index < passwordSize - 1)
        {
            password[index++] = ch;
            printf("*"); // Display an asterisk for each character
        }
    }

    password[index] = '\0';
    printf("\n");
}

void applyToListProperty(struct Block **blockchain, int *chainLength)
{
    char propertyData[BLOCK_SIZE];
    char propertyName[50];
    char address[100];
    char city[50];
    char postalCode[10];
    char rent[20];
    char sellingPrice[20];

    printf("---------------------------------------------------------\n");
    printf("Enter Property Name: ");
    inputString("", propertyName, sizeof(propertyName));

    // Check if the property already exists
    struct Block *currentBlock = *blockchain;
    while (currentBlock)
    {
        if (strcmp(currentBlock->propertyName, propertyName) == 0)
        {
            printf("---------------------------------------------------------\n");
            printf("This Property Already Exists\n");
            printf("---------------------------------------------------------\n");
            return;
        }
        currentBlock = currentBlock->next;
    }

    printf("Enter Address: ");
    inputString("", address, sizeof(address));

    printf("Enter City: ");
    inputString("", city, sizeof(city));

    printf("Enter Postal Code: ");
    inputString("", postalCode, sizeof(postalCode));

    printf("Enter Rent: ");
    inputString("", rent, sizeof(rent));

    printf("Enter Selling Price: ");
    inputString("", sellingPrice, sizeof(sellingPrice));

    snprintf(propertyData, sizeof(propertyData), "Property Name: %s\nAddress: %s\nCity: %s\nPostal Code: %s\nRent: %s\nSelling Price: %s", propertyName, address, city, postalCode, rent, sellingPrice);

    if (*chainLength == 0)
    {
        *blockchain = createBlock(NULL, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    }
    else
    {
        struct Block *currentBlock = *blockchain;
        while (currentBlock->next != NULL)
        {
            currentBlock = currentBlock->next;
        }
        currentBlock->next = createBlock(currentBlock, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    }

    (*chainLength)++;
    printf("---------------------------------------------------------\n");
    printf("Your Property listing application submitted successfully.\n Your Property will be added after review.\n");
    printf("---------------------------------------------------------\n");
}

int main()
{
    struct User users[MAX_USERS] = {
        {"admin", "adminpass", 1},
        {"user", "userpass", 0}};
    int numUsers = 2;

    struct Block *genesisBlock = (struct Block *)malloc(sizeof(struct Block));
    genesisBlock->index = 0;
    genesisBlock->timestamp = time(NULL);
    strcpy(genesisBlock->data, "Genesis Block");
    strcpy(genesisBlock->originalData, "Genesis Block");
    strcpy(genesisBlock->propertyName, "Genesis Property");
    strcpy(genesisBlock->address, "Genesis Address");
    strcpy(genesisBlock->city, "Genesis City");
    strcpy(genesisBlock->postalCode, "000000");
    strcpy(genesisBlock->rent, "N/A");
    strcpy(genesisBlock->sellingPrice, "N/A");
    strcpy(genesisBlock->prevHash, "0");
    calculateHash(genesisBlock);
    genesisBlock->next = NULL;

    struct Block *blockchain = genesisBlock;
    int chainLength = 1;

    struct Block *loadedBlockchain = loadBlockchainFromFile("blockchain.dat");
    if (loadedBlockchain != NULL)
    {
        freeBlockchain(blockchain);
        blockchain = loadedBlockchain;
        struct Block *currentBlock = blockchain;
        while (currentBlock)
        {
            calculateHash(currentBlock);
            currentBlock = currentBlock->next;
        }
    }

    char username[50];
    char password[50];
    int isAdmin = 0;

    printf("---------------------------------------------------------\n");
    printf("Login:\n");
    printf("---------------------------------------------------------\n");
    inputString("Username: ", username, sizeof(username));
    inputPassword("Password: ", password, sizeof(password));

    if (authenticateUser(users, numUsers, username, password, &isAdmin))
    {
        printf("---------------------------------------------------------\n");
        printf("Login successful!\n");
        printf("---------------------------------------------------------\n");

        printf("Loading Property Registrar...");
        fflush(stdout);
        for (int i = 0; i < 3; i++)
        {
            printf(".");
            fflush(stdout);
            sleep(1);
        }

        clearScreen();

        if (isAdmin)
        {
            char propertyToDelete[BLOCK_SIZE];
            while (1)
            {
                printf("---------------------------------------------------------\n");
                printf("Options for Admin:\n");
                printf("---------------------------------------------------------\n");
                printf("1. Add Property for Auction\n");
                printf("2. Search Property by Name\n");
                printf("3. Delete Property by Name\n");
                printf("4. View all Available Properties\n");
                printf("5. Save and Exit\n");
                printf("---------------------------------------------------------\n");
                printf("Enter your choice: ");

                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n')
                    ;

                switch (choice)
                {
                case 1:
                    addPropertyForAuction(&blockchain, &chainLength, isAdmin);
                    saveBlockchainToFile("blockchain.dat", blockchain);
                    break;
                case 2:
                    searchPropertyByName(blockchain);
                    break;
                case 3:
                    inputString("Enter Property Name to delete: ", propertyToDelete, sizeof(propertyToDelete));
                    deletePropertyByName(&blockchain, &chainLength, propertyToDelete);
                    saveBlockchainToFile("blockchain.dat", blockchain);
                    break;
                case 4:
                    displayBlockchain(blockchain);
                    break;
                case 5:
                    printf("Exiting the program.\n");
                    freeBlockchain(blockchain);
                    exit(0);
                default:
                    printf("Invalid choice. Please try again.\n");
                }
            }
        }
        else
        {
            while (1)
            {
                printf("---------------------------------------------------------\n");
                printf("Options for User:\n");
                printf("---------------------------------------------------------\n");
                printf("1. Search Property by Name\n");
                printf("2. View all Available Properties\n");
                printf("3. Apply to List Your Property\n");
                printf("4. Save and Exit\n");
                printf("Enter your choice: ");

                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n')
                    ;

                switch (choice)
                {
                case 1:
                    searchPropertyByName(blockchain);
                    break;
                case 2:
                    displayBlockchain(blockchain);
                    break;
                case 3:
                    applyToListProperty(&blockchain, &chainLength);
                    break;
                case 4:
                    printf("---------------------------------------------------------\n");
                    printf("Exiting the program.\n");
                    printf("---------------------------------------------------------\n");
                    freeBlockchain(blockchain);
                    exit(0);
                default:
                    printf("---------------------------------------------------------\n");
                    printf("Invalid choice. Please try again.\n");
                    printf("---------------------------------------------------------\n");
                }
            }
        }
    }
    else
    {
        printf("---------------------------------------------------------\n");
        printf("Login failed. Invalid credentials.\n");
        printf("---------------------------------------------------------\n");
    }

    freeBlockchain(blockchain);
    return 0;
}
