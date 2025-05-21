#include <stdio.h>
#include <string.h>


// ---Password Struct---
struct Password 
{
    int P_ID;               //Password ID
    char Name[100];         //Name of the Entry
    char Password[100];     //The Password

};

// ---Main---
int main()
{

    //generating a new entry
    struct Password myPassword;

    printf("Please enter the name of the password:", myPassword.Name);
    scanf("%s", &myPassword.Name);

    printf("Please enter the password:", myPassword.Password);
    scanf("%s", &myPassword.Password);

    //setting values
    myPassword.P_ID = 1;

    FILE *fptr = fopen("Password-Manager.txt", "w");    //creating a file
    fprintf(fptr, "%d. %s: %s\n", myPassword.P_ID, myPassword.Name, myPassword.Password);   //writing into the file
    fclose(fptr);   //closing the file

    //test output
    printf("P_ID: %d. %s: %s\n", myPassword.P_ID, myPassword.Name, myPassword.Password);



    return 0;
}
