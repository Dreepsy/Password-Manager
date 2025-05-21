#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ---Password struct---
//Structure to store each password entry
struct Password 
{
    int P_ID;                  //Unique ID for each password entry
    char Name[100];            //Name/label for the password
    char Password[1000];       //Encrypted password string
};

// ---Function prototypes---
void showMainMenu();           //Displays the main menu and handles user choices
void viewPasswords();          //Shows all saved passwords (decrypted)
void addNewPassword();         //Adds a new password entry
void changePassword();         //Allows user to change an existing password
int countPasswords();          //Counts the number of password entries in the file

// ---RSA functions---
unsigned long long mod_pow(unsigned long long base, unsigned long long exp, unsigned long long mod);//Modular exponentiation
unsigned long long gcd(unsigned long long a, unsigned long long b);                                 //Greatest common divisor
long long mod_inverse(long long e, long long phi);                                                  //Modular inverse for RSA
void setup_rsa();                                                                                   //Initializes RSA keys
void encrypt_password(const char *input, char *output);                                             //Encrypts a password
void decrypt_password(const char *input, char *output);                                             //Decrypts a password

// ---Global variables---
int passwordCount = 0;                  //Tracks the number of passwords
unsigned long long RSA_N, RSA_E, RSA_D; //RSA key components

// ---Main---
//Entry point of the program
int main() 
{
    setup_rsa();                      //Generate RSA keys
    passwordCount = countPasswords(); //Count existing passwords
    showMainMenu();                   //Start the main menu loop
    return 0;
}

// ---Show main menu---
//Displays the main menu and processes user input
void showMainMenu() 
{
    int choice;

    while (1) 
    {
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
                addNewPassword();     //Add a new password
                break;
            case 2:
                viewPasswords();      //View all passwords
                break;
            case 3:
                changePassword();     //Change an existing password
                break;
            case 4:
                printf("Exiting the program.\n");
                return;               //Exit the program
            default:
                printf("Invalid option. Please try again.\n");
        }
    }
}

// ---Add a new password---
//Prompts user for a new password and saves it
void addNewPassword() 
{
    struct Password entry;
    char encrypted[1000];

    entry.P_ID = passwordCount + 1; //Assign next available ID

    printf("Enter name for the password (e.g., Google): ");
    scanf("%s", entry.Name);

    char plain[100];
    printf("Enter the password: ");
    scanf("%s", plain);

    encrypt_password(plain, encrypted);      //Encrypt the password
    strcpy(entry.Password, encrypted);       //Store encrypted password

    //Open file for appending
    FILE *fptr = fopen("Password-Manager.txt", "a");
    
    //Error handling
    if (fptr == NULL) 
    {
        printf("Error opening file.\n");
        return;
    }

    //Write the new entry to file
    fprintf(fptr, "%d. %s: %s\n", entry.P_ID, entry.Name, entry.Password);
    fclose(fptr);

    printf("Password saved successfully.\n");
    passwordCount++; //Increment password count
}

// ---View saved passwords---
//Reads all password entries from file, decrypts, and displays them
void viewPasswords() 
{
    //Open file for reading
    FILE *fptr = fopen("Password-Manager.txt", "r");
   
    //Error handling
    if (fptr == NULL) 
    {
        printf("No passwords found.\n");
        return;
    }

    char line[1024];
    int id;
    char name[100], encrypted[1000], decrypted[100];

    printf("\n=== Saved Passwords ===\n");
    //Read each line and parse the entry
    while (fgets(line, sizeof(line), fptr)) 
    {
        //Parse line into ID, Name, and Encrypted Password
        if (sscanf(line, "%d. %99[^:]: %[^\n]", &id, name, encrypted) == 3)
        {
            decrypt_password(encrypted, decrypted); //Decrypt password
            printf("%d. %s: %s\n", id, name, decrypted); //Display entry
        }
    }
    fclose(fptr);
}

// ---Count lines in the file---
//Returns the number of password entries in the file
int countPasswords() 
{
    //Open file for reading
    FILE *fptr = fopen("Password-Manager.txt", "r");
    
    // Error handling
    if (fptr == NULL)
    {
        return 0;
    }

    int count = 0;
    char line[256];

    //Count each line
    while (fgets(line, sizeof(line), fptr))
    {
        count++;
    } 

    fclose(fptr);
    return count;
}

