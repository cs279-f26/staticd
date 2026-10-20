#include "http.h"

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

static const struct { const char *ext, *type; } types[] = {
    {".html", "text/html"}, {".css", "text/css"}, {".js", "application/javascript"},
    {".txt", "text/plain"}, {".png", "image/png"}, {".jpg", "image/jpeg"},
    {".json", "application/json"},
};

static const char *content_type(const char *path)
{
    const char *dot = strrchr(path, '.');

    for (size_t i = 0; dot != NULL && i < sizeof types / sizeof types[0]; i++)
        if (strcmp(dot, types[i].ext) == 0)
            return types[i].type;
    return "application/octet-stream";
}

static void status(FILE *out, int code, const char *reason)
{
    fprintf(out, "HTTP/1.0 %d %s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s\n",
            code, reason, strlen(reason) + 1, reason);
}

static int hexval(int c)
{
    if (isdigit(c))
        return c - '0';
    c = tolower(c);
    return c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

/* Decodes %XX escapes in place. Returns -1 on a malformed escape or %00. */
static int url_decode(char *s)
{
    char *r = s, *w = s;

    while (*r != '\0') {
        if (*r == '%') {
            int hi = hexval((unsigned char)r[1]), lo = hi < 0 ? -1 : hexval((unsigned char)r[2]);
            if (hi < 0 || lo < 0 || (hi == 0 && lo == 0))
                return -1;
            *w++ = (char)(hi * 16 + lo);
            r += 3;
        } else
            *w++ = *r++;
    }
    *w = '\0';
    return 0;
}

/* Removes every "../" and "/.." segment, so the path stays under the root. */
static void strip_dotdot(char *s)
{
    char *p;

    while ((p = strstr(s, "../")) != NULL)
        memmove(p, p + 3, strlen(p + 3) + 1);
    while ((p = strstr(s, "/..")) != NULL && (p[3] == '\0' || p[3] == '/'))
        memmove(p, p + 3, strlen(p + 3) + 1);
}

/* Maps the request target to a file path under root. Returns -1 if invalid. */
static int resolve(const char *root, char *target, char *path, size_t size)
{
    struct stat st;
    int n;

    target[strcspn(target, "?#")] = '\0';
    if (target[0] != '/')
        return -1;
    strip_dotdot(target);
    if (url_decode(target) != 0)
        return -1;
    n = snprintf(path, size, "%s%s", root, target);
    if (n < 0 || (size_t)n >= size)
        return -1;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        n = snprintf(path + n, size - (size_t)n, "%sindex.html",
                     path[n - 1] == '/' ? "" : "/");
        if (n < 0)
            return -1;
    }
    return 0;
}

void serve(const char *root, FILE *in, FILE *out)
{
    char line[MAX_REQUEST_LINE], method[8], target[MAX_REQUEST_LINE], version[16];
    char path[MAX_PATH], buf[8192];
    struct stat st;
    FILE *f;
    size_t n;
    int head;

    if (fgets(line, sizeof line, in) == NULL
        || sscanf(line, "%7s %2047s %15s", method, target, version) != 3
        || strncmp(version, "HTTP/1.", 7) != 0) {
        status(out, 400, "Bad Request");
        return;
    }
    while (fgets(buf, sizeof buf, in) != NULL && strcmp(buf, "\r\n") != 0 && strcmp(buf, "\n") != 0)
        ;   /* headers are ignored */
    head = strcmp(method, "HEAD") == 0;
    if (!head && strcmp(method, "GET") != 0) {
        status(out, 405, "Method Not Allowed");
        return;
    }
    if (resolve(root, target, path, sizeof path) != 0) {
        status(out, 400, "Bad Request");
        return;
    }
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || (f = fopen(path, "rb")) == NULL) {
        status(out, 404, "Not Found");
        return;
    }
    if (st.st_size > MAX_FILE) {
        fclose(f);
        status(out, 413, "Payload Too Large");
        return;
    }
    fprintf(out, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %lld\r\n\r\n",
            content_type(path), (long long)st.st_size);
    while (!head && (n = fread(buf, 1, sizeof buf, f)) > 0)
        fwrite(buf, 1, n, out);
    fclose(f);
}
