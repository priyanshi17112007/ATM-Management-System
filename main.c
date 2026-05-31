#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "atm_accounts.dat"

// Structure to store Account Details
typedef struct {
    char acc_no[20];
    char name[50];
    char pin[10];
    double balance;
} Account;

// Function Prototypes
void mainMenu();
void createAccount();
int loginAccount(Account *loggedInUser);
void userMenu(Account *user);
void checkBalance(Account *user);
void depositMoney(Account *user);
void withdrawMoney(Account *user);
void transferMoney(Account *user);
void updateRecord(Account user);

int main() {
    mainMenu();
    return 0;
}

// 1. Main Landing Menu
void mainMenu() {
    int choice;
    Account currentUser;

    while (1) {
        printf("\n===================================\n");
        printf("    WELCOME TO THE ATM SIMULATOR   \n");
        printf("===================================\n");
        printf("1. Create New Account\n");
        printf("2. Login to Existing Account\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Clear newline buffer

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                if (loginAccount(&currentUser)) {
                    userMenu(&currentUser);
                }
                break;
            case 3:
                printf("\nThank you for using our ATM. Goodbye!\n");
                exit(0);
            default:
                printf("\n[Error] Invalid choice! Please try again.\n");
        }
    }
}

// 2. Feature: Account Creation
void createAccount() {
    FILE *fp = fopen(DATA_FILE, "ab"); // Open in append binary mode
    if (fp == NULL) {
        printf("\n[Error] System file error! Cannot create account.\n");
        return;
    }

    Account newAcc;
    printf("\n--- Create New Account ---\n");
    printf("Enter Account Number: ");
    fgets(newAcc.acc_no, sizeof(newAcc.acc_no), stdin);
    newAcc.acc_no[strcspn(newAcc.acc_no, "\n")] = 0; // Remove trailing newline

    printf("Enter Account Holder Name: ");
    fgets(newAcc.name, sizeof(newAcc.name), stdin);
    newAcc.name[strcspn(newAcc.name, "\n")] = 0;

    printf("Create 4-Digit Security PIN: ");
    fgets(newAcc.pin, sizeof(newAcc.pin), stdin);
    newAcc.pin[strcspn(newAcc.pin, "\n")] = 0;

    printf("Enter Initial Deposit Amount: $");
    scanf("%lf", &newAcc.balance);
    getchar(); 

    // Write structure block directly to binary file
    fwrite(&newAcc, sizeof(Account), 1, fp);
    fclose(fp);

    printf("\n[Success] Account successfully registered for %s!\n", newAcc.name);
}

