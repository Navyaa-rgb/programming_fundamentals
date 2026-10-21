#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*
 * FILE ERROR HANDLING
 * ====================
 * Robust file I/O requires checking and handling errors at every step.
 * C provides several mechanisms for this:
 *
 *   errno          — global integer set by library functions on failure
 *   perror(msg)    — print msg + ": " + system error string to stderr
 *   strerror(errno)— return the human-readable error string for errno
 *   feof(fp)       — non-zero if the end-of-file indicator is set
 *   ferror(fp)     — non-zero if the error indicator is set
 *   clearerr(fp)   — clear both EOF and error indicators
 *
 * COMMON errno VALUES (defined in <errno.h>):
 *   ENOENT   — No such file or directory
 *   EACCES   — Permission denied
 *   EEXIST   — File exists
 *   ENOSPC   — No space left on device
 *   EINVAL   — Invalid argument
 *   EMFILE   — Too many open files
 *
 * BEST PRACTICES:
 *   1. Always check fopen() for NULL before using the FILE pointer.
 *   2. Check fwrite() / fread() return values — they return the number
 *      of items transferred, which may be less than requested on error.
 *   3. Distinguish EOF from error in read loops (use feof/ferror).
 *   4. Always fclose() in all exit paths (including error paths).
 *   5. Use perror() / strerror() for actionable diagnostics.
 */

static void section(const char *title) {
    printf("\n=== %s ===\n", title);
}

/* ------------------------------------------------------------------ */
/* Helper: open file with detailed error reporting                      */
/* ------------------------------------------------------------------ */
static FILE *safeOpen(const char *path, const char *mode) {
    errno = 0;
    FILE *fp = fopen(path, mode);
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] Cannot open '%s' (mode='%s'): %s (errno=%d)\n",
                path, mode, strerror(errno), errno);
    }
    return fp;
}

/* ------------------------------------------------------------------ */
/* Helper: safe fwrite with verification                                */
/* ------------------------------------------------------------------ */
static int safeWrite(FILE *fp, const void *data, size_t size, size_t count) {
    size_t written = fwrite(data, size, count, fp);
    if (written != count) {
        fprintf(stderr, "[ERROR] fwrite: requested %zu, wrote %zu — %s\n",
                count, written, strerror(errno));
        return 0;   /* failure */
    }
    return 1;   /* success */
}

/* ------------------------------------------------------------------ */
/* Helper: safe fread with verification                                 */
/* ------------------------------------------------------------------ */
static size_t safeRead(FILE *fp, void *buf, size_t size, size_t count) {
    size_t n = fread(buf, size, count, fp);
    if (n < count) {
        if (feof(fp)) {
            /* Normal: reached end of file */
        } else if (ferror(fp)) {
            fprintf(stderr, "[ERROR] fread: requested %zu, got %zu — %s\n",
                    count, n, strerror(errno));
        }
    }
    return n;
}

