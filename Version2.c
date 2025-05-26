#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// ---Configuration---
#define SALT_LEN       16                       //defines the length of the salt to be used
#define HASH_LEN       32                       //defines the lenght of the output hash
#define PBKDF2_ITERS 100000                     //defines how many times the password-based key derivation function (PBKDF2) should irterate
#define MASTER_FILE  "vault.auth"               //defines the filenamewhere the master password hash and salt are stored
#define DB_FILE      "Password-Manager.txt"     //defines the filename where the actual passwords are saved

// ---RSA helpers & globals---
unsigned long long RSA_N, RSA_E, RSA_D;
unsigned long long mod_pow(unsigned long long base, unsigned long long exp, unsigned long long mod)     //computes (base^exp)mod
{
    unsigned long long result = 1;
    base %= mod;
    while (exp > 0)
    {
        if (exp & 1) result = (result * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}
unsigned long long gcd(unsigned long long a, unsigned long long b)      //calculates the gcd out of our two prime numbers
{
    while (b)
    {
        unsigned long long t = b; b = a % b; a = t;
    }
    return a;
}
long long mod_inverse(long long e, long long phi)       //calculates the mod_inverse
{
    long long t = 0, newt = 1;
    long long r = phi, newr = e;
    while (newr)
    {
        long long q = r / newr;
        long long tmp = newt; newt = t - q * newt; t = tmp;
        tmp = newr; newr = r - q * newr; r = tmp;
    }
    if (r > 1) return -1;
    if (t < 0) t += phi;
    return t;
}
void setup_rsa()
{
    //small-demo primes
    unsigned long long p = 61, q = 53;
    RSA_N = p * q;
    unsigned long long phi = (p - 1) * (q - 1);
    RSA_E = 17;
    RSA_D = mod_inverse(RSA_E, phi);
    if (RSA_D == (unsigned long long)-1) {
        fprintf(stderr, "RSA key gen failed\n");
        exit(1);
    }
}
void encrypt_password(const char *input, char *output)      //encryptes the string 
{
    output[0] = '\0';
    for (size_t i = 0; input[i]; i++)
    {
        unsigned long long c = mod_pow((unsigned long long)input[i], RSA_E, RSA_N);
        char buf[32];
        sprintf(buf, "%llu ", c);
        strcat(output, buf);
    }
}
void decrypt_password(const char *input, char *output)      //decrypt the string
{
    size_t idx = 0;
    unsigned long long num;
    const char *p = input;
    while (sscanf(p, "%llu", &num) == 1)
    {
        output[idx++] = (char)mod_pow(num, RSA_D, RSA_N);
        while (*p && *p != ' ') p++;
        while (*p == ' ') p++;
    }
    output[idx] = '\0';
}

// ---Master-password (vault auth) globals & helpers---
unsigned char master_salt[SALT_LEN];
unsigned char master_hash[HASH_LEN];

void to_hex(const unsigned char *in, int len, char *out)        //turns a binary byte array into a hexadecimal string
{
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < len; i++)
    {
        out[2*i]   = hex[(in[i] >> 4) & 0xF];
        out[2*i+1] = hex[in[i] & 0xF];
    }
    out[2*len] = '\0';
}
void hex_to_bytes(const char *in, unsigned char *out, int len)
{
    char buf[3] = {0};
    for (int i = 0; i < len; i++)
    {
        buf[0] = in[2*i];
        buf[1] = in[2*i+1];
        out[i] = (unsigned char)strtol(buf, NULL, 16);
    }
}
int derive_master_hash(const char *pw, const unsigned char *salt, unsigned char *out)
{
    return PKCS5_PBKDF2_HMAC(
        pw, strlen(pw),
        salt, SALT_LEN,
        PBKDF2_ITERS,
        EVP_sha256(),
        HASH_LEN, out
    );
}
void prompt_password(const char *prompt, char *buf, size_t sz)
{
    printf("%s", prompt); fflush(stdout);
    system("stty -echo");
    if (!fgets(buf, sz, stdin)) buf[0] = '\0';
    system("stty echo");
    printf("\n");
    buf[strcspn(buf, "\n")] = '\0';
}
void save_master_file()
{
    char sh[2*SALT_LEN+1], hh[2*HASH_LEN+1];
    to_hex(master_salt, SALT_LEN, sh);
    to_hex(master_hash, HASH_LEN, hh);
    FILE *f = fopen(MASTER_FILE, "w");
    if (!f)
    {
        perror("fopen master"); exit(1);
    }
    fprintf(f, "%s:%s\n", sh, hh);
    fclose(f);
}
void load_master_file()
{
    char sh[2*SALT_LEN+1], hh[2*HASH_LEN+1];
    FILE *f = fopen(MASTER_FILE, "r");
    if (!f)
    {
        fprintf(stderr, "Cannot open %s\n", MASTER_FILE); exit(1);
    }
    if (fscanf(f, "%32[a-f0-9]:%64[a-f0-9]\n", sh, hh) != 2)
    {
        fprintf(stderr, "Corrupt %s\n", MASTER_FILE); exit(1);
    }
    fclose(f);
    hex_to_bytes(sh, master_salt, SALT_LEN);
    hex_to_bytes(hh, master_hash, HASH_LEN);
}
void setup_master_password()
{
    char p1[128], p2[128];
    printf("=== Create Vault Password ===\n");
    prompt_password("Enter new vault password: ", p1, sizeof(p1));
    prompt_password("Confirm vault password: ",    p2, sizeof(p2));
    if (strcmp(p1, p2) != 0)
    {
        fprintf(stderr, "Passwords do not match.\n");
        exit(1);
    }
    if (!RAND_bytes(master_salt, SALT_LEN) ||
        !derive_master_hash(p1, master_salt, master_hash))
    {
        fprintf(stderr, "Master hash error.\n");
        exit(1);
    }
    save_master_file();
    printf("Vault password set!\n\n");
}
void verify_master_password()
{
    load_master_file();
    char p[128];
    prompt_password("Enter vault password: ", p, sizeof(p));
    unsigned char test[HASH_LEN];
    derive_master_hash(p, master_salt, test);
    if (CRYPTO_memcmp(test, master_hash, HASH_LEN) != 0)
    {
        fprintf(stderr, "Incorrect vault password.\n");
        exit(1);
    }
}

// ---Password-entry struct & prototypes---
struct Password
{
    int  P_ID;
    char Name[100];
    char Password[1000];
};

void showMainMenu();
void viewPasswords();
void addNewPassword();
void changePassword();
int countPasswords();

// ---Count existing entries---
int countPasswords()
{
    FILE *f = fopen(DB_FILE, "r");
    if (!f) return 0;
    int c = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) c++;
    fclose(f);
    return c;
}

