#include <stdio.h>
#include <errno.h>
#include <string.h>

/*
 * FILE HANDLING BASICS
 * ====================
 * A file is a collection of related data stored on disk.
 * C provides a rich set of library functions for file I/O via <stdio.h>.
 *
 * FILE TYPES:
 *   - Text files   : Human-readable; newline translation may occur on some OSes.
 *   - Binary files : Raw byte-for-byte storage; no translation; portable across
 *                    programs when the same data types are used.
 *
 * FILE OPENING MODES:
 *   Mode   | Description
 *   -------|------------------------------------------------------------------
 *   "r"    | Open text file for reading.  File must exist.
 *   "w"    | Create/truncate text file for writing.
 *   "a"    | Open/create text file for appending (writes go to end).
 *   "r+"   | Open text file for reading AND writing.  File must exist.
 *   "w+"   | Create/truncate text file for reading AND writing.
 *   "a+"   | Open/create text file for reading AND appending.
 *   "rb"   | Binary read.
 *   "wb"   | Binary write (create/truncate).
 *   "ab"   | Binary append.
 *   "rb+"  | Binary read/write.  File must exist.
 *   "wb+"  | Binary read/write (create/truncate).
 *   "ab+"  | Binary read/append.
 *
 * FILE POINTER:
 *   FILE *fp  — opaque pointer to a FILE structure managed by the C library.
 *   Never manipulate the FILE structure directly; always use library functions.
 *
 * IMPORTANT RULES:
 *   1. Always check the return value of fopen() for NULL.
 *   2. Always fclose() every file you fopen().
 *   3. After an error, use perror() or strerror(errno) for diagnostics.
 */

/* Helper: print a labelled divider */
static void section(const char *title) {
    printf("\n=== %s ===\n", title);
}

int main() {

    printf("=== File Handling Basics ===\n");

    /* ------------------------------------------------------------------ */
    section("1. Text File: Write Mode (\"w\")");
    /* ------------------------------------------------------------------ */
    FILE *fp = fopen("example.txt", "w");
    if (fp == NULL) {
        perror("fopen example.txt");   /* prints system error message     */
        return 1;
    }
    fprintf(fp, "Line 1: Hello, File!\n");
    fprintf(fp, "Line 2: C file I/O is powerful.\n");
    fprintf(fp, "Line 3: Always close your files.\n");
    fclose(fp);
    printf("Written 3 lines to example.txt\n");

    /* ------------------------------------------------------------------ */
    section("2. Text File: Read Mode (\"r\")");
    /* ------------------------------------------------------------------ */
    fp = fopen("example.txt", "r");
    if (fp == NULL) {
        perror("fopen example.txt for reading");
        return 1;
    }
    char line[128];
    int lineNum = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("  [%d] %s", lineNum++, line);
    }
    /* Check whether loop ended on error or EOF */
    if (ferror(fp)) {
        fprintf(stderr, "Read error on example.txt\n");
    }
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("3. Text File: Append Mode (\"a\")");
    /* ------------------------------------------------------------------ */
    fp = fopen("example.txt", "a");
    if (fp == NULL) {
        perror("fopen example.txt for appending");
        return 1;
    }
    fprintf(fp, "Line 4: Appended after original content.\n");
    fclose(fp);
    printf("Appended line 4.  Total lines now:\n");

    fp = fopen("example.txt", "r");
    if (fp != NULL) {
        lineNum = 1;
        while (fgets(line, sizeof(line), fp) != NULL) {
            printf("  [%d] %s", lineNum++, line);
        }
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("4. Binary File: Write and Read (\"wb\" / \"rb\")");
    /* ------------------------------------------------------------------ */
    int scores[] = {95, 82, 78, 91, 65};
    int n = sizeof(scores) / sizeof(scores[0]);

    fp = fopen("score.bin", "wb");
    if (fp == NULL) {
        perror("fopen scores.bin for writing");
        return 1;
    }
    size_t written = fwrite(scores, sizeof(int), n, fp);
    fclose(fp);
    printf("Wrote %zu integers to scores.bin\n", written);

    fp = fopen("score.bin", "rb");
    if (fp == NULL) {
        perror("fopen scores.bin for reading");
        return 1;
    }
    int readScores[5];
    size_t count = fread(readScores, sizeof(int), n, fp);
    fclose(fp);
    printf("Read %zu integers from scores.bin: ", count);
    for (size_t i = 0; i < count; i++) {
        printf("%d ", readScores[i]);
    }
    printf("\n");

    /* ------------------------------------------------------------------ */
    section("5. Read+Write Mode (\"r+\")");
    /* ------------------------------------------------------------------ */
    /* Open existing example.txt and overwrite only the first character   */
    fp = fopen("example.txt", "r+");
    if (fp == NULL) {
        perror("fopen example.txt r+");
        return 1;
    }
    fputc('*', fp);   /* Replace first byte with '*' */
    fclose(fp);
    printf("Overwrote first character of example.txt with '*'\n");

    /* ------------------------------------------------------------------ */
    section("6. Error Handling: feof() and ferror()");
    /* ------------------------------------------------------------------ */
    fp = fopen("example.txt", "r");
    if (fp != NULL) {
        /* Read past end intentionally */
        while (!feof(fp)) {
            fgetc(fp);
        }
        printf("feof() = %d  (non-zero means end-of-file reached)\n", feof(fp));
        printf("ferror() = %d  (0 means no error)\n", ferror(fp));
        clearerr(fp);   /* Reset EOF and error flags */
        printf("After clearerr(): feof() = %d\n", feof(fp));
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("7. Summary of Key Functions");
    /* ------------------------------------------------------------------ */
    printf("  fopen()   — open / create a file\n");
    printf("  fclose()  — flush buffers and close file\n");
    printf("  fprintf() — formatted write to file\n");
    printf("  fscanf()  — formatted read from file\n");
    printf("  fgets()   — read a line (safe, length-limited)\n");
    printf("  fputs()   — write a string to file\n");
    printf("  fgetc()   — read one character\n");
    printf("  fputc()   — write one character\n");
    printf("  fread()   — read raw bytes (binary)\n");
    printf("  fwrite()  — write raw bytes (binary)\n");
    printf("  fseek()   — move file position indicator\n");
    printf("  ftell()   — get current file position\n");
    printf("  rewind()  — reset position to beginning\n");
    printf("  feof()    — test end-of-file indicator\n");
    printf("  ferror()  — test error indicator\n");
    printf("  clearerr()— clear EOF and error flags\n");
    printf("  remove()  — delete a file\n");
    printf("  rename()  — rename / move a file\n");
    printf("  perror()  — print system error message\n");

    /* Cleanup */
    remove("scores.bin");

    return 0;
}
