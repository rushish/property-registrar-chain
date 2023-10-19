#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <ctype.h>

#define BLOCK_SIZE 256
#define MAX_USERS 10

const char validCharacters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

struct User {
    char username[50];
    char password[50];
    int isAdmin;
};

struct Block {
    int index;
    time_t timestamp;
    char data[BLOCK_SIZE];
    char originalData[BLOCK_SIZE];  // Store the original case data
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

struct Block *createBlock(struct Block *prevBlock, char *data) {
    struct Block *newBlock = (struct Block *)malloc(sizeof(struct Block));
    newBlock->index = prevBlock->index + 1;
    newBlock->timestamp = time(NULL);
    strcpy(newBlock->data, data);
    strcpy(newBlock->originalData, data);
    strcpy(newBlock->prevHash, prevBlock->hash);
    newBlock->next = NULL;
    calculateHash(newBlock);
    return newBlock;
}

// Function to free the entire linked list
void freeBlockchain(struct Block *head) {
    while (head) {
        struct Block *temp = head;
        head = head->next;
        free(temp);
    }
}

void inputString(const char *prompt, char *dest, size_t destSize) {
    printf("%s", prompt);
    fgets(dest, destSize, stdin);
    dest[strcspn(dest, "\n")] = '\0';
}

void displayBlockchain(struct Block *blockchain) {
    struct Block *currentBlock = blockchain;
    int i = 0;
    while (currentBlock) {
        printf("Property #%d:\n", i);
        printf("%s\n", currentBlock->originalData);
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

    inputString("Enter Property Name: ", propertyData, sizeof(propertyData));
    strncat(propertyData, "\nAddress: ", sizeof(propertyData) - strlen(propertyData) - 1);
    inputString("Enter Address: ", propertyData + strlen(propertyData), sizeof(propertyData) - strlen(propertyData));
    strncat(propertyData, "\nCity: ", sizeof(propertyData) - strlen(propertyData) - 1);
    inputString("Enter City: ", propertyData + strlen(propertyData), sizeof(propertyData) - strlen(propertyData));
    strncat(propertyData, "\nPostal Code: ", sizeof(propertyData) - strlen(propertyData) - 1);
    inputString("Enter Postal Code: ", propertyData + strlen(propertyData), sizeof(propertyData) - strlen(propertyData));
    strncat(propertyData, "\nRent: ", sizeof(propertyData) - strlen(propertyData) - 1);
    inputString("Enter Rent: ", propertyData + strlen(propertyData), sizeof(propertyData) - strlen(propertyData));
    strncat(propertyData, "\nSelling Price: ", sizeof(propertyData) - strlen(propertyData) - 1);
    inputString("Enter Selling Price: ", propertyData + strlen(propertyData), sizeof(propertyData) - strlen(propertyData));

    if (*chainLength == 0) {
        *blockchain = createBlock(NULL, propertyData);
    } else {
        struct Block *currentBlock = *blockchain;
        while (currentBlock->next != NULL) {
            currentBlock = currentBlock->next;
        }
        currentBlock->next = createBlock(currentBlock, propertyData);
    }

    (*chainLength)++;
    printf("Property added for auction.\n");
}

void searchPropertyByName(struct Block *blockchain) {
    char propertyName[BLOCK_SIZE];
    int found = 0;

    inputString("Enter Property Name to search: ", propertyName, sizeof(propertyName));
    strlwr(propertyName); // Convert the search string to lowercase for case-insensitive search

    struct Block *currentBlock = blockchain;
    int i = 1;
    while (currentBlock) {
        char propertyDataLowerCase[BLOCK_SIZE];
        strcpy(propertyDataLowerCase, currentBlock->data);
        strlwr(propertyDataLowerCase); // Convert property data to lowercase for case-insensitive search

        if (strstr(propertyDataLowerCase, propertyName) != NULL) {
            found = 1;
            printf("Property #%d:\n", i);
            printf("%s\n", currentBlock->originalData); // Display original case property data
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

// Function to clear the screen (platform-specific)
void clearScreen() {
#ifdef _WIN32
    system("cls"); // For Windows
#else
    system("clear"); // For Linux and macOS
#endif
}

// Save the blockchain data to a .dat file
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

// Load the blockchain data from a .dat file
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
    strcpy(genesisBlock->prevHash, "0");
    calculateHash(genesisBlock);
    genesisBlock->next = NULL;

    struct Block *blockchain = genesisBlock;
    int chainLength = 1;

    // Load the blockchain data from a .dat file if it exists
    struct Block *loadedBlockchain = loadBlockchainFromFile("blockchain.dat");
    if (loadedBlockchain != NULL) {
        freeBlockchain(blockchain);
        blockchain = loadedBlockchain;
        // Recalculate the hashes
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

        clearScreen(); // Clear the screen after loading Registrar

        if (isAdmin) {
            while (1) {
                printf("Options for Admin:\n");
                printf("1. Add Property for Auction\n");
                printf("2. Search Property by Name\n");
                printf("3. View all Available Properties\n");
                printf("4. Save and Exit\n");
                printf("Enter your choice: ");

                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n');

                switch (choice) {
                    case 1:
                        addPropertyForAuction(&blockchain, &chainLength, isAdmin);
                        // Save the updated blockchain to the .dat file
                        saveBlockchainToFile("blockchain.dat", blockchain);
                        break;
                    case 2:
                        searchPropertyByName(blockchain);
                        break;
                    case 3:
                        displayBlockchain(blockchain);
                        break;
                    case 4:
                        printf("Exiting the program.\n");
                        freeBlockchain(blockchain);
                        exit(0);
                    default:
                        printf("Invalid choice. Please try again.\n");
                }
            }
        } else { // User
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
