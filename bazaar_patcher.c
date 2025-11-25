#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

static const char *THEME_STRINGS[] = {
    "pride-rainbow-flag",
    "pride-rainbow-theme",
    "Pride Colors",
    "lesbian-pride-flag",
    "lesbian-pride-theme",
    "Lesbian Pride Colors",
    "transgender-flag",
    "transgender-theme",
    "Transgender Pride Colors",
    "nonbinary-flag",
    "nonbinary-theme",
    "Nonbinary Pride Colors",
    "bisexual-flag",
    "bisexual-theme",
    "Bisexual Pride Colors",
    "asexual-flag",
    "asexual-theme",
    "Asexual Pride Colors",
    "pansexual-flag",
    "pansexual-theme",
    "Pansexual Pride Colors",
    "aromantic-flag",
    "aromantic-theme",
    "Aromantic Pride Colors",
    "genderfluid-flag",
    "genderfluid-theme",
    "Genderfluid Pride Colors",
    "polysexual-flag",
    "polysexual-theme",
    "Polysexual Pride Colors",
    "omnisexual-flag",
    "omnisexual-theme",
    "Omnisexual Pride Colors",
    NULL
};

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct {
    size_t found;
    size_t patched;
} PatchResult;

static const char *find_binary_path(void) {
    static const char *candidates[] = {
        "/var/lib/flatpak/app/io.github.kolunmi.Bazaar/current/active/files/bin/bazaar",
        "/app/bin/bazaar",
        "/usr/local/bin/bazaar",
        "/usr/bin/bazaar",
        NULL
    };

    for (size_t i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], F_OK) == 0) {
            return candidates[i];
        }
    }

    return NULL;
}

static bool verify_not_running(void) {
    FILE *fp = popen("pgrep -x bazaar 2>/dev/null", "r");
    if (fp == NULL) {
        return true; // Assume not running if can't check
    }

    char buf[16];
    bool running = (fgets(buf, sizeof(buf), fp) != NULL);
    pclose(fp);

    return !running;
}

static size_t search_and_patch(unsigned char *data, size_t data_len, const char *needle) {
    size_t count = 0;
    size_t needle_len = strlen(needle);

    if (needle_len == 0 || needle_len > data_len) {
        return 0;
    }

    // Search for all occurrences and null them out
    for (size_t i = 0; i <= data_len - needle_len; i++) {
        if (memcmp(&data[i], needle, needle_len) == 0) {
            memset(&data[i], 0, needle_len);
            count++;
        }
    }

    return count;
}

static int patch_binary(const char *path, PatchResult *result) {
    int fd = -1;
    unsigned char *mapped = NULL;
    struct stat st;
    int ret = -1;

    // Initialize result
    result->found = 0;
    result->patched = 0;

    // Open file
    fd = open(path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open %s: %s\n", path, strerror(errno));
        goto cleanup;
    }

    // Get file size
    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "Error: Cannot stat file: %s\n", strerror(errno));
        goto cleanup;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "Error: File is empty\n");
        goto cleanup;
    }

    // Memory map the file
    mapped = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        fprintf(stderr, "Error: Cannot map file: %s\n", strerror(errno));
        mapped = NULL;
        goto cleanup;
    }

    // Patch all theme strings
    for (size_t i = 0; THEME_STRINGS[i] != NULL; i++) {
        size_t count = search_and_patch(mapped, st.st_size, THEME_STRINGS[i]);
        if (count > 0) {
            result->found++;
            result->patched += count;
            printf("  Patched %zu occurrence(s) of: %s\n", count, THEME_STRINGS[i]);
        }
    }

    // Ensure changes are written to disk
    if (msync(mapped, st.st_size, MS_SYNC) < 0) {
        fprintf(stderr, "Error: Cannot sync changes: %s\n", strerror(errno));
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (mapped != NULL && mapped != MAP_FAILED) {
        munmap(mapped, st.st_size);
    }
    if (fd >= 0) {
        close(fd);
    }

    return ret;
}

int main(int argc, char *argv[]) {
    const char *target_path = NULL;
    PatchResult result;

    printf("Bazaar Theme Patcher v1.0\n\n");

    // Verify app is not running
    if (!verify_not_running()) {
        fprintf(stderr, "Error: Bazaar is currently running. Please close it first.\n");
        return 1;
    }

    // Determine target path
    if (argc > 1) {
        target_path = argv[1];
    } else {
        target_path = find_binary_path();
        if (target_path == NULL) {
            fprintf(stderr, "Error: Could not auto-detect Bazaar installation.\n\n");
            fprintf(stderr, "Usage: %s <path-to-bazaar-binary>\n\n", argv[0]);
            fprintf(stderr, "Example:\n");
            fprintf(stderr, "  sudo %s /usr/local/bin/bazaar\n", argv[0]);
            return 1;
        }
        printf("Found Bazaar binary: %s\n", target_path);
    }

    // Verify file exists
    if (access(target_path, F_OK) != 0) {
        fprintf(stderr, "Error: Binary not found at: %s\n", target_path);
        return 1;
    }

    printf("\nTarget: %s\n", target_path);

    // Patch the binary
    printf("\nPatching binary...\n");
    if (patch_binary(target_path, &result) != 0) {
        fprintf(stderr, "\nFailed to patch binary\n");
        return 1;
    }

    printf("\nSuccess!\n");
    printf("  %zu unique strings found\n", result.found);
    printf("  %zu total occurrences patched\n", result.patched);

    if (result.found == 0) {
        printf("\nWarning: No theme strings found. Binary may already be patched or structure has changed.\n");
    }

    printf("\nDone. Restart Bazaar to see changes.\n");
    return 0;
}
