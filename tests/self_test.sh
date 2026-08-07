#!/bin/sh
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
cd "$SCRIPT_DIR/.." || { echo "FAIL: could not locate the repo root above $SCRIPT_DIR"; exit 1; }
if [ ! -f Makefile ]; then
    echo "FAIL: no Makefile found in $(pwd) - this script expects to live in <repo root>/tests/"
    exit 1
fi
echo "(running from $(pwd))"

PASS=0
FAIL=0

check() {
    desc="$1"
    actual="$2"
    expected="$3"
    if [ "$actual" = "$expected" ]; then
        echo "PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $desc (expected '$expected', got '$actual')"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== Building CoreX ==="
make clean
make
if [ ! -f build/CoreX ]; then
    echo "FAIL: build did not produce build/CoreX - stopping, nothing else can be tested"
    exit 1
fi
echo "PASS: build produced build/CoreX"
PASS=$((PASS + 1))

COREX=./build/CoreX
WORK=/tmp/corex_selftest
mkdir -p "$WORK"

echo ""
echo "=== Frontend: lexer/parser/resolver ==="

UNKNOWN_COUNT=$($COREX tokens tests/lexer_full_feature.cx 2>/dev/null | grep -c "UNKNOWN")
check "no UNKNOWN tokens in the full feature file" "$UNKNOWN_COUNT" "0"

$COREX parse tests/lexer_full_feature.cx >/dev/null 2>&1
check "full feature file parses without error" "$?" "0"

CHECK_OUT=$($COREX check tests/lexer_full_feature.cx 2>&1)
check "full feature file passes semantic check" "$CHECK_OUT" "tests/lexer_full_feature.cx: OK"

cat > "$WORK/bad_immutable.cx" << 'EOF'
func main() -> int {
    let x: int = 10
    x = 20
    return x
}
EOF
$COREX check "$WORK/bad_immutable.cx" >/dev/null 2>&1
check "assigning to an immutable variable is rejected" "$?" "1"

cat > "$WORK/bad_undeclared.cx" << 'EOF'
func main() -> int {
    return y
}
EOF
$COREX check "$WORK/bad_undeclared.cx" >/dev/null 2>&1
check "using an undeclared identifier is rejected" "$?" "1"

echo ""
echo "=== Codegen: self-contained programs (no external calls) ==="

cat > "$WORK/arithmetic.cx" << 'EOF'
func main() -> int {
    return 1 + 2 * 3 - 4 / 2
}
EOF
$COREX run "$WORK/arithmetic.cx" >/dev/null 2>&1
check "operator precedence: 1+2*3-4/2 == 5" "$?" "5"

cat > "$WORK/fib.cx" << 'EOF'
func fib(n: int) -> int {
    if n < 2 {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}
func main() -> int {
    return fib(10)
}
EOF
$COREX run "$WORK/fib.cx" >/dev/null 2>&1
check "recursion: fib(10) == 55" "$?" "55"

cat > "$WORK/loop.cx" << 'EOF'
func main() -> int {
    mut sum: int = 0
    for mut i: int = 1; i <= 10; i = i + 1 {
        sum = sum + i
    }
    return sum
}
EOF
$COREX run "$WORK/loop.cx" >/dev/null 2>&1
check "for loop: sum of 1..10 == 55" "$?" "55"

cat > "$WORK/logic.cx" << 'EOF'
func main() -> int {
    let a: boolean = true
    let b: boolean = false
    if (a && !b) || (1 > 2) {
        return 1
    }
    return 0
}
EOF
$COREX run "$WORK/logic.cx" >/dev/null 2>&1
check "short-circuit boolean logic (&&, ||, !)" "$?" "1"

cat > "$WORK/compound.cx" << 'EOF'
func main() -> int {
    mut x: int = 10
    x += 5
    x -= 2
    x *= 2
    x /= 3
    return x
}
EOF
$COREX run "$WORK/compound.cx" >/dev/null 2>&1
check "compound assignment: ((10+5-2)*2)/3 == 8" "$?" "8"

cat > "$WORK/baddefer.cx" << 'EOF'
extern "C" {
    func puts(s: *const char) -> int
}
func main() -> int {
    defer puts("cleanup")
    return 0
}
EOF
$COREX build "$WORK/baddefer.cx" -o "$WORK/baddefer" >/dev/null 2>&1
check "'defer' is rejected by codegen with a clear error, not a crash" "$?" "1"

echo ""
echo "=== Codegen: extern \"C\" call into the system library ==="
echo "    (this is the one platform-sensitive check - see note printed below)"

cat > "$WORK/hello.cx" << 'EOF'
extern "C" {
    func puts(s: *const char) -> int
}
func main() -> int {
    puts("Hello from CoreX!")
    return 0
}
EOF
HELLO_OUT=$($COREX run "$WORK/hello.cx" 2>&1)
check "extern puts() call prints the expected line" "$HELLO_OUT" "Hello from CoreX!"

echo ""
echo "=== RESULTS: $PASS passed, $FAIL failed ==="
if [ "$FAIL" -eq 0 ]; then
    echo "ALL CHECKS PASSED"
    exit 0
else
    echo "SOME CHECKS FAILED - see above"
    exit 1
fi