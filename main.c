#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <ctype.h>

#define BLOCK_SIZE 256
#define MAX_USERS 10

const char validCharacters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@#$%^-+&~";

struct User {
    char username[50];
    char password[50];
    int isAdmin;
};

struct Block {
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

void calculateHash(struct Block *block) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, block->data, strlen(block->data));
    SHA256_Update(&sha256, block->prevHash, 64);
    SHA256_Update(&sha256, (char *)&(block->timestamp), sizeof(time_t));

    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_Final(digest, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        block->hash[i] = validCharacters[digest[i] % 62];
    }
    block->hash[SHA256_DIGEST_LENGTH] = '\0';
}

struct Block *createBlock(struct Block *prevBlock, char *data, char *propertyName, char *address, char *city, char *postalCode, char *rent, char *sellingPrice) {
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

void deletePropertyByName(struct Block **blockchain, int *chainLength, char *propertyName) {
    struct Block *currentBlock = *blockchain;
    struct Block *prevBlock = NULL;

    while (currentBlock) {
        if (strcmp(currentBlock->propertyName, propertyName) == 0) {
            if (prevBlock) {
                prevBlock->next = currentBlock->next;
            } else {
                *blockchain = currentBlock->next;
            }
            free(currentBlock);
            (*chainLength)--;
            printf("Property '%s' deleted.\n", propertyName);
            return;
        }
        prevBlock = currentBlock;
        currentBlock = currentBlock->next;
    }
    printf("Property '%s' not found.\n");
}

void freeBlockchain(struct Block *head) {
    while (head) {
        struct Block *temp = head;
        head = head->next;
        free(temp);
    }
}

void displayBlockchain(struct Block *blockchain) {
    struct Block *currentBlock = blockchain;
    int i = 0;
    while (currentBlock) {
        printf("Property #%d:\n", i);
        printf("Property Name: %s\n", currentBlock->propertyName);
        printf("Address: %s\n", currentBlock->address);
        printf("City: %s\n", currentBlock->city);
        printf("Postal Code: %s\n", currentBlock->postalCode);
        printf("Rent: %s\n", currentBlock->rent);
        printf("Selling Price: %s\n", currentBlock->sellingPrice);
        printf("Timestamp: %ld\n", currentBlock->timestamp);
        printf("Prev Hash: %s\n", currentBlock->prevHash);
        printf("Hash: %s\n\n", currentBlock->hash);
        currentBlock = currentBlock->next;
        i++;
    }
}

int authenticateUser(struct User *users, int numUsers, char *username, char *password, int *isAdmin) {
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            *isAdmin = users[i].isAdmin;
            return 1;
        }
    }
    return 0;
}

