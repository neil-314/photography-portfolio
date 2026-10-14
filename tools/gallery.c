/* Portable C11 static gallery generator. No runtime dependencies. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMIT 256
#define LINE 16384
#define FIELDS 10
typedef struct { char *field[FIELDS]; } Photo;
static Photo photos[LIMIT];
static size_t count;
static void cleanup(void) {
    for (size_t i = 0; i < LIMIT; i++)
        for (size_t f = 0; f < FIELDS; f++) free(photos[i].field[f]);
}

static void escape(const char *s) {
    for (; *s; s++) {
        switch (*s) {
        case '&': fputs("&amp;", stdout); break;
        case '<': fputs("&lt;", stdout); break;
        case '>': fputs("&gt;", stdout); break;
        case '"': fputs("&quot;", stdout); break;
        case '\'': fputs("&#39;", stdout); break;
        default: putchar((unsigned char)*s);
        }
    }
}
static int read_photos(const char *path) {
    FILE *in = fopen(path, "rb"); char line[LINE];
    if (!in) { perror(path); return 0; }
    while (fgets(line, sizeof line, in)) {
        size_t length = strlen(line);
        if (length == sizeof line - 1 || count == LIMIT) goto invalid;
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0] || line[0] == '#') continue;
        char *cursor = line;
        for (size_t f = 0; f < FIELDS; f++) {
            char *end = strchr(cursor, '\t');
            if ((f < FIELDS - 1 && !end) || (f == FIELDS - 1 && end)) goto invalid;
            if (end) *end = 0;
            if (!*cursor) goto invalid;
            photos[count].field[f] = malloc(strlen(cursor) + 1);
            if (!photos[count].field[f]) goto invalid;
            strcpy(photos[count].field[f], cursor);
            if (end) cursor = end + 1;
        }
        const char *category = photos[count].field[2], *url = photos[count].field[3];
        if (strcmp(category, "birds") && strcmp(category, "mammals")) goto invalid;
        if (strncmp(url, "https://", 8) && strncmp(url, "assets/photos/", 14)) goto invalid;
        count++;
    }
    if (ferror(in) || !count) goto invalid;
    fclose(in); return 1;
invalid:
    fprintf(stderr, "Invalid photo manifest near entry %zu (10 nonempty tab-separated fields required).\n", count + 1);
    fclose(in); return 0;
}
static void image(Photo *p, int eager) {
    fputs("<img src=\"", stdout); escape(p->field[3]);
    fputs("\" alt=\"", stdout); escape(p->field[4]);
    printf("\" loading=\"%s\" decoding=\"async\">", eager ? "eager" : "lazy");
}
static void gallery(void) {
    const char *names[] = {"Camera", "Lens", "Shutter", "Aperture", "ISO"};
    for (size_t i = 0; i < count; i++) {
        Photo *p = &photos[i];
        fputs("<article class=\"encounter\" data-category=\"", stdout); escape(p->field[2]);
        printf("\"><button class=\"image-open\" popovertarget=\"view-%zu\" aria-label=\"Enlarge ", i + 1);
        escape(p->field[1]); fputs(" photograph\">", stdout); image(p, i == 0);
        fputs("<span class=\"badge\">SAMPLE / DRAFT</span></button><div class=\"frame-label\"><span>", stdout);
        printf("%02zu</span><h2>", i + 1); escape(p->field[0]);
        fputs("</h2><span>", stdout); escape(p->field[1]);
        fputs("</span></div><details><summary>Photo details</summary><dl>", stdout);
        for (size_t f = 5; f < FIELDS; f++) {
            printf("<dt>%s</dt><dd>", names[f - 5]); escape(p->field[f]); fputs("</dd>", stdout);
        }
        fputs("</dl><p class=\"hint\">Camera details are supplied in the manifest; automatic EXIF extraction is not enabled.</p></details>", stdout);
        printf("<div class=\"photo-viewer\" id=\"view-%zu\" popover><div class=\"viewer-bar\"><span>", i + 1);
        escape(p->field[0]);
        printf("</span><button popovertarget=\"view-%zu\" popovertargetaction=\"hide\">Close ×</button></div>", i + 1);
        image(p, 0);
        printf("<div class=\"viewer-nav\"><button popovertarget=\"view-%zu\">← Previous</button><span>%02zu / %02zu</span><button popovertarget=\"view-%zu\">Next →</button></div></div></article>\n", (i + count - 1) % count + 1, i + 1, count, (i + 1) % count + 1);
    }
}
int main(int argc, char **argv) {
    if (argc != 3) { fputs("Usage: gallery data/photos.tsv templates/page.html > index.html\n", stderr); return 1; }
    if (!read_photos(argv[1])) { cleanup(); return 1; }
    FILE *template = fopen(argv[2], "rb"); char line[LINE]; int markers = 0;
    if (!template) { perror(argv[2]); cleanup(); return 1; }
    while (fgets(line, sizeof line, template)) {
        char *marker = strstr(line, "@@GALLERY@@");
        if (marker) { *marker = 0; fputs(line, stdout); gallery(); fputs(marker + 11, stdout); markers++; }
        else fputs(line, stdout);
    }
    int failed = ferror(template) || markers != 1;
    fclose(template);
    cleanup();
    if (fflush(stdout) == EOF) failed = 1;
    if (failed) fputs("Template must contain exactly one gallery marker; check output before replacing index.html.\n", stderr);
    return failed ? 1 : 0;
}
