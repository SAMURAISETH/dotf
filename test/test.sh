
#!/usr/bin/env bash
set -uo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
APP="$ROOT_DIR/build/dotfiles-managers"
TEST_ROOT="$(mktemp -d)"
OUTPUT="$TEST_ROOT/output.txt"

export HOME="$TEST_ROOT/home"
export DOTFILES_PATH="$TEST_ROOT/dotfiles"

export GIT_AUTHOR_NAME="Dotfiles Test"
export GIT_AUTHOR_EMAIL="test@example.invalid"
export GIT_COMMITTER_NAME="Dotfiles Test"
export GIT_COMMITTER_EMAIL="test@example.invalid"

PASS_COUNT=0
FAIL_COUNT=0

cleanup() {
    rm -rf -- "$TEST_ROOT"
}
trap cleanup EXIT

pass() {
    PASS_COUNT=$((PASS_COUNT + 1))
    printf 'PASS: %s\n' "$1"
}

fail() {
    FAIL_COUNT=$((FAIL_COUNT + 1))
    printf 'FAIL: %s\n' "$1" >&2
}

expect_success() {
    local name="$1"
    shift

    if "$@" >"$OUTPUT" 2>&1; then
        pass "$name"
    else
        fail "$name"
        sed -n '1,12p' "$OUTPUT" >&2
    fi
}

expect_failure() {
    local name="$1"
    shift

    if "$@" >"$OUTPUT" 2>&1; then
        fail "$name (unexpected success)"
    else
        pass "$name"
    fi
}

printf '=== Dotfiles Manager Tests ===\n\n'

# 0. Executable and test environment
if [[ -x "$APP" ]]; then
    pass "executable exists"
else
    fail "executable exists"
    printf 'Build the project first with: cmake --build build\n'
    exit 1
fi

mkdir -p "$HOME"

# 1. Basic commands
expect_success "help command" "$APP" help
if grep -q 'Usage:' "$OUTPUT"; then
    pass "help displays usage"
else
    fail "help displays usage"
fi

expect_success "path command" "$APP" path
if grep -Fq "$DOTFILES_PATH" "$OUTPUT"; then
    pass "path displays configured directory"
else
    fail "path displays configured directory"
fi

# 2. Initialization
expect_success "initialize repository" "$APP" init

if [[ -f "$DOTFILES_PATH/.links" ]]; then
    pass "init creates .links"
else
    fail "init creates .links"
fi

if [[ -d "$DOTFILES_PATH/.git" ]]; then
    pass "init creates Git repository"
else
    fail "init creates Git repository"
fi

if git -C "$DOTFILES_PATH" rev-parse --verify HEAD >/dev/null 2>&1; then
    pass "init creates initial commit"
else
    fail "init creates initial commit"
fi

# 3. Link registration
printf 'sample configuration\n' > "$DOTFILES_PATH/sample.conf"

expect_success "register link" \
    "$APP" link sample.conf "$HOME/sample.conf"

expect_failure "reject duplicate target" \
    "$APP" link sample.conf "$HOME/sample.conf"

expect_failure "reject missing source" \
    "$APP" link missing.conf "$HOME/missing.conf"

expect_failure "reject target inside dotfiles" \
    "$APP" link sample.conf "$DOTFILES_PATH/invalid.conf"

expect_success "list registered links" "$APP" list

if "$APP" list | grep -Fq 'sample.conf -> $HOME/sample.conf'; then
    pass "list formats home target"
else
    fail "list formats home target"
fi

# 4. Update and hard-link behavior
expect_success "update creates links" "$APP" update -y

if [[ -f "$HOME/sample.conf" ]]; then
    pass "update creates target"
else
    fail "update creates target"
fi

if [[ "$DOTFILES_PATH/sample.conf" -ef "$HOME/sample.conf" ]]; then
    pass "update creates a hard link"
else
    fail "update creates a hard link"
fi

# Replace the target with a different file to test -n.
rm -f -- "$HOME/sample.conf"
printf 'keep this content\n' > "$HOME/sample.conf"

expect_success "update -n skips existing target" "$APP" update -n

if [[ "$(cat "$HOME/sample.conf")" == "keep this content" ]]; then
    pass "update -n preserves existing target"
else
    fail "update -n preserves existing target"
fi

expect_success "update -y overrides existing target" "$APP" update -y

