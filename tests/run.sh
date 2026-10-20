#!/bin/sh
# Usage: tests/run.sh ./staticd   (from the repository root, so www/ is found)
set -u
bin=${1:-./staticd}
fail=0
check() {  # check <name> <request line> <expected substring>
    out=$(printf '%s\r\n\r\n' "$2" | "$bin" www)
    case $out in
        *"$3"*) echo "PASS $1" ;;
        *) echo "FAIL $1: expected '$3' in: $out"; fail=1 ;;
    esac
}
check index     'GET / HTTP/1.0'                    'It works'
check dir       'GET /docs/ HTTP/1.0'               'Nothing here yet'
check dir-slash 'GET /docs HTTP/1.0'                'Nothing here yet'
check escaped   'GET /docs/read%20me.txt HTTP/1.0'  'plain text file'
check type      'GET /docs/read%20me.txt HTTP/1.0'  'Content-Type: text/plain'
check query     'GET /index.html?x=1 HTTP/1.0'      'It works'
check head      'HEAD /index.html HTTP/1.0'         'Content-Length:'
check missing   'GET /nope.html HTTP/1.0'           '404 Not Found'
check method    'POST / HTTP/1.0'                   '405'
check dotdot    'GET /../../etc/passwd HTTP/1.0'    '400 Bad Request'
check encoded   'GET /%2e%2e/%2e%2e/etc/passwd HTTP/1.0' '400 Bad Request'
check bad       'GARBAGE'                           '400'
exit $fail
