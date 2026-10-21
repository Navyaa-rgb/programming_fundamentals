#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * BINARY FILE OPERATIONS WITH STRUCTURES
 * ========================================
 * Storing structs in binary files is the foundation of simple flat-file
 * databases in C.  Because fwrite/fread copy raw bytes, every field
 * (int, float, char[]) is stored exactly as it is in memory.
 *
 * ADVANTAGES over text files:
 *   - Fixed record size  → O(1) random access by index
 *   - No parsing needed  → fast reads
 *   - Exact numeric representation (no floating-point rounding via text)
 *
 * TRADE-OFFS:
 *   - Not human-readable
 *   - Platform-dependent endianness / struct padding
 *   - Use for programs that write and read on the same architecture
 *
 * TOPICS COVERED:
 *   1. Write an array of structs
 *   2. Read all records
 *   3. Search for a record
 *   4. Update a record in-place
 *   5. Delete a record (rewrite technique)
 *   6. Append a new record
 *   7. Display all records as a formatted table
 */

#define MAX_NAME 30
#define MAX_DEPT 20

typedef struct {
    int   empId;
    char  name[MAX_NAME];
    char  dept[MAX_DEPT];
    float salary;
} Employee;

static const char *DB_FILE = "employees.bin";

/* ------------------------------------------------------------------ */
/* Helper: print a single employee row                                  */
/* ------------------------------------------------------------------ */
static void printEmployee(const Employee *e) {
    printf("  %-6d  %-28s  %-18s  %.2f\n",
           e->empId, e->name, e->dept, e->salary);
}

/* ------------------------------------------------------------------ */
/* Helper: print all records in the binary file                         */
/* ------------------------------------------------------------------ */
static void printAllEmployees(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) { perror("printAllEmployees"); return; }

    printf("  %-6s  %-28s  %-18s  %s\n", "ID", "Name", "Department", "Salary");
    printf("  %s\n", "------  ----------------------------  ------------------  --------");

    Employee e;
    while (fread(&e, sizeof(Employee), 1, fp) == 1) {
        printEmployee(&e);
    }
    fclose(fp);
}

/* ------------------------------------------------------------------ */
/* Helper: count records in file                                        */
/* ------------------------------------------------------------------ */
static long countRecords(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) return 0;
    fseek(fp, 0, SEEK_END);
    long bytes = ftell(fp);
    fclose(fp);
    return bytes / (long)sizeof(Employee);
}

static void section(const char *title) {
    printf("\n=== %s ===\n", title);
}

