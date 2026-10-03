#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

COMPAT_DIR="$WORKSPACE/musl-printf"
SRC="$COMPAT_DIR/parse_printf_format.c"
OBJ="$COMPAT_DIR/parse_printf_format.o"
LIB="$COMPAT_DIR/libmuslprintf.a"

INSTALL_LIB="$SYSROOT/usr/lib/libmuslprintf.a"

echo "========================================"
echo "Create musl printf parser"
echo "========================================"
echo
echo "Target      : $TARGET"
echo "Compiler    : $PREFIX/bin/$TARGET-gcc"
echo "Sysroot     : $SYSROOT"
echo "Work dir    : $COMPAT_DIR"
echo

# ------------------------------------------------------------
# Check compiler
# ------------------------------------------------------------

if [ ! -x "$PREFIX/bin/$TARGET-gcc" ]; then
    echo "ERROR: Target compiler not found:"
    echo "  $PREFIX/bin/$TARGET-gcc"
    exit 1
fi

mkdir -p "$COMPAT_DIR"
mkdir -p "$SYSROOT/usr/lib"

# ------------------------------------------------------------
# Create parser implementation
# ------------------------------------------------------------

cat > "$SRC" <<'EOF'
/*
 * musl-compatible implementation of the GNU parse_printf_format()
 * interface required by systemd.
 *
 * This is intentionally a small implementation covering the printf
 * argument types used by systemd.
 */

#include <stddef.h>
#include <ctype.h>
#include <printf.h>

/*
 * Store one argument type.
 *
 * GNU parse_printf_format() returns the number of arguments required
 * and fills argtypes[] with the corresponding PA_* values.
 */
static void store_type(int *argtypes, size_t n, size_t *count, int type)
{
        if (*count < n)
                argtypes[*count] = type;

        (*count)++;
}

static int is_flag_char(char c)
{
        switch (c) {
        case '#':
        case '0':
        case '-':
        case ' ':
        case '+':
        case '\'':
        case 'I':
                return 1;

        default:
                return 0;
        }
}


static int integer_type_from_length(
        int long_count,
        int short_count)
{
        if (short_count)
                return PA_INT | PA_FLAG_SHORT;

        if (long_count >= 2)
                return PA_INT | PA_FLAG_LONG_LONG;

        if (long_count == 1)
                return PA_INT | PA_FLAG_LONG;

        return PA_INT;
}