if [[ "$DOTFILES_PATH/sample.conf" -ef "$HOME/sample.conf" ]]; then
    pass "update -y replaces target with hard link"
else
    fail "update -y replaces target with hard link"
fi

# 5. Add a file without moving it
printf 'original configuration\n' > "$HOME/original.conf"

expect_success "add registers existing config" \
    "$APP" add "$HOME/original.conf" added.conf

if [[ "$DOTFILES_PATH/added.conf" -ef "$HOME/original.conf" ]]; then
    pass "add creates hard link"
else
    fail "add creates hard link"
fi

if [[ -f "$HOME/original.conf" ]]; then
    pass "add preserves original pathname"
else
    fail "add preserves original pathname"
fi

expect_failure "add rejects existing destination" \
    "$APP" add "$HOME/original.conf" added.conf

# 6. Add with move option
printf 'move me\n' > "$HOME/move.conf"

expect_success "add --move" \
    "$APP" add "$HOME/move.conf" moved.conf --move

if [[ -f "$DOTFILES_PATH/moved.conf" && ! -e "$HOME/move.conf" ]]; then
    pass "move keeps destination and removes original"
else
    fail "move keeps destination and removes original"
fi

if "$APP" list | grep -Fq 'moved.conf ->'; then
    fail "move does not register a link"
else
    pass "move does not register a link"
fi

# 7. Parent directory handling
printf 'nested config\n' > "$HOME/nested.conf"

expect_failure "add rejects missing parent without -p" \
    "$APP" add "$HOME/nested.conf" missing/parent.conf

expect_success "add -p creates parent directories" \
    "$APP" add "$HOME/nested.conf" nested/parent.conf -p

if [[ -f "$DOTFILES_PATH/nested/parent.conf" ]]; then
    pass "add -p creates destination"
else
    fail "add -p creates destination"
fi

# 8. Paths containing spaces
printf 'space config\n' > "$DOTFILES_PATH/space file.conf"

expect_success "link supports spaces in paths" \
    "$APP" link "space file.conf" "$HOME/space target.conf"

expect_success "update supports spaces in paths" "$APP" update -y

if [[ "$DOTFILES_PATH/space file.conf" -ef "$HOME/space target.conf" ]]; then
    pass "paths with spaces produce hard links"
else
    fail "paths with spaces produce hard links"
fi

# 9. Unlink
printf 'unlink target\n' > "$HOME/unlink.conf"
printf 'unlink source\n' > "$DOTFILES_PATH/unlink.conf"

expect_success "register unlink test mapping" \
    "$APP" link unlink.conf "$HOME/unlink.conf"

# Answer "n" when asked whether to delete the target.
if printf 'n\n' | "$APP" unlink "$HOME/unlink.conf" >"$OUTPUT" 2>&1; then
    pass "unlink removes mapping"
else
    fail "unlink removes mapping"
    sed -n '1,12p' "$OUTPUT" >&2
fi

if [[ -f "$HOME/unlink.conf" ]]; then
    pass "unlink preserves target when deletion declined"
else
    fail "unlink preserves target when deletion declined"
fi

if "$APP" list | grep -Fq 'unlink.conf ->'; then
    fail "unlinked mapping is absent"
else
    pass "unlinked mapping is absent"
fi

# 10. Git checkout
expect_success "checkout main branch" "$APP" checkout main

if [[ "$(git -C "$DOTFILES_PATH" branch --show-current)" == "main" ]]; then
    pass "checkout selects main"
else
    fail "checkout selects main"
fi

expect_failure "checkout rejects unknown branch" \
    "$APP" checkout definitely-not-a-real-branch

# 11. Commands requiring initialization
UNINITIALIZED="$TEST_ROOT/uninitialized"
mkdir -p "$UNINITIALIZED"

expect_failure "update rejects uninitialized directory" \
    env DOTFILES_PATH="$UNINITIALIZED" "$APP" update

expect_failure "link rejects uninitialized directory" \
    env DOTFILES_PATH="$UNINITIALIZED" "$APP" link a b

# 12. Summary
printf '\n=== Test Summary ===\n'
printf 'Passed: %d\n' "$PASS_COUNT"
printf 'Failed: %d\n' "$FAIL_COUNT"

if (( FAIL_COUNT > 0 )); then
    printf '\nSome tests failed. Review the output above.\n'
    exit 1
fi

printf '\nAll tests passed!\n'
exit 0