// ---Change an existing password---
//Allows user to select an entry by ID and update its password
void changePassword()
{
    struct Password list[100]; //Array to hold all entries
    int count = 0;

    //Open file for reading
    FILE *fptr = fopen("Password-Manager.txt", "r");
    if (fptr == NULL)
    {
        printf("No passwords to edit!\n");
        return;
    }

    char line[1024];
    //Read all entries into the list array
    while (fgets(line, sizeof(line), fptr))
    {
        if (sscanf(line, "%d. %99[^:]: %[^\n]", &list[count].P_ID, list[count].Name, list[count].Password) == 3)
        {
            count++;
        }
    }
    fclose(fptr);

    char decrypted[100];
    printf("\n=== Password Entries ===\n");
    //Display all entries with decrypted passwords
    for (int i = 0; i < count; i++)
    {
        decrypt_password(list[i].Password, decrypted);
        printf("%d. %s: %s\n", list[i].P_ID, list[i].Name, decrypted);
    }

    int targetID;
    printf("\nEnter the ID of the password you would like to change: ");
    scanf("%d", &targetID);

    int found = 0;
    //Search for the entry with the given ID
    for (int i = 0; i < count; i++)
    {
        if (list[i].P_ID == targetID)
        {
            char new_plain[100], encrypted[1000];
            printf("Enter new password for %s: ", list[i].Name);
            scanf("%s", new_plain);
            encrypt_password(new_plain, encrypted);      //Encrypt new password
            strcpy(list[i].Password, encrypted);         //Update entry
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("No entry found with ID %d.\n", targetID);
        return;
    }

    //Open file for writing (overwrite)
    fptr = fopen("Password-Manager.txt", "w");
    if (fptr == NULL)
    {
        printf("Error opening file for writing.\n");
        return;
    }

    //Write all entries back to file
    for (int i = 0; i < count; i++)
    {
        fprintf(fptr, "%d. %s: %s\n", list[i].P_ID, list[i].Name, list[i].Password);
    }

    fclose(fptr);
    printf("Password updated successfully.\n");
}

// ---RSA Functions---

//Calculates the greatest common divisor
unsigned long long gcd(unsigned long long a, unsigned long long b) 
{
    while (b != 0) 
    {
        unsigned long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

//Calculates the modular inverse (used for RSA private key)
long long mod_inverse(long long e, long long phi) 
{
    long long t = 0, newt = 1;
    long long r = phi, newr = e;

    while (newr != 0) 
    {
        long long quotient = r / newr;
        long long temp = newt;
        newt = t - quotient * newt;
        t = temp;

        temp = newr;
        newr = r - quotient * newr;
        r = temp;
    }

    if (r > 1) return -1; //No inverse exists
    if (t < 0) t += phi;
    return t;
}

//Performs modular exponentiation (base^exp % mod)
unsigned long long mod_pow(unsigned long long base, unsigned long long exp, unsigned long long mod) 
{
    unsigned long long result = 1;
    base = base % mod;

    while (exp > 0) 
    {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }

    return result;
}

//Initializes RSA keys (public and private)
void setup_rsa() 
{
    // Small primes for demonstration
    unsigned long long p = 61, q = 53;
    RSA_N = p * q;                              //Modulus
    unsigned long long phi = (p - 1) * (q - 1); //Euler's totient
    RSA_E = 17;                                 //Public exponent
    RSA_D = mod_inverse(RSA_E, phi);            //Private exponent
    if (RSA_D == -1) {
        printf("RSA key generation failed.\n");
        exit(1);
    }
}

//Encrypts a password using RSA
void encrypt_password(const char *input, char *output) 
{
    char buffer[20];
    output[0] = '\0';

    for (int i = 0; input[i] != '\0'; i++) 
    {
        unsigned long long encrypted = mod_pow((unsigned long long)input[i], RSA_E, RSA_N);
        sprintf(buffer, "%llu ", encrypted); //Store encrypted value as string
        strcat(output, buffer);              //Append to output
    }
}

//Decrypts an RSA-encrypted password back to plain text
void decrypt_password(const char *input, char *output) 
{
    unsigned long long num;
    int index = 0;
    const char *ptr = input;
    output[0] = '\0';

    //Read each encrypted number, decrypt, and convert to char
    while (sscanf(ptr, "%llu", &num) == 1) 
    {
        output[index++] = (char)mod_pow(num, RSA_D, RSA_N);
        while (*ptr != ' ' && *ptr != '\0') ptr++; //Move to next number
        while (*ptr == ' ') ptr++;
    }
    output[index] = '\0'; //Null-terminate the string
}