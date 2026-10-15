/* Portable C11 static gallery generator. No runtime dependencies. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMIT 256
#define LINE 16384
#define FIELDS 14
typedef struct { char *field[FIELDS]; } Photo;
static Photo photos[LIMIT];
static size_t count;
static const char *region_label(const char *region) {
    if (!strcmp(region, "kenya")) return "Kenya";
    if (!strcmp(region, "alberta")) return "Alberta";
    if (!strcmp(region, "iceland")) return "Iceland";
    if (!strcmp(region, "montenegro")) return "Bar, Montenegro";
    if (!strcmp(region, "corfu")) return "Corfu, Greece";
    if (!strcmp(region, "montserrat")) return "Montserrat, Spain";
    return NULL;
}
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
        if (strcmp(category, "wildlife") && strcmp(category, "landscapes") && strcmp(category, "water") && strcmp(category, "architecture")) goto invalid;
        const char *region = photos[count].field[6];
        if (!region_label(region)) goto invalid;
        if (strncmp(url, "https://", 8) && strncmp(url, "assets/photos/", 14)) goto invalid;
        char *end_width, *end_height;
        long width = strtol(photos[count].field[10], &end_width, 10);
        long height = strtol(photos[count].field[11], &end_height, 10);
        if (*end_width || *end_height || width <= height || height < 1 || width > 20000) goto invalid;
        const char *precision = photos[count].field[13];
        if (strcmp(precision, "confirmed") && strcmp(precision, "identified") && strcmp(precision, "regional")) goto invalid;
        count++;
    }
    if (ferror(in) || !count) goto invalid;
    fclose(in); return 1;
invalid:
    fprintf(stderr, "Invalid photo manifest near entry %zu (14 nonempty tab-separated fields, valid category/location, and horizontal dimensions required).\n", count + 1);
    fclose(in); return 0;
}
static void image(Photo *p, int eager) {
    fputs("<img src=\"", stdout); escape(p->field[3]);
    size_t length = strlen(p->field[3]);
    if (!strncmp(p->field[3], "assets/photos/", 14) && length > 5 && !strcmp(p->field[3] + length - 5, ".webp")) {
        char small[LINE];
        snprintf(small, sizeof small, "%.*s-small.webp", (int)(length - 5), p->field[3]);
        fputs("\" srcset=\"", stdout); escape(small); fputs(" 800w, ", stdout);
        escape(p->field[3]); fputs(" 1800w\" sizes=\"auto, (max-width: 700px) 92vw, 92vw", stdout);
    }
    fputs("\" alt=\"", stdout); escape(p->field[4]);
    printf("\" width=\"%s\" height=\"%s\" loading=\"%s\" decoding=\"async\">", p->field[10], p->field[11], eager ? "eager" : "lazy");
}
static void gallery(void) {
    const char *names[] = {"Shutter", "Aperture", "ISO"};
    for (size_t i = 0; i < count; i++) {
        Photo *p = &photos[i];
        fputs("<article class=\"encounter\" data-category=\"", stdout); escape(p->field[2]);
        printf("\"><button class=\"image-open\" popovertarget=\"view-%zu\" aria-label=\"Enlarge ", i + 1);
        escape(p->field[1]); fputs(" photograph\">", stdout); image(p, i == 0);
        fputs("</button><div class=\"frame-label\"><span>", stdout);
        printf("%02zu</span><h2>", i + 1); escape(p->field[0]);
        fputs("</h2><span>", stdout); escape(p->field[1]);
        fputs("</span></div><div class=\"photo-notes\"><span class=\"place\">", stdout);
        escape(p->field[12]);
        fputs("</span><details><summary>Frame notes +</summary><dl>", stdout);
        fputs("<dt>Location</dt><dd>", stdout); escape(p->field[12]); fputs("</dd>", stdout);
        for (size_t f = 7; f < 10; f++) {
            printf("<dt>%s</dt><dd>", names[f - 7]); escape(p->field[f]); fputs("</dd>", stdout);
        }
        fputs("</dl>", stdout);
        if (!strcmp(p->field[13], "regional")) fputs("<p>Exact shooting spot not recorded.</p>", stdout);
        if (!strcmp(p->field[13], "identified")) fputs("<p>Location identified from visible landmarks; not confirmed by GPS.</p>", stdout);
        fputs("<p>Photographed &amp; edited by Neil.</p></details></div>", stdout);
        printf("<div class=\"photo-viewer\" id=\"view-%zu\" popover><div class=\"viewer-bar\"><div><span class=\"viewer-title\">", i + 1);
        escape(p->field[0]);
        fputs("</span><span class=\"viewer-location\">", stdout); escape(p->field[12]);
        if (!strcmp(p->field[13], "identified")) fputs(" · visual match", stdout);
        if (!strcmp(p->field[13], "regional")) fputs(" · exact spot unconfirmed", stdout);
        printf("</span></div><button popovertarget=\"view-%zu\" popovertargetaction=\"hide\">Close ×</button></div>", i + 1);
        image(p, 0);
        printf("<div class=\"viewer-nav\"><span>FRAME %02zu / %02zu</span><span>Photographed &amp; edited by Neil</span><span>Esc to return</span></div></div></article>\n", i + 1, count);
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