// ---Add new RSA‐encrypted entry---
void addNewPassword()
{
    struct Password e;
    e.P_ID = countPasswords() + 1;
    printf("Enter label: "); scanf("%99s", e.Name);
    char plain[128];
    prompt_password("Enter entry password: ", plain, sizeof(plain));
    encrypt_password(plain, e.Password);

    FILE *f = fopen(DB_FILE, "a");
    if (!f)
    {
        perror("fopen");
        return;
    }
    fprintf(f, "%d. %s: %s\n", e.P_ID, e.Name, e.Password);
    fclose(f);
    printf("Saved entry %d.\n", e.P_ID);
}

// ---Show all entries (decrypt each)---
void viewPasswords()
{
    FILE *f = fopen(DB_FILE, "r");
    if (!f)
    {
        printf("No entries.\n");
        return;
    }
    printf("\n=== Stored Passwords ===\n");
    char line[1024], dec[1000];
    int id;
    char name[100], blob[1000];
    while (fgets(line, sizeof(line), f))
    {
        if (sscanf(line, "%d. %99[^:]: %[^\n]", &id, name, blob) == 3)
        {
            decrypt_password(blob, dec);
            printf("%d. %s: %s\n", id, name, dec);
        }
    }
    fclose(f);
}

// ---Change an existing entry---
void changePassword()
{
    struct Password list[200];
    int n = 0;
    FILE *f = fopen(DB_FILE, "r");
    if (!f)
    {
        printf("No entries to edit.\n");
        return;
    }
    char line[1024];
    while (fgets(line, sizeof(line), f) && n < 200)
    {
        if (sscanf(line, "%d. %99[^:]: %[^\n]",
                   &list[n].P_ID,
                   list[n].Name,
                   list[n].Password) == 3)
                   {
            n++;
        }
    }
    fclose(f);

    //show with decrypted
    printf("\n=== Entries ===\n");
    char dec[1000];
    for (int i = 0; i < n; i++)
    {
        decrypt_password(list[i].Password, dec);
        printf("%d. %s: %s\n", list[i].P_ID, list[i].Name, dec);
    }

    printf("\nID to change: ");
    int target; scanf("%d", &target);
    for (int i = 0; i < n; i++)
    {
        if (list[i].P_ID == target)
        {
            char plain[128], newblob[1000];
            prompt_password("Enter new password: ", plain, sizeof(plain));
            encrypt_password(plain, newblob);
            strcpy(list[i].Password, newblob);
            break;
        }
    }

    //write back all
    f = fopen(DB_FILE, "w");
    if (!f)
    {
        perror("fopen"); return;
    }
    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%d. %s: %s\n",
                list[i].P_ID,
                list[i].Name,
                list[i].Password);
    }
    fclose(f);
    printf("Updated entry %d.\n", target);
}

// ---Main menu loop---
void showMainMenu()
{
    int choice;
    while (1)
    {
        printf(
            "\n=== Vault Menu ===\n"
            "1) Add new password\n"
            "2) Show entries\n"
            "3) Change password\n"
            "4) Exit\n"
            "Choose: "
        );
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
                printf("Goodbye!\n");
                return;
            default:
            printf("Invalid choice.\n");
        }
    }
}

// ---Entry point---
int main()
{
    OpenSSL_add_all_algorithms();

    // 1) Master-password protection
    if (access(MASTER_FILE, F_OK) != 0)
        setup_master_password();
    else
        verify_master_password();

    // 2) Initialize RSA and enter menu
    setup_rsa();
    showMainMenu();
    return 0;
}