#include <stdio.h>
#include <string.h>

/*
 * FILE I/O OPERATIONS
 * ===================
 * This file demonstrates all major text and binary I/O functions:
 *
 *   CHARACTER I/O : fgetc(), fputc()
 *   STRING I/O    : fgets(), fputs()
 *   FORMATTED I/O : fprintf(), fscanf()
 *   BINARY I/O    : fread(), fwrite()
 *   STDIN/STDOUT  : getchar(), putchar(), puts()
 *   FILE UTILITY  : remove(), rename()
 */

static void section(const char *title) {
    printf("\n=== %s ===\n", title);
}

int main() {
    FILE *fp;
    char buffer[256];

    printf("=== File I/O Operations ===\n");

    /* ------------------------------------------------------------------ */
    section("1. fprintf() — Formatted Write to File");
    /* ------------------------------------------------------------------ */
    fp = fopen("data.txt", "w");
    if (fp == NULL) { perror("fopen data.txt"); return 1; }

    fprintf(fp, "Name: Alice\n");
    fprintf(fp, "Age:  %d\n", 30);
    fprintf(fp, "GPA:  %.2f\n", 3.85);
    fprintf(fp, "Pass: %s\n", 3.85 >= 2.0 ? "Yes" : "No");
    fclose(fp);
    printf("Wrote formatted data to data.txt\n");

    /* ------------------------------------------------------------------ */
    section("2. fscanf() — Formatted Read from File");
    /* ------------------------------------------------------------------ */
    fp = fopen("data.txt", "r");
    if (fp == NULL) { perror("fopen data.txt"); return 1; }

    char key[32], name[32], passStr[8];
    int  age;
    float gpa;

    /* Read "Name: Alice" */
    fscanf(fp, "%s %s", key, name);          /* key="Name:", name="Alice" */
    /* Read "Age:  30"   */
    fscanf(fp, "%s %d", key, &age);
    /* Read "GPA:  3.85" */
    fscanf(fp, "%s %f", key, &gpa);
    /* Read "Pass: Yes"  */
    fscanf(fp, "%s %s", key, passStr);
    fclose(fp);

    printf("fscanf read — Name: %s, Age: %d, GPA: %.2f, Pass: %s\n",
           name, age, gpa, passStr);

    /* ------------------------------------------------------------------ */
    section("3. fgets() — Safe Line Read");
    /* ------------------------------------------------------------------ */
    fp = fopen("data.txt", "r");
    if (fp == NULL) { perror("fopen data.txt"); return 1; }

    printf("Reading data.txt line by line with fgets():\n");
    int lineNum = 1;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        /* Strip trailing newline for cleaner display */
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("  Line %d: %s\n", lineNum++, buffer);
    }
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("4. fputs() — Write String to File");
    /* ------------------------------------------------------------------ */
    fp = fopen("notes.txt", "w");
    if (fp == NULL) { perror("fopen notes.txt"); return 1; }

    fputs("fputs writes a string without printf-style formatting.\n", fp);
    fputs("It does NOT add a newline automatically.\n", fp);
    fputs("You must include \\n yourself if needed.\n", fp);
    fclose(fp);
    printf("Wrote 3 lines to notes.txt with fputs()\n");

    /* ------------------------------------------------------------------ */
    section("5. fgetc() / fputc() — Character-by-Character I/O");
    /* ------------------------------------------------------------------ */
    /* Count vowels in data.txt using fgetc */
    fp = fopen("data.txt", "r");
    if (fp == NULL) { perror("fopen data.txt"); return 1; }

    int c, vowels = 0, totalChars = 0;
    while ((c = fgetc(fp)) != EOF) {
        totalChars++;
        char lc = (char)(c | 32);   /* cheap tolower for ASCII letters */
        if (lc == 'a' || lc == 'e' || lc == 'i' || lc == 'o' || lc == 'u') {
            vowels++;
        }
    }
    fclose(fp);
    printf("data.txt — total chars: %d, vowels: %d\n", totalChars, vowels);

    /* Copy file character by character */
    FILE *src = fopen("data.txt", "r");
    FILE *dst = fopen("data_copy.txt", "w");
    if (src && dst) {
        while ((c = fgetc(src)) != EOF) {
            fputc(c, dst);
        }
        fclose(src);
        fclose(dst);
        printf("Copied data.txt -> data_copy.txt with fgetc/fputc\n");
    }

    /* ------------------------------------------------------------------ */
    section("6. fwrite() / fread() — Binary I/O");
    /* ------------------------------------------------------------------ */
    double pi_vals[] = {3.14159, 2.71828, 1.61803, 1.41421, 1.73205};
    int    n         = sizeof(pi_vals) / sizeof(pi_vals[0]);

    fp = fopen("constants.bin", "wb");
    if (fp == NULL) { perror("fopen constants.bin"); return 1; }
    size_t nw = fwrite(pi_vals, sizeof(double), n, fp);
    fclose(fp);
    printf("fwrite: wrote %zu doubles to constants.bin\n", nw);

    double readVals[5];
    fp = fopen("constants.bin", "rb");
    if (fp == NULL) { perror("fopen constants.bin"); return 1; }
    size_t nr = fread(readVals, sizeof(double), n, fp);
    fclose(fp);
    printf("fread:  read %zu doubles: ", nr);
    for (size_t i = 0; i < nr; i++) {
        printf("%.5f ", readVals[i]);
    }
    printf("\n");

    /* ------------------------------------------------------------------ */
    section("7. putchar() / getchar() — Console Character I/O");
    /* ------------------------------------------------------------------ */
    printf("putchar: spelling 'C' -> ");
    putchar('C'); putchar('\n');

    /* Note: getchar() blocks for user input; shown here for reference.
     * Uncomment to run interactively:
     *
     *   printf("Enter a character: ");
     *   int ch = getchar();
     *   printf("You entered: ");
     *   putchar(ch);
     *   putchar('\n');
     */
    printf("(getchar() skipped to keep output non-interactive)\n");

    /* ------------------------------------------------------------------ */
    section("8. puts() — Write String + Newline to stdout");
    /* ------------------------------------------------------------------ */
    puts("puts() automatically appends a newline character.");
    puts("This is a second line printed with puts().");

    /* ------------------------------------------------------------------ */
    section("9. remove() and rename()");
    /* ------------------------------------------------------------------ */
    /* Create a temp file, rename it, then delete it */
    fp = fopen("temp_old.txt", "w");
    if (fp) { fputs("temporary\n", fp); fclose(fp); }

    if (rename("temp_old.txt", "temp_new.txt") == 0) {
        printf("rename(): temp_old.txt -> temp_new.txt  [OK]\n");
    } else {
        perror("rename");
    }

    if (remove("temp_new.txt") == 0) {
        printf("remove(): temp_new.txt deleted  [OK]\n");
    } else {
        perror("remove");
    }

    /* ------------------------------------------------------------------ */
    section("10. Summary");
    /* ------------------------------------------------------------------ */
    printf("  fprintf/fscanf  — formatted text I/O (like printf/scanf but to file)\n");
    printf("  fgets/fputs     — line/string I/O (fgets is safer than fscanf for strings)\n");
    printf("  fgetc/fputc     — single-character I/O\n");
    printf("  fread/fwrite    — raw binary block I/O\n");
    printf("  getchar/putchar — character I/O on stdin/stdout\n");
    printf("  puts            — string + newline to stdout\n");
    printf("  remove/rename   — file management\n");

    /* Cleanup */
    remove("notes.txt");
    remove("data_copy.txt");
    remove("constants.bin");

    return 0;
}