size_t parse_printf_format(
        const char *format,
        size_t n,
        int *argtypes)
{
        const char *p;
        size_t count = 0;

        if (!format)
                return 0;

        p = format;

        while (*p) {

                /*
                 * Ordinary character.
                 */
                if (*p != '%') {
                        p++;
                        continue;
                }

                p++;

                /*
                 * %% does not consume an argument.
                 */
                if (*p == '%') {
                        p++;
                        continue;
                }

                /*
                 * POSIX/GNU positional argument syntax:
                 *
                 *   %2$d
                 *
                 * Systemd's uses do not depend on positional arguments.
                 *
                 * We still skip the positional index so that the
                 * conversion itself can be classified.
                 */
                if (isdigit((unsigned char)*p)) {
                        const char *q = p;

                        while (isdigit((unsigned char)*q))
                                q++;

                        if (*q == '$')
                                p = q + 1;
                }

                /*
                 * Flags.
                 */
                while (is_flag_char(*p))
                        p++;

                /*
                 * Field width.
                 *
                 * '*' consumes an int argument.
                 */
                if (*p == '*') {
                        store_type(argtypes, n, &count, PA_INT);
                        p++;

                        /*
                         * Positional width:
                         *
                         *   %*2$d
                         *
                         * Skip the positional index.
                         */
                        if (isdigit((unsigned char)*p)) {
                                const char *q = p;

                                while (isdigit((unsigned char)*q))
                                        q++;

                                if (*q == '$')
                                        p = q + 1;
                        }
                } else {
                        while (isdigit((unsigned char)*p))
                                p++;
                }

                /*
                 * Precision.
                 */
                if (*p == '.') {
                        p++;

                        if (*p == '*') {
                                store_type(argtypes, n, &count, PA_INT);
                                p++;

                                if (isdigit((unsigned char)*p)) {
                                        const char *q = p;

                                        while (isdigit((unsigned char)*q))
                                                q++;

                                        if (*q == '$')
                                                p = q + 1;
                                }
                        } else {
                                while (isdigit((unsigned char)*p))
                                        p++;
                        }
                }

                /*
                 * Length modifier.
                 */
                int long_count = 0;
                int short_count = 0;

                if (*p == 'h') {
                        short_count = 1;
                        p++;

                        if (*p == 'h') {
                                short_count = 2;
                                p++;
                        }
                } else if (*p == 'l') {
                        long_count = 1;
                        p++;

                        if (*p == 'l') {
                                long_count = 2;
                                p++;
                        }
                } else if (*p == 'j' ||
                           *p == 'z' ||
                           *p == 't') {
                        /*
                         * These are integer-sized types.
                         *
                         * For the purposes of va_arg(), systemd's
                         * VA_FORMAT_ADVANCE() treats them according
                         * to the promoted integer category.
                         */
                        long_count = 1;
                        p++;
                } else if (*p == 'L') {
                        long_count = 3;
                        p++;
                } else if (*p == 'q') {
                        long_count = 2;
                        p++;
                }

                /*
                 * Conversion.
                 */
                switch (*p) {

                /*
                 * Signed/unsigned integer conversions.
                 */
                case 'd':
                case 'i':
                case 'o':
                case 'u':
                case 'x':
                case 'X':
                        store_type(
                                argtypes,
                                n,
                                &count,
                                integer_type_from_length(
                                        long_count,
                                        short_count));
                        break;

                /*
                 * Character.
                 */
                case 'c':
                        if (long_count)
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_WCHAR);
                        else
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_CHAR);
                        break;

                /*
                 * String.
                 */
                case 's':
                        if (long_count)
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_WSTRING);
                        else
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_STRING);
                        break;

                /*
                 * Pointer.
                 */
                case 'p':
                        store_type(
                                argtypes,
                                n,
                                &count,
                                PA_POINTER);
                        break;

                /*
                 * Floating-point.
                 *
                 * float arguments are promoted to double in varargs.
                 */
                case 'a':
                case 'A':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case 'g':
                case 'G':
                        if (long_count == 3)
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_DOUBLE | PA_FLAG_LONG_DOUBLE);
                        else
                                store_type(
                                        argtypes,
                                        n,
                                        &count,
                                        PA_DOUBLE);
                        break;

                /*
                 * %n stores through a pointer.
                 */
                case 'n': {
                        int type;

                        if (short_count >= 2)
                                type = PA_CHAR;
                        else if (short_count)
                                type = PA_INT | PA_FLAG_SHORT;
                        else if (long_count >= 2)
                                type = PA_INT | PA_FLAG_LONG_LONG;
                        else if (long_count == 1)
                                type = PA_INT | PA_FLAG_LONG;
                        else
                                type = PA_INT;

                        store_type(
                                argtypes,
                                n,
                                &count,
                                type | PA_FLAG_PTR);
                        break;
                }

                /*
                 * GNU %m prints strerror(errno) and consumes no
                 * argument.
                 */
                case 'm':
                        break;

                /*
                 * Unknown conversion.
                 *
                 * Do not consume an argument.
                 */
                default:
                        break;
                }

                if (*p)
                        p++;
        }

        /*
         * GNU parse_printf_format() returns the total number of
         * arguments required, even when n is smaller than count.
         */
        return count;
}
EOF

echo "Source created:"
ls -l "$SRC"

# ------------------------------------------------------------
# Compile
# ------------------------------------------------------------

echo
echo "Compiling parser..."

"$PREFIX/bin/$TARGET-gcc" \
    --sysroot="$SYSROOT" \
    -O2 \
    -fPIC \
    -Wall \
    -Wextra \
    -Werror \
    -c "$SRC" \
    -o "$OBJ"

echo "Object created:"
ls -l "$OBJ"

# ------------------------------------------------------------
# Create static library
# ------------------------------------------------------------

echo
echo "Creating libmuslprintf.a..."

"$PREFIX/bin/$TARGET-ar" \
    rcs \
    "$LIB" \
    "$OBJ"

"$PREFIX/bin/$TARGET-ranlib" "$LIB"

echo "Library created:"
ls -l "$LIB"

# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

echo
echo "Installing library into target sysroot..."

cp -av "$LIB" "$INSTALL_LIB"

echo
echo "Installed:"
ls -l "$INSTALL_LIB"

# ------------------------------------------------------------
# Verify symbols
# ------------------------------------------------------------

echo
echo "Checking library symbol..."

"$PREFIX/bin/$TARGET-nm" "$INSTALL_LIB" |
    grep "parse_printf_format"

echo
echo "========================================"
echo "SUCCESS"
echo "========================================"
echo
echo "Musl printf parser library created."
echo
echo "Source:"
echo "  $SRC"
echo
echo "Library:"
echo "  $INSTALL_LIB"
echo
echo "Next step:"
echo "  Link systemd against -lmuslprintf"
echo