#!/bin/bash

################################################################################
# Ornidia Project Integrity Verification Script
# This script verifies the integrity of the Ornidia solar monitoring project
################################################################################

# Don't exit on errors, we want to continue testing
# set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Test results file
RESULTS_FILE="/tmp/ornidia_test_results.txt"
echo "Ornidia Project Integrity Verification Results" > "$RESULTS_FILE"
echo "================================================" >> "$RESULTS_FILE"
echo "Date: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
    echo "✓ $1" >> "$RESULTS_FILE"
    ((PASSED_TESTS++))
    ((TOTAL_TESTS++))
}

print_failure() {
    echo -e "${RED}✗ $1${NC}"
    echo "✗ $1" >> "$RESULTS_FILE"
    ((FAILED_TESTS++))
    ((TOTAL_TESTS++))
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
    echo "⚠ $1" >> "$RESULTS_FILE"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Navigate to project root
cd "$(dirname "$0")"
PROJECT_ROOT=$(pwd)

print_header "ORNIDIA PROJECT INTEGRITY VERIFICATION"
echo ""

################################################################################
# Test 1: Verify project structure
################################################################################
print_header "1. PROJECT STRUCTURE VERIFICATION"

check_directory() {
    if [ -d "$1" ]; then
        print_success "Directory exists: $1"
    else
        print_failure "Directory missing: $1"
    fi
}

check_file() {
    if [ -f "$1" ]; then
        print_success "File exists: $1"
    else
        print_failure "File missing: $1"
    fi
}

# Check main directories
check_directory "Arduino"
check_directory "Arduino/solar_monitor"
check_directory "test"
check_directory "datasheet"
check_file "README.md"
check_file ".gitignore"

echo ""

################################################################################
# Test 2: Verify Arduino sketches syntax
################################################################################
print_header "2. ARDUINO SKETCH VALIDATION"

validate_sketch() {
    local sketch_file=$1
    local sketch_name=$(basename "$sketch_file" .ino)
    
    print_info "Validating: $sketch_file"
    
    # Check if file exists
    if [ ! -f "$sketch_file" ]; then
        print_failure "Sketch not found: $sketch_file"
        return
    fi
    
    # Basic syntax checks
    local errors=0
    
    # Check for basic Arduino structure
    if ! grep -q "void setup()" "$sketch_file"; then
        print_warning "Missing setup() function in $sketch_name"
        ((errors++))
    fi
    
    if ! grep -q "void loop()" "$sketch_file"; then
        print_warning "Missing loop() function in $sketch_name"
        ((errors++))
    fi
    
    # Check for balanced braces
    local open_braces=$(grep -o "{" "$sketch_file" | wc -l)
    local close_braces=$(grep -o "}" "$sketch_file" | wc -l)
    
    if [ "$open_braces" -ne "$close_braces" ]; then
        print_failure "Unbalanced braces in $sketch_name (open: $open_braces, close: $close_braces)"
    else
        print_success "Balanced braces in $sketch_name"
    fi
    
    # Check for balanced parentheses in function definitions
    if grep -E "void [a-zA-Z_][a-zA-Z0-9_]*\([^\)]*$" "$sketch_file" > /dev/null; then
        print_warning "Potential unbalanced parentheses in function definitions in $sketch_name"
    fi
    
    # Check file is not empty
    if [ ! -s "$sketch_file" ]; then
        print_failure "Sketch file is empty: $sketch_name"
    else
        print_success "Sketch file has content: $sketch_name"
    fi
    
    # Check for Serial.begin in setup (common practice)
    if grep -A 20 "void setup()" "$sketch_file" | grep -q "Serial.begin"; then
        print_success "Serial communication initialized in $sketch_name"
    fi
    
    if [ $errors -eq 0 ]; then
        print_success "Basic validation passed for $sketch_name"
    fi
}

# Find and validate all .ino files
echo ""
print_info "Searching for Arduino sketches..."
while IFS= read -r sketch; do
    validate_sketch "$sketch"
    echo ""
done < <(find "$PROJECT_ROOT" -name "*.ino" -type f | sort)

################################################################################
# Test 3: Verify library dependencies
################################################################################
print_header "3. LIBRARY DEPENDENCY VERIFICATION"

# Extract all #include statements
echo "" >> "$RESULTS_FILE"
echo "Required Libraries:" >> "$RESULTS_FILE"
echo "==================" >> "$RESULTS_FILE"

find "$PROJECT_ROOT" -name "*.ino" -type f -exec grep -h "^#include" {} \; | sort -u | while read -r include; do
    lib_name=$(echo "$include" | sed 's/#include [<"]\(.*\)[>"]/\1/')
    echo "  - $lib_name" >> "$RESULTS_FILE"
    print_info "Required library: $lib_name"
done

print_success "Library dependency analysis completed"

echo ""

################################################################################
# Test 4: Main sketches validation
################################################################################
print_header "4. MAIN SKETCHES DETAILED VALIDATION"

# Main solar monitor sketch
MAIN_SKETCH="$PROJECT_ROOT/Arduino/solar_monitor/solar_monitor.ino"
if [ -f "$MAIN_SKETCH" ]; then
    print_info "Validating main solar monitor sketch..."
    
    # Check for required functions
    if grep -q "void leerSensores()" "$MAIN_SKETCH"; then
        print_success "leerSensores() function found"
    else
        print_failure "leerSensores() function missing"
    fi
    
    if grep -q "void controlarLEDs()" "$MAIN_SKETCH"; then
        print_success "controlarLEDs() function found"
    else
        print_failure "controlarLEDs() function missing"
    fi
    
    if grep -q "void manejarClienteWeb()" "$MAIN_SKETCH"; then
        print_success "manejarClienteWeb() function found"
    else
        print_failure "manejarClienteWeb() function missing"
    fi
    
    # Check for pin definitions
    if grep -q "#define.*LED_" "$MAIN_SKETCH"; then
        print_success "LED pin definitions found"
    fi
    
    if grep -q "#define.*PIN_" "$MAIN_SKETCH"; then
        print_success "Sensor pin definitions found"
    fi
fi

# Phase 1 test sketch
FASE1_SKETCH="$PROJECT_ROOT/test/fase1/fase1.ino"
if [ -f "$FASE1_SKETCH" ]; then
    print_info "Validating Phase 1 test sketch..."
    
    if grep -q "struct KalmanFilter" "$FASE1_SKETCH"; then
        print_success "Kalman filter implementation found"
    else
        print_failure "Kalman filter implementation missing"
    fi
    
    if grep -q "void calibrarSistema()" "$FASE1_SKETCH"; then
        print_success "System calibration function found"
    else
        print_failure "System calibration function missing"
    fi
fi

echo ""

################################################################################
# Test 5: Web files validation
################################################################################
print_header "5. WEB INTERFACE FILES VALIDATION"

WEB_DIR="$PROJECT_ROOT/Arduino/solar_monitor/data"
if [ -d "$WEB_DIR" ]; then
    # Check HTML file
    if [ -f "$WEB_DIR/index.html" ]; then
        print_success "index.html found"
        
        # Basic HTML validation
        if grep -q "<!DOCTYPE" "$WEB_DIR/index.html" || grep -q "<html" "$WEB_DIR/index.html"; then
            print_success "Valid HTML structure in index.html"
        else
            print_warning "Possible HTML structure issues in index.html"
        fi
    else
        print_failure "index.html missing"
    fi
    
    # Check CSS file
    if [ -f "$WEB_DIR/style.css" ]; then
        print_success "style.css found"
    else
        print_failure "style.css missing"
    fi
    
    # Check JavaScript file
    if [ -f "$WEB_DIR/script.js" ]; then
        print_success "script.js found"
        
        # Check for balanced braces in JavaScript
        open_js=$(grep -o "{" "$WEB_DIR/script.js" | wc -l)
        close_js=$(grep -o "}" "$WEB_DIR/script.js" | wc -l)
        
        if [ "$open_js" -eq "$close_js" ]; then
            print_success "Balanced braces in script.js"
        else
            print_failure "Unbalanced braces in script.js"
        fi
    else
        print_failure "script.js missing"
    fi
else
    print_warning "Web data directory not found"
fi

echo ""

################################################################################
# Test 6: Documentation validation
################################################################################
print_header "6. DOCUMENTATION VALIDATION"

if [ -f "$PROJECT_ROOT/README.md" ]; then
    print_success "README.md exists"
    
    # Check if README has content
    if [ -s "$PROJECT_ROOT/README.md" ]; then
        word_count=$(wc -w < "$PROJECT_ROOT/README.md")
        print_success "README.md has content ($word_count words)"
        
        # Check for hardware documentation
        if grep -qi "hardware\|componente\|sensor" "$PROJECT_ROOT/README.md"; then
            print_success "Hardware documentation found in README"
        fi
        
        # Check for connection diagrams
        if grep -qi "conexión\|pin\|diagrama" "$PROJECT_ROOT/README.md"; then
            print_success "Connection information found in README"
        fi
    else
        print_failure "README.md is empty"
    fi
else
    print_failure "README.md missing"
fi

# Check for datasheets
if [ -d "$PROJECT_ROOT/datasheet" ]; then
    datasheet_count=$(find "$PROJECT_ROOT/datasheet" -type f | wc -l)
    print_success "Datasheet directory exists with $datasheet_count files"
else
    print_warning "Datasheet directory missing"
fi

echo ""

################################################################################
# Test 7: Git integrity
################################################################################
print_header "7. GIT REPOSITORY INTEGRITY"

if [ -d "$PROJECT_ROOT/.git" ]; then
    print_success "Git repository initialized"
    
    # Check for .gitignore
    if [ -f "$PROJECT_ROOT/.gitignore" ]; then
        print_success ".gitignore exists"
        
        # Check if common build artifacts are ignored
        if grep -q "\.o\|\.exe\|\.out" "$PROJECT_ROOT/.gitignore"; then
            print_success "Build artifacts are ignored"
        fi
    else
        print_warning ".gitignore missing"
    fi
    
    # Check repository status
    if git -C "$PROJECT_ROOT" status &>/dev/null; then
        print_success "Git repository is valid"
    else
        print_failure "Git repository has issues"
    fi
else
    print_failure "Not a git repository"
fi

echo ""

################################################################################
# Final Summary
################################################################################
print_header "VERIFICATION SUMMARY"

echo ""
echo "Total Tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"

echo "" >> "$RESULTS_FILE"
echo "================================================" >> "$RESULTS_FILE"
echo "SUMMARY" >> "$RESULTS_FILE"
echo "================================================" >> "$RESULTS_FILE"
echo "Total Tests: $TOTAL_TESTS" >> "$RESULTS_FILE"
echo "Passed: $PASSED_TESTS" >> "$RESULTS_FILE"
echo "Failed: $FAILED_TESTS" >> "$RESULTS_FILE"

if [ $FAILED_TESTS -eq 0 ]; then
    print_success "ALL TESTS PASSED! ✓"
    echo "Result: ALL TESTS PASSED ✓" >> "$RESULTS_FILE"
    echo ""
    echo -e "${GREEN}Project integrity verified successfully!${NC}"
    EXIT_CODE=0
else
    print_failure "$FAILED_TESTS TESTS FAILED!"
    echo "Result: $FAILED_TESTS TESTS FAILED ✗" >> "$RESULTS_FILE"
    echo ""
    echo -e "${RED}Project has integrity issues that need attention.${NC}"
    EXIT_CODE=1
fi

echo ""
echo "Detailed results saved to: $RESULTS_FILE"
echo ""

exit $EXIT_CODE
