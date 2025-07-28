#include "string_util.h"
#include <stdio.h>
#include <string.h>

void test_str_len() {
    printf("[str_len] ");
    printf("%s\n", str_len("abc") == 3 ? "PASS" : "FAIL");
}

void test_str_cpy() {
    char buf[10];
    str_cpy(buf, "hello", sizeof(buf));
    printf("[str_cpy] %s\n", strcmp(buf, "hello") == 0 ? "PASS" : "FAIL");
}

void test_str_cmp() {
    printf("[str_cmp] %s\n", str_cmp("abc", "abc", 3) == 0 ? "PASS" : "FAIL");
    printf("[str_cmp] %s\n", str_cmp("abc", "abd", 3) < 0 ? "PASS" : "FAIL");
}

void test_str_chr() {
    printf("[str_chr] %s\n", str_chr("hello", 'e') != NULL ? "PASS" : "FAIL");
    printf("[str_chr] %s\n", str_chr("hello", 'z') == NULL ? "PASS" : "FAIL");
}

void test_str_str() {
    printf("[str_str] %s\n", str_str("abcdef", "cd") != NULL ? "PASS" : "FAIL");
    printf("[str_str] %s\n", str_str("abcdef", "gh") == NULL ? "PASS" : "FAIL");
}

void test_str_tok() {
    char buf[32] = "a,b,c";
    char* token = str_tok(buf, ",");
    int pass = 1;
    if (!token || strcmp(token, "a") != 0) pass = 0;
    token = str_tok(NULL, ",");
    if (!token || strcmp(token, "b") != 0) pass = 0;
    token = str_tok(NULL, ",");
    if (!token || strcmp(token, "c") != 0) pass = 0;
    token = str_tok(NULL, ",");
    if (token != NULL) pass = 0;
    printf("[str_tok] %s\n", pass ? "PASS" : "FAIL");
}

void test_str_to_upper() {
    char buf[16] = "abcDEF";
    str_to_upper(buf);
    printf("[str_to_upper] %s\n", strcmp(buf, "ABCDEF") == 0 ? "PASS" : "FAIL");
}

void test_str_to_lower() {
    char buf[16] = "ABCdef";
    str_to_lower(buf);
    printf("[str_to_lower] %s\n", strcmp(buf, "abcdef") == 0 ? "PASS" : "FAIL");
}

void test_str_reverse() {
    char buf[16] = "abcdef";
    str_reverse(buf);
    printf("[str_reverse] %s\n", strcmp(buf, "fedcba") == 0 ? "PASS" : "FAIL");
}

void test_hex_str_to_uint() {
    u32 val = 0;
    int pass = hex_str_to_uint("1A3F", &val) && val == 0x1A3F;
    pass = pass && !hex_str_to_uint("xyz", &val);
    printf("[hex_str_to_uint] %s\n", pass ? "PASS" : "FAIL");
}

void test_uint_to_hex_str() {
    char buf[9];
    uint_to_hex_str(0x1A3F, buf, sizeof(buf));
    printf("[uint_to_hex_str] %s\n", strcmp(buf, "1A3F") == 0 ? "PASS" : "FAIL");
}

int main() {
    test_str_len();
    test_str_cpy();
    test_str_cmp();
    test_str_chr();
    test_str_str();
    test_str_tok();
    test_str_to_upper();
    test_str_to_lower();
    test_str_reverse();
    test_hex_str_to_uint();
    test_uint_to_hex_str();
    return 0;
}
