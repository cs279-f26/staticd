# staticd

A minimal static-file HTTP/1.0 server for appliances and test rigs. It is
inetd-style: one request on standard input, one response on standard output,
so it runs under `inetd`, `xinetd`, `systemd` socket activation, or
`socat TCP-LISTEN:8080,fork,reuseaddr EXEC:"./staticd www"`.

```
$ printf 'GET /index.html HTTP/1.0\r\n\r\n' | ./staticd www
HTTP/1.0 200 OK
Content-Type: text/html
Content-Length: 120
...
```

## Behavior

- `GET` and `HEAD` only; anything else is `405`.
- Files are served from the document root given as the first argument
  (default `www`). A path that names a directory serves its `index.html`.
- Percent-encoded paths (`%20`, and so on) are decoded; a query string is
  ignored.
- Content types come from the file extension (`.html`, `.css`, `.js`,
  `.txt`, `.png`, `.jpg`, `.json`); anything else is
  `application/octet-stream`.
- Files larger than 16 MiB are refused with `413`.

## Building

`make` builds `./staticd`; `make test` runs the tests against `www/`.
