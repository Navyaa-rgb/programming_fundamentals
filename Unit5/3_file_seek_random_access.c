#include <stdio.h>
#include <string.h>

/*
 * FILE SEEK AND RANDOM ACCESS
 * ============================
 * Normally file I/O is sequential — you read/write from wherever the
 * "file position indicator" (FPI) currently sits.  The seek functions
 * let you move the FPI anywhere, enabling random (non-sequential) access.
 *
 * KEY FUNCTIONS:
 *   fseek(fp, offset, whence)  — move FPI by 'offset' bytes from 'whence'
 *   ftell(fp)                  — return current FPI as a long
 *   rewind(fp)                 — reset FPI to byte 0 (like fseek(fp,0,SEEK_SET))
 *
 * WHENCE VALUES:
 *   SEEK_SET (0)  — from the beginning of the file
 *   SEEK_CUR (1)  — from the current position
 *   SEEK_END (2)  — from the end of the file
 *
 * NOTE: For binary files fseek/ftell are fully portable.
 *       For text files on some systems only SEEK_SET with values returned
 *       by ftell() is guaranteed to work correctly.
 */

static void section(const char *title) {
    printf("\n=== %s ===\n", title);
}

/* Fixed-width record so we can index directly into the binary file */
typedef struct {
    int  id;
    char subject[20];
    int  marks;
} Record;

int main() {
    printf("=== File Seek and Random Access ===\n");

    /* ------------------------------------------------------------------ */
    section("1. ftell() — Get Current File Position");
    /* ------------------------------------------------------------------ */
    FILE *fp = fopen("seek_demo.txt", "w");
    if (fp == NULL) { perror("fopen seek_demo.txt"); return 1; }

    fprintf(fp, "ABCDEFGHIJ\n");   /* 11 bytes (10 chars + newline) */
    fprintf(fp, "1234567890\n");   /* another 11 bytes               */
    long posAfterWrite = ftell(fp);
    printf("ftell() after writing 2 lines: %ld bytes\n", posAfterWrite);
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("2. fseek() with SEEK_SET — Jump to Absolute Position");
    /* ------------------------------------------------------------------ */
    fp = fopen("seek_demo.txt", "r");
    if (fp == NULL) { perror("fopen seek_demo.txt"); return 1; }

    /* Jump to byte 5 (0-based) and read one character */
    fseek(fp, 5, SEEK_SET);
    printf("Byte 5 (SEEK_SET): '%c'  (expected 'F')\n", fgetc(fp));

    /* Jump to byte 0 — start of file */
    fseek(fp, 0, SEEK_SET);
    printf("Byte 0 (SEEK_SET): '%c'  (expected 'A')\n", fgetc(fp));

    /* ------------------------------------------------------------------ */
    section("3. fseek() with SEEK_CUR — Relative Move");
    /* ------------------------------------------------------------------ */
    rewind(fp);                    /* go back to start */
    fgetc(fp);                     /* read 'A', FPI now at byte 1 */
    fseek(fp, 3, SEEK_CUR);        /* skip 3 bytes: 'B','C','D' */
    printf("After SEEK_CUR +3 from byte 1: '%c'  (expected 'E')\n", fgetc(fp));

    fseek(fp, -2, SEEK_CUR);       /* step back 2 bytes */
    printf("After SEEK_CUR -2: '%c'  (expected 'D')\n", fgetc(fp));

    /* ------------------------------------------------------------------ */
    section("4. fseek() with SEEK_END — Relative to End");
    /* ------------------------------------------------------------------ */
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    printf("File size (SEEK_END offset 0): %ld bytes\n", fileSize);

    fseek(fp, -12, SEEK_END);      /* 12 bytes before end = start of line 2 */
    printf("12 bytes before EOF: '");
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n') putchar(c);
    printf("'  (expected '1234567890')\n");

    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("5. rewind() — Reset to Beginning");
    /* ------------------------------------------------------------------ */
    fp = fopen("seek_demo.txt", "r");
    if (fp == NULL) { perror("fopen seek_demo.txt"); return 1; }

    /* Advance, then rewind */
    fseek(fp, 8, SEEK_SET);
    printf("Before rewind, ftell() = %ld\n", ftell(fp));
    rewind(fp);
    printf("After  rewind, ftell() = %ld  (should be 0)\n", ftell(fp));
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("6. Random Access on Binary Records");
    /* ------------------------------------------------------------------ */
    /*
     * Because each Record is the same size, record[i] starts at byte
     *   i * sizeof(Record)
     * from the beginning of the file — true O(1) random access.
     */
    Record records[] = {
        {101, "Mathematics", 88},
        {102, "Physics",     76},
        {103, "Chemistry",   92},
        {104, "Biology",     81},
        {105, "English",     95}
    };
    int n = sizeof(records) / sizeof(records[0]);

    fp = fopen("records.bin", "wb");
    if (fp == NULL) { perror("fopen records.bin"); return 1; }
    fwrite(records, sizeof(Record), n, fp);
    fclose(fp);
    printf("Wrote %d records to records.bin\n", n);

    /* Read record at index 2 (Chemistry) directly */
    fp = fopen("records.bin", "rb");
    if (fp == NULL) { perror("fopen records.bin"); return 1; }

    int idx = 2;
    fseek(fp, idx * (long)sizeof(Record), SEEK_SET);
    Record r;
    fread(&r, sizeof(Record), 1, fp);
    printf("Direct read record[%d]: ID=%d  Subject=%-12s  Marks=%d\n",
           idx, r.id, r.subject, r.marks);

    /* Read all records sequentially to verify */
    printf("\nAll records (sequential read):\n");
    rewind(fp);
    while (fread(&r, sizeof(Record), 1, fp) == 1) {
        printf("  ID=%-4d  Subject=%-12s  Marks=%d\n",
               r.id, r.subject, r.marks);
    }
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("7. Update a Record In-Place (rb+)");
    /* ------------------------------------------------------------------ */
    fp = fopen("records.bin", "rb+");
    if (fp == NULL) { perror("fopen records.bin rb+"); return 1; }

    /* Update record[1] (Physics) marks from 76 to 85 */
    idx = 1;
    fseek(fp, idx * (long)sizeof(Record), SEEK_SET);
    fread(&r, sizeof(Record), 1, fp);
    printf("\nBefore update: ID=%d  Marks=%d\n", r.id, r.marks);

    r.marks = 85;
    fseek(fp, idx * (long)sizeof(Record), SEEK_SET);  /* rewind to record start */
    fwrite(&r, sizeof(Record), 1, fp);
    printf("After  update: ID=%d  Marks=%d\n", r.id, r.marks);

    /* Verify */
    fseek(fp, idx * (long)sizeof(Record), SEEK_SET);
    fread(&r, sizeof(Record), 1, fp);
    printf("Verify from file: ID=%d  Subject=%s  Marks=%d\n",
           r.id, r.subject, r.marks);
    fclose(fp);

    /* ------------------------------------------------------------------ */
    section("8. Summary");
    /* ------------------------------------------------------------------ */
    printf("  fseek(fp, offset, SEEK_SET) — seek from file start\n");
    printf("  fseek(fp, offset, SEEK_CUR) — seek relative to current position\n");
    printf("  fseek(fp, offset, SEEK_END) — seek relative to file end\n");
    printf("  ftell(fp)                   — return current byte offset\n");
    printf("  rewind(fp)                  — equivalent to fseek(fp, 0, SEEK_SET)\n");
    printf("  Fixed-size records enable O(1) random access: offset = i*sizeof(T)\n");

    /* Cleanup */
    remove("seek_demo.txt");
    remove("records.bin");

    return 0;
}