// 3. Feature: Secure Login
int loginAccount(Account *loggedInUser) {
    FILE *fp = fopen(DATA_FILE, "rb"); // Open in read binary mode
    if (fp == NULL) {
        printf("\n[Error] No accounts found in the database. Please register first.\n");
        return 0;
    }

    char input_acc[20], input_pin[10];
    Account temp;
    int found = 0;

    printf("\n--- Account Login ---\n");
    printf("Enter Account Number: ");
    fgets(input_acc, sizeof(input_acc), stdin);
    input_acc[strcspn(input_acc, "\n")] = 0;

    printf("Enter Security PIN: ");
    fgets(input_pin, sizeof(input_pin), stdin);
    input_pin[strcspn(input_pin, "\n")] = 0;

    // Search file record by record
    while (fread(&temp, sizeof(Account), 1, fp)) {
        if (strcmp(temp.acc_no, input_acc) == 0 && strcmp(temp.pin, input_pin) == 0) {
            *loggedInUser = temp; // Pass data back to active session
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (found) {
        printf("\n[Success] Authentication successful! Welcome back, %s.\n", loggedInUser->name);
        return 1;
    } else {
        printf("\n[Error] Incorrect Account Number or PIN!\n");
        return 0;
    }
}

// 4. Secure Session Dashboard
void userMenu(Account *user) {
    int choice;
    while (1) {
        printf("\n--- ATM Dashboard (%s) ---\n", user->name);
        printf("1. Check Balance\n");
        printf("2. Deposit Cash\n");
        printf("3. Withdraw Cash\n");
        printf("4. Transfer Money\n");
        printf("5. Logout\n");
        printf("Enter option: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: checkBalance(user); break;
            case 2: depositMoney(user); break;
            case 3: withdrawMoney(user); break;
            case 4: transferMoney(user); break;
            case 5: 
                printf("\nLogging out from session...\n");
                return; // Drops back to the mainMenu loop
            default: 
                printf("\n[Error] Invalid option!\n");
        }
    }
}

// 5. Feature: Check Balance
void checkBalance(Account *user) {
    printf("\n===================================\n");
    printf(" Current Balance: $%.2f\n", user->balance);
    printf("===================================\n");
}

// 6. Feature: Deposit
void depositMoney(Account *user) {
    double amount;
    printf("\nEnter amount to deposit: $");
    scanf("%lf", &amount);
    getchar();

    if (amount <= 0) {
        printf("\n[Error] Invalid transaction amount.\n");
        return;
    }

    user->balance += amount;
    updateRecord(*user); // Sync state back to file
    printf("\n[Success] $%.2f deposited successfully!\n", amount);
}

// 7. Feature: Withdrawal
void withdrawMoney(Account *user) {
    double amount;
    printf("\nEnter amount to withdraw: $");
    scanf("%lf", &amount);
    getchar();

    if (amount <= 0) {
        printf("\n[Error] Invalid transaction amount.\n");
        return;
    }

    if (amount > user->balance) {
        printf("\n[Error] Insufficient funds! Current Balance: $%.2f\n", user->balance);
        return;
    }

    user->balance -= amount;
    updateRecord(*user);
    printf("\n[Success] $%.2f withdrawn successfully!\n", amount);
}

// 8. Feature: Money Transfer
void transferMoney(Account *user) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) return;

    char target_acc[20];
    double amount;
    Account targetUser;
    int targetFound = 0;

    printf("\nEnter Recipient Account Number: ");
    fgets(target_acc, sizeof(target_acc), stdin);
    target_acc[strcspn(target_acc, "\n")] = 0;

    if (strcmp(user->acc_no, target_acc) == 0) {
        printf("\n[Error] You cannot transfer money to your own account!\n");
        fclose(fp);
        return;
    }

    // Verify recipient existence
    while (fread(&targetUser, sizeof(Account), 1, fp)) {
        if (strcmp(targetUser.acc_no, target_acc) == 0) {
            targetFound = 1;
            break;
        }
    }
    fclose(fp);

    if (!targetFound) {
        printf("\n[Error] Target recipient account number not found.\n");
        return;
    }

    printf("Enter amount to transfer to %s: $", targetUser.name);
    scanf("%lf", &amount);
    getchar();

    if (amount <= 0 || amount > user->balance) {
        printf("\n[Error] Invalid amount or insufficient balance.\n");
        return;
    }

    // Process Ledger Changes
    user->balance -= amount;
    targetUser.balance += amount;

    // Persist changes for both accounts
    updateRecord(*user);
    updateRecord(targetUser);

    printf("\n[Success] Securely transferred $%.2f to %s.\n", amount, targetUser.name);
}

// 9. Data Layer: Update existing data blocks inside file
void updateRecord(Account user) {
    FILE *fp = fopen(DATA_FILE, "r+b"); // Open for reading and writing binary
    if (fp == NULL) return;

    Account temp;
    while (fread(&temp, sizeof(Account), 1, fp)) {
        if (strcmp(temp.acc_no, user.acc_no) == 0) {
            // Move file pointer backwards to start of this structural block
            fseek(fp, -sizeof(Account), SEEK_CUR);
            // Overwrite old block with updated properties
            fwrite(&user, sizeof(Account), 1, fp);
            break;
        }
    }
    fclose(fp);
}