int main() {
    printf("=== File Error Handling ===\n");

    /* ------------------------------------------------------------------ */
    section("1. Opening a Non-Existent File (ENOENT)");
    /* ------------------------------------------------------------------ */
    errno = 0;
    FILE *fp = fopen("no_such_file.txt", "r");
    if (fp == NULL) {
        printf("fopen failed as expected.\n");
        printf("  errno        = %d\n",  errno);
        printf("  strerror()   = %s\n",  strerror(errno));
        /* perror prints to stderr automatically */
        perror("  perror output");
    }

    /* ------------------------------------------------------------------ */
    section("2. Checking feof() vs ferror() in a Read Loop");
    /* ------------------------------------------------------------------ */
    /* Write a small file first */
    fp = safeOpen("test_eof.txt", "w");
    if (fp) {
        fprintf(fp, "Line A\nLine B\nLine C\n");
        fclose(fp);
    }

    fp = safeOpen("test_eof.txt", "r");
    if (fp) {
        char buf[64];
        int  linesRead = 0;

        while (fgets(buf, sizeof(buf), fp) != NULL) {
            buf[strcspn(buf, "\n")] = '\0';
            printf("  Read: '%s'\n", buf);
            linesRead++;
        }

        /* Determine WHY the loop ended */
        if (feof(fp)) {
            printf("Loop ended: EOF reached after %d lines.\n", linesRead);
        }
        if (ferror(fp)) {
            fprintf(stderr, "Loop ended: I/O error — %s\n", strerror(errno));
        }
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("3. clearerr() — Reset EOF and Error Indicators");
    /* ------------------------------------------------------------------ */
    fp = safeOpen("test_eof.txt", "r");
    if (fp) {
        /* Read until EOF */
        while (fgetc(fp) != EOF) { /* drain */ }

        printf("After draining: feof=%d  ferror=%d\n", feof(fp), ferror(fp));

        clearerr(fp);
        printf("After clearerr: feof=%d  ferror=%d  (both should be 0)\n",
               feof(fp), ferror(fp));

        /* Rewind and read again */
        rewind(fp);
        char line[64];
        if (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = '\0';
            printf("After rewind, first line: '%s'\n", line);
        }
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("4. safeWrite() / safeRead() — Verify Transfer Counts");
    /* ------------------------------------------------------------------ */
    int data[] = {10, 20, 30, 40, 50};
    int n = sizeof(data) / sizeof(data[0]);

    fp = safeOpen("safe_io.bin", "wb");
    if (fp) {
        if (safeWrite(fp, data, sizeof(int), n)) {
            printf("safeWrite: successfully wrote %d integers\n", n);
        }
        fclose(fp);
    }

    int readBuf[5] = {0};
    fp = safeOpen("safe_io.bin", "rb");
    if (fp) {
        size_t got = safeRead(fp, readBuf, sizeof(int), n);
        printf("safeRead:  read %zu integers: ", got);
        for (size_t i = 0; i < got; i++) printf("%d ", readBuf[i]);
        printf("\n");
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("5. Handling fseek() / ftell() Errors");
    /* ------------------------------------------------------------------ */
    fp = safeOpen("test_eof.txt", "r");
    if (fp) {
        /* Valid seek */
        if (fseek(fp, 5, SEEK_SET) == 0) {
            printf("fseek to byte 5: OK, ftell=%ld\n", ftell(fp));
        } else {
            perror("fseek failed");
        }

        /* Seek beyond file end is allowed; reading will then return EOF */
        fseek(fp, 1000, SEEK_SET);
        printf("fseek to byte 1000 (past end): ftell=%ld\n", ftell(fp));
        int ch = fgetc(fp);
        printf("fgetc at byte 1000: %s\n",
               ch == EOF ? (feof(fp) ? "EOF (as expected)" : "Error") : "Got char?");
        fclose(fp);
    }

    /* ------------------------------------------------------------------ */
    section("6. remove() and rename() Error Handling");
    /* ------------------------------------------------------------------ */
    errno = 0;
    int rc = remove("ghost_file.txt");   /* file doesn't exist */
    if (rc != 0) {
        printf("remove(ghost_file.txt) failed: %s\n", strerror(errno));
    }

    errno = 0;
    rc = rename("ghost_src.txt", "ghost_dst.txt");
    if (rc != 0) {
        printf("rename(ghost_src, ghost_dst) failed: %s\n", strerror(errno));
    }

    /* ------------------------------------------------------------------ */
    section("7. Robust File-Copy with Full Error Checking");
    /* ------------------------------------------------------------------ */
    /* Write source */
    fp = safeOpen("src_copy.txt", "w");
    if (!fp) return 1;
    fprintf(fp, "This file will be copied robustly.\n");
    fprintf(fp, "Every byte is verified.\n");
    fclose(fp);

    FILE *src = safeOpen("src_copy.txt", "r");
    FILE *dst = safeOpen("dst_copy.txt", "w");

    if (src && dst) {
        int ch;
        long bytesCopied = 0;
        int copyOk = 1;

        while ((ch = fgetc(src)) != EOF) {
            if (fputc(ch, dst) == EOF) {
                fprintf(stderr, "[ERROR] fputc failed at byte %ld: %s\n",
                        bytesCopied, strerror(errno));
                copyOk = 0;
                break;
            }
            bytesCopied++;
        }

        if (ferror(src)) {
            fprintf(stderr, "[ERROR] Read error on source: %s\n", strerror(errno));
            copyOk = 0;
        }

        fclose(src);
        if (fclose(dst) == EOF) {
            /* fclose can fail if flush fails (e.g., disk full) */
            fprintf(stderr, "[ERROR] fclose on destination failed: %s\n",
                    strerror(errno));
            copyOk = 0;
        }

        if (copyOk) {
            printf("Robust copy: %ld bytes copied OK\n", bytesCopied);
        } else {
            printf("Copy FAILED — see error messages above\n");
        }
    } else {
        if (src) fclose(src);
        if (dst) fclose(dst);
    }

    /* ------------------------------------------------------------------ */
    section("8. Summary of Error-Handling Functions");
    /* ------------------------------------------------------------------ */
    printf("  fopen() returns NULL on failure; always check before use.\n");
    printf("  errno              — set by library on failure; read right after the call.\n");
    printf("  perror(msg)        — prints 'msg: <system error>' to stderr.\n");
    printf("  strerror(errno)    — returns error string; useful in fprintf.\n");
    printf("  feof(fp)           — non-zero if EOF indicator is set.\n");
    printf("  ferror(fp)         — non-zero if error indicator is set.\n");
    printf("  clearerr(fp)       — clears both feof and ferror flags.\n");
    printf("  fwrite/fread       — return items transferred; compare to expected count.\n");
    printf("  fclose() returns EOF on flush/close failure; always check.\n");
    printf("  remove/rename      — return 0 on success, -1 on failure (check errno).\n");

    /* Cleanup */
    remove("test_eof.txt");
    remove("safe_io.bin");
    remove("src_copy.txt");
    remove("dst_copy.txt");

    return 0;
}
