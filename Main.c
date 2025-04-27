#include <stdio.h>
#include <windows.h>
#include <string.h>

#define MAX_FILES 1000

typedef struct {
    char path[MAX_PATH];
    DWORD size;
    int isJunk;
} FileInfo;

FileInfo files[MAX_FILES];
int fileCount = 0;

// Function to get the free space of a drive
ULONGLONG getFreeSpace(const char *drive) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceEx(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        return freeBytesAvailable.QuadPart;
    }
    return 0;
}

// Smooth stylish loading bar with percentage
void loadingBar(const char *message, int totalSteps) {
    system("color 0A");
    char lightBlock = 176;
    char darkBlock = 219;

    printf("\n\n\n\n\t\t\t\t%s\n\n", message);
    printf("\t\t\t\t");

    // Print an empty progress bar with 30 blocks
    for (int i = 0; i < 30; i++) printf("%c", lightBlock);
    printf("\r\t\t\t\t");

    // Update progress bar and percentage
    for (int i = 0; i < totalSteps; i++) {
        // Print the filled blocks
        printf("%c", darkBlock);
        fflush(stdout);
        Sleep(50);

        // Print the percentage next to the bar
        if (i == totalSteps - 1) {
            printf(" %d%%", (i + 1) * 100 / totalSteps);
        }
    }
    printf("\n\n");
}

// Smooth exit animation
void exitAnimation() {
    system("color 0C");
    printf("\n\n\t\tExiting");
    for (int i = 0; i < 5; i++) {
        printf(".");
        fflush(stdout);
        Sleep(400);
    }
    printf("\n\n\t\tThank you for using Disk Cleaner!\n");
    Beep(800, 200); // Flash sound
}

// Check if file is junk type
int isJunkFile(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        if (_stricmp(ext, ".tmp") == 0 || _stricmp(ext, ".log") == 0 || _stricmp(ext, ".bak") == 0 ||
            _stricmp(ext, ".old") == 0 || _stricmp(ext, ".gid") == 0 || _stricmp(ext, ".chk") == 0) {
            return 1;
        }
    }
    return 0;
}

// Scan folders recursively
void scanFolder(const char *folderPath) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char searchPath[MAX_PATH];

    snprintf(searchPath, sizeof(searchPath), "%s\\*", folderPath);
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Unable to open directory: %s\n", folderPath);
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
            char fullPath[MAX_PATH];
            snprintf(fullPath, sizeof(fullPath), "%s\\%s", folderPath, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                scanFolder(fullPath);
            } else {
                if (fileCount < MAX_FILES) {
                    strncpy(files[fileCount].path, fullPath, MAX_PATH);
                    files[fileCount].size = findFileData.nFileSizeLow;
                    files[fileCount].isJunk = isJunkFile(findFileData.cFileName);
                    fileCount++;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

// List found files
void listFiles() {
    printf("\n%-5s %-50s %-10s %s\n", "No.", "File Path", "Size(Bytes)", "Type");
    printf("-----------------------------------------------------------------------------\n");
    for (int i = 0; i < fileCount; i++) {
        printf("%-5d %-50s %-10lu %s\n",
            i + 1,
            files[i].path,
            files[i].size,
            files[i].isJunk ? "Junk" : "Normal");
    }
    printf("\nTotal Files Found: %d\n", fileCount);
}

// Delete files function with error log and user confirmation
void deleteFiles(int deleteAllJunk) {
    int deletedFiles = 0;
    unsigned long long totalFreed = 0;
    FILE *errorLog = fopen("error_log.txt", "a"); // Open error log file

    if (deleteAllJunk) {
        for (int i = 0; i < fileCount; i++) {
            if (files[i].isJunk) {
                if (DeleteFile(files[i].path)) {
                    deletedFiles++;
                    totalFreed += files[i].size;
                } else {
                    if (errorLog) fprintf(errorLog, "Failed to delete: %s\n", files[i].path);
                }
            }
        }
    } else {
        int choice;
        char confirm;
        printf("\nEnter file numbers to delete (-1 to stop):\n");
        while (1) {
            printf("File No.: ");
            scanf("%d", &choice);

            if (choice == -1) break;

            if (choice >= 1 && choice <= fileCount) {
                printf("Are you sure you want to delete this file? (Y/N): ");
                scanf(" %c", &confirm);
                if (confirm == 'Y' || confirm == 'y') {
                    if (DeleteFile(files[choice - 1].path)) {
                        printf("Deleted: %s\n", files[choice - 1].path);
                        deletedFiles++;
                        totalFreed += files[choice - 1].size;
                    } else {
                        if (errorLog) fprintf(errorLog, "Failed to delete: %s\n", files[choice - 1].path);
                    }
                }
            } else {
                printf("Invalid file number.\n");
            }
        }
    }

    if (errorLog) fclose(errorLog); // Close error log file
    printf("\nDeleted %d file(s), Freed %llu bytes.\n", deletedFiles, totalFreed);
}

// Smooth Fade Welcome
void welcomeScreen() {
    system("color 0B");
    const char *text = "PureBYTE Cleaner - Your SystemOs Best Friend";
    printf("\n\n\n\n\t\t");
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        Sleep(40); // smooth fade effect
    }
    printf("\n\n");
}

int main() {
    char folderPath[MAX_PATH];
    int choice;

    loadingBar("Launching Disk Cleaner...", 30); // Using your original loading bar
    Sleep(500);
    system("cls");

    welcomeScreen();

    printf("\nEnter folder path to scan: ");
    fgets(folderPath, sizeof(folderPath), stdin);
    folderPath[strcspn(folderPath, "\n")] = 0;

    printf("\nScanning folder...\n");
    Sleep(500);

    // Show disk space before cleaning
    ULONGLONG beforeSpace = getFreeSpace("C:\\");
    printf("Disk space before cleaning: %llu bytes\n", beforeSpace);

    scanFolder(folderPath);
    system("cls");

    if (fileCount == 0) {
        printf("\nNo files found.\n");
        exitAnimation();
        return 0;
    } else {
        listFiles();
    }

    printf("\nWhat would you like to do?\n");
    printf("1. Delete all junk files automatically\n");
    printf("2. Manually select files to delete\n");
    printf("3. Exit\n");

    printf("Enter your choice: ");
    scanf("%d", &choice);
    system("cls");

    switch (choice) {
        case 1:
            printf("\nDeleting all junk files...\n");
            loadingBar("Cleaning junk files", 30); // Using your original loading bar
            deleteFiles(1);
            break;
        case 2:
            listFiles();
            loadingBar("Preparing manual delete mode", 30); // Using your original loading bar
            deleteFiles(0);
            break;
        case 3:
            printf("Exiting without deleting files.\n");
            exitAnimation();
            return 0;
        default:
            printf("Invalid choice.\n");
            break;
    }

    // Show disk space after cleaning
    ULONGLONG afterSpace = getFreeSpace("C:\\");
    printf("Disk space after cleaning: %llu bytes\n", afterSpace);

    exitAnimation();
    return 0;
}
