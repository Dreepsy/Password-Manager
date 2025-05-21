#include <stdio.h>
#include <string.h>

// ---Password struct---
struct Password 
{
    int P_ID;
    char Name[100];
    char Password[100];
};

// ---Function prototypes---
void showMainMenu();
void viewPasswords();
void addNewPassword();
int countPasswords();

// ---Global variable---
int passwordCount = 0;  //Tracks how many passwords are in the list

// ---Main---
int main() 
{
    passwordCount = countPasswords();
    showMainMenu();  //Call the main menu
    return 0;
}

// ---Show main menu---
void showMainMenu() 
{
    int choice;

    while (1) {
        printf("\n=== Password Manager ===\n");
        printf("1. Add new password\n");
        printf("2. Show entries\n");
        printf("3. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);

        switch (choice) 
        {
            case 1:
                addNewPassword();
                break;
            case 2:
                viewPasswords();
                break;
            case 3:
                printf("Exiting the program.\n");
                return;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }
}

// ---Add a new password---
void addNewPassword() 
{
    struct Password entry;

    entry.P_ID = passwordCount + 1;

    //Get user input
    printf("Enter name for the password (e.g., Google): ");
    scanf("%s", entry.Name);

    printf("Enter the password: ");
    scanf("%s", entry.Password);

    //Save to file
    FILE *fptr = fopen("Password-Manager.txt", "a");
    if (fptr == NULL) 
    {
        printf("Error opening file.\n");
        return;
    }

    fprintf(fptr, "%d. %s: %s\n", entry.P_ID, entry.Name, entry.Password);
    fclose(fptr);

    printf("Password saved successfully.\n");

    passwordCount++;  //Update count for next ID
}

// ---View saved passwords---
void viewPasswords() 
{
    FILE *fptr = fopen("Password-Manager.txt", "r");

    if (fptr == NULL) 
    {
        printf("No passwords found.\n");
        return;
    }

    char line[256];
    printf("\n=== Saved Passwords ===\n");
    while (fgets(line, sizeof(line), fptr)) 
    {
        printf("%s", line);
    }
    fclose(fptr);
}

// ---Count lines (passwords) in the file---
int countPasswords() 
{
    FILE *fptr = fopen("Password-Manager.txt", "r");

    if (fptr == NULL) 
    {
        // File doesn't exist yet
        return 0;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), fptr)) 
    {
        count++;
    }

    fclose(fptr);
    return count;
}
