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
void changePassword();
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
        printf("3. Change Password\n");
        printf("4. Exit\n");
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
                changePassword();
                break;

            case 4:
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

// ---Count lines in the file---
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

void changePassword()
{
    struct Password list[100];
    int count = 0;

    // Open the file for reading
    FILE *fptr = fopen("Password-Manager.txt", "r");
    if (fptr == NULL)
    {
        printf("No passwords to edit!\n");
        return;
    }

    // Load all entries from the file into the array
    char line[256];
    while (fgets(line, sizeof(line), fptr))
    {
        if (sscanf(line, "%d. %99[^:]: %99[^\n]", &list[count].P_ID, list[count].Name, list[count].Password) == 3)
        {
            count++;
        }
    }
    fclose(fptr);

    // Display entries
    printf("\n=== Password Entries ===\n");
    for (int i = 0; i < count; i++)
    {
        printf("%d. %s: %s\n", list[i].P_ID, list[i].Name, list[i].Password);
    }

    // Ask user which ID to update
    int targetID;
    printf("\nEnter the ID of the password you would like to change: ");
    scanf("%d", &targetID);

    // Search for the entry with the matching ID
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (list[i].P_ID == targetID)
        {
            printf("Enter new password for %s: ", list[i].Name);
            scanf("%s", list[i].Password);  // directly store the new password
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("No entry found with ID %d.\n", targetID);
        return;
    }

    // Rewrite the file with updated data
    fptr = fopen("Password-Manager.txt", "w");
    if (fptr == NULL)
    {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        fprintf(fptr, "%d. %s: %s\n", list[i].P_ID, list[i].Name, list[i].Password);
    }

    fclose(fptr);
    printf("Password updated successfully.\n");
}