void addPropertyForAuction(struct Block **blockchain, int *chainLength, int isAdmin) {
    char propertyData[BLOCK_SIZE];
    char propertyName[50];
    char address[100];
    char city[50];
    char postalCode[10];
    char rent[20];
    char sellingPrice[20];

    printf("Enter Property Name: ");
    inputString("", propertyName, sizeof(propertyName));

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

    if (*chainLength == 0) {
        *blockchain = createBlock(NULL, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    } else {
        struct Block *currentBlock = *blockchain;
        while (currentBlock->next != NULL) {
            currentBlock = currentBlock->next;
        }
        currentBlock->next = createBlock(currentBlock, propertyData, propertyName, address, city, postalCode, rent, sellingPrice);
    }

    (*chainLength)++;
    printf("Property added for auction.\n");
}

void searchPropertyByName(struct Block *blockchain) {
    char propertyName[BLOCK_SIZE];
    int found = 0;

    printf("Enter Property Name to search: ");
    inputString("", propertyName, sizeof(propertyName));

    // Convert the input property name to lowercase
    for (int i = 0; propertyName[i]; i++) {
        propertyName[i] = tolower((unsigned char)propertyName[i]);
    }

    struct Block *currentBlock = blockchain;
    int i = 1;
    while (currentBlock) {
        char propertyNameLowerCase[50];
        // Convert the property name in the blockchain to lowercase
        strcpy(propertyNameLowerCase, currentBlock->propertyName);
        for (int j = 0; propertyNameLowerCase[j]; j++) {
            propertyNameLowerCase[j] = tolower((unsigned char)propertyNameLowerCase[j]);
        }

        if (strstr(propertyNameLowerCase, propertyName) != NULL) {
            found = 1;
            printf("Property #%d:\n", i);
            printf("Property Name: %s\n", currentBlock->propertyName);
            printf("Address: %s\n", currentBlock->address);
            printf("City: %s\n", currentBlock->city);
            printf("Postal Code: %s\n", currentBlock->postalCode);
            printf("Rent: %s\n", currentBlock->rent);
            printf("Selling Price: %s\n", currentBlock->sellingPrice);
            printf("Timestamp: %ld\n", currentBlock->timestamp);
            printf("Prev Hash: %s\n", currentBlock->prevHash);
            printf("Hash: %s\n\n", currentBlock->hash);
        }

        currentBlock = currentBlock->next;
        i++;
    }

    if (!found) {
        printf("No matching properties found.\n");
    }
}

void inputString(const char *prompt, char *dest, size_t destSize) {
    printf("%s", prompt);
    fgets(dest, destSize, stdin);
    dest[strcspn(dest, "\n")] = '\0';
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void saveBlockchainToFile(const char *filename, struct Block *blockchain) {
    FILE *file = fopen(filename, "wb");
    if (file != NULL) {
        struct Block *currentBlock = blockchain;
        while (currentBlock) {
            fwrite(currentBlock, sizeof(struct Block), 1, file);
            currentBlock = currentBlock->next;
        }
        fclose(file);
    } else {
        printf("Error: Unable to open file for writing.\n");
    }
}

struct Block *loadBlockchainFromFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    struct Block *blockchain = NULL;
    if (file != NULL) {
        struct Block block;
        while (fread(&block, sizeof(struct Block), 1, file) == 1) {
            struct Block *newBlock = (struct Block *)malloc(sizeof(struct Block));
            *newBlock = block;
            newBlock->next = NULL;

            if (blockchain == NULL) {
                blockchain = newBlock;
            } else {
                struct Block *currentBlock = blockchain;
                while (currentBlock->next != NULL) {
                    currentBlock = currentBlock->next;
                }
                currentBlock->next = newBlock;
            }
        }
        fclose(file);
    } else {
        printf("Error: Unable to open file for reading.\n");
    }
    return blockchain;
}

int main() {
    struct User users[MAX_USERS] = {
        {"admin", "adminpass", 1},
        {"user", "userpass", 0}
    };
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
    if (loadedBlockchain != NULL) {
        freeBlockchain(blockchain);
        blockchain = loadedBlockchain;
        struct Block *currentBlock = blockchain;
        while (currentBlock) {
            calculateHash(currentBlock);
            currentBlock = currentBlock->next;
        }
    }

    char username[50];
    char password[50];
    int isAdmin = 0;

    printf("Login:\n");
    inputString("Username: ", username, sizeof(username));
    inputString("Password: ", password, sizeof(password));

    if (authenticateUser(users, numUsers, username, password, &isAdmin)) {
        printf("Login successful!\n");

        printf("Loading Property Registrar...");
        fflush(stdout);
        for (int i = 0; i < 3; i++) {
            printf(".");
            fflush(stdout);
            sleep(1);
        }

        clearScreen();

        if (isAdmin) {
            char propertyToDelete[BLOCK_SIZE];
            while (1) {
                printf("Options for Admin:\n");
                printf("1. Add Property for Auction\n");
                printf("2. Search Property by Name\n");
                printf("3. Delete Property by Name\n");
                printf("4. View all Available Properties\n");
                printf("5. Save and Exit\n");
                printf("Enter your choice: ");

                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n');

                switch (choice) {
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
        } else {
            while (1) {
                printf("Options for User:\n");
                printf("1. Search Property by Name\n");
                printf("2. View all Available Properties\n");
                printf("3. Save and Exit\n");
                printf("Enter your choice: ");

                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n');

                switch (choice) {
                    case 1:
                        searchPropertyByName(blockchain);
                        break;
                    case 2:
                        displayBlockchain(blockchain);
                        break;
                    case 3:
                        printf("Exiting the program.\n");
                        freeBlockchain(blockchain);
                        exit(0);
                    default:
                        printf("Invalid choice. Please try again.\n");
                }
            }
        }
    } else {
        printf("Login failed. Invalid credentials.\n");
    }

    freeBlockchain(blockchain);
    return 0;
}