int main() {
    printf("=== Binary File Operations with Structures ===\n");

    /* ------------------------------------------------------------------ */
    section("1. Write Array of Structs (\"wb\")");
    /* ------------------------------------------------------------------ */
    Employee employees[] = {
        {1001, "Alice Johnson",   "Engineering",  75000.00f},
        {1002, "Bob Smith",       "Marketing",    62000.00f},
        {1003, "Carol Williams",  "Engineering",  82000.00f},
        {1004, "David Brown",     "HR",           55000.00f},
        {1005, "Eve Davis",       "Finance",      71000.00f}
    };
    int n = sizeof(employees) / sizeof(employees[0]);

    FILE *fp = fopen(DB_FILE, "wb");
    if (fp == NULL) { perror("fopen employees.bin"); return 1; }

    size_t written = fwrite(employees, sizeof(Employee), n, fp);
    fclose(fp);
    printf("Wrote %zu employee records (%zu bytes each) to %s\n",
           written, sizeof(Employee), DB_FILE);

    /* ------------------------------------------------------------------ */
    section("2. Read All Records (\"rb\")");
    /* ------------------------------------------------------------------ */
    printf("Total records in file: %ld\n", countRecords(DB_FILE));
    printAllEmployees(DB_FILE);

    /* ------------------------------------------------------------------ */
    section("3. Search for a Record by ID");
    /* ------------------------------------------------------------------ */
    int searchId = 1003;
    fp = fopen(DB_FILE, "rb");
    if (fp == NULL) { perror("fopen"); return 1; }

    Employee found;
    int foundFlag = 0;
    long recIndex = 0;

    while (fread(&found, sizeof(Employee), 1, fp) == 1) {
        if (found.empId == searchId) {
            foundFlag = 1;
            break;
        }
        recIndex++;
    }
    fclose(fp);

    if (foundFlag) {
        printf("Found ID %d at record index %ld:\n", searchId, recIndex);
        printEmployee(&found);
    } else {
        printf("ID %d not found.\n", searchId);
    }

    /* ------------------------------------------------------------------ */
    section("4. Update a Record In-Place (\"rb+\")");
    /* ------------------------------------------------------------------ */
    /* Give Bob Smith a raise to 68000 */
    int updateId = 1002;
    fp = fopen(DB_FILE, "rb+");
    if (fp == NULL) { perror("fopen rb+"); return 1; }

    Employee emp;
    long recPos = 0;
    int  updated = 0;

    while (fread(&emp, sizeof(Employee), 1, fp) == 1) {
        if (emp.empId == updateId) {
            printf("Before update: ID=%d  Salary=%.2f\n", emp.empId, emp.salary);
            emp.salary = 68000.00f;
            /* Seek back to the start of this record and overwrite */
            fseek(fp, recPos * (long)sizeof(Employee), SEEK_SET);
            fwrite(&emp, sizeof(Employee), 1, fp);
            printf("After  update: ID=%d  Salary=%.2f\n", emp.empId, emp.salary);
            updated = 1;
            break;
        }
        recPos++;
    }
    fclose(fp);
    if (!updated) printf("ID %d not found for update.\n", updateId);

    /* ------------------------------------------------------------------ */
    section("5. Append a New Record (\"ab\")");
    /* ------------------------------------------------------------------ */
    Employee newEmp = {1006, "Frank Miller", "Legal", 79000.00f};
    fp = fopen(DB_FILE, "ab");
    if (fp == NULL) { perror("fopen ab"); return 1; }
    fwrite(&newEmp, sizeof(Employee), 1, fp);
    fclose(fp);
    printf("Appended: ID=%d  Name=%s\n", newEmp.empId, newEmp.name);
    printf("Total records now: %ld\n", countRecords(DB_FILE));

    /* ------------------------------------------------------------------ */
    section("6. Delete a Record (Rewrite to Temp File Technique)");
    /* ------------------------------------------------------------------ */
    /*
     * C has no "delete row" primitive for binary files.
     * Standard approach:
     *   a) Copy all records EXCEPT the one to delete to a temp file.
     *   b) Remove the original.
     *   c) Rename the temp file to the original name.
     */
    int deleteId = 1004;  /* Delete David Brown */
    const char *tmpFile = "employees_tmp.bin";

    FILE *src = fopen(DB_FILE, "rb");
    FILE *dst = fopen(tmpFile, "wb");
    if (!src || !dst) { perror("fopen delete"); return 1; }

    int deletedCount = 0;
    Employee e;
    while (fread(&e, sizeof(Employee), 1, src) == 1) {
        if (e.empId != deleteId) {
            fwrite(&e, sizeof(Employee), 1, dst);
        } else {
            deletedCount++;
            printf("Deleted record: ID=%d  Name=%s\n", e.empId, e.name);
        }
    }
    fclose(src);
    fclose(dst);

    if (deletedCount == 0) {
        printf("ID %d not found; nothing deleted.\n", deleteId);
        remove(tmpFile);
    } else {
        remove(DB_FILE);
        rename(tmpFile, DB_FILE);
        printf("Records remaining: %ld\n", countRecords(DB_FILE));
    }

    /* ------------------------------------------------------------------ */
    section("7. Final State of the Employee Database");
    /* ------------------------------------------------------------------ */
    printAllEmployees(DB_FILE);

    /* ------------------------------------------------------------------ */
    section("8. Struct Padding Awareness");
    /* ------------------------------------------------------------------ */
    printf("sizeof(Employee) = %zu bytes\n", sizeof(Employee));
    printf("  int empId      : %zu bytes\n", sizeof(int));
    printf("  char name[%d]  : %d bytes\n", MAX_NAME, MAX_NAME);
    printf("  char dept[%d]  : %d bytes\n", MAX_DEPT, MAX_DEPT);
    printf("  float salary   : %zu bytes\n", sizeof(float));
    printf("  (difference is compiler padding/alignment)\n");
    printf("\nTIP: Use __attribute__((packed)) or #pragma pack(1) to remove\n");
    printf("     padding when portability across platforms is needed.\n");

    /* Cleanup */
    remove(DB_FILE);

    return 0;
}
