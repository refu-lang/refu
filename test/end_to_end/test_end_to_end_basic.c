#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <rflib/string/core.h>
#include <ast/ast.h>

#include "testsupport_end_to_end.h"

#include CLIB_TEST_HELPERS

START_TEST (test_smoke) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",
            "fn main()->u32{return 42}")
    };
    ck_end_to_end_run(inputs, 42);
} END_TEST

START_TEST (test_addition) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",
            "fn main()->u32{return 12 + 22}")
    };
    ck_end_to_end_run(inputs, 34);
} END_TEST

START_TEST (test_multiple_real_arithmetic) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "return (12 + (22 * 12 - 33) + (121 * 56 - 29)) / 233\n"
            "}")
    };
    ck_end_to_end_run(inputs, 30);
} END_TEST

START_TEST (test_negative_integer_constants) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "a:i32 = -34\n"
            "b:u32 = 44 + a\n"
            "return b"
            "}")
    };
    ck_end_to_end_run(inputs, 10);
} END_TEST

START_TEST (test_print_string) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "s:string = \"Celina\"\n"
            "print(s)\n"
            "print(\" is awesome\")\n"
            "return 13\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("Celina is awesome");
    ck_end_to_end_run(inputs, 13, &output);
} END_TEST

START_TEST (test_print_string_literal) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "print(\"Hello World\")\n"
            "return 13\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("Hello World");
    ck_end_to_end_run(inputs, 13, &output);
} END_TEST

START_TEST (test_print_integer) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "a:i64 = 3245\n"
            "print(a)\n"
            "print(65)\n"
            "return 1\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("324565");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_print_member_access) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "type person { name:string, age:u32}\n"
            "fn main()->u32{\n"
            "p:person = person(\"Moriarty\", 45)\n"
            "print(p.name)\n"
            "return p.age\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("Moriarty");
    ck_end_to_end_run(inputs, 45, &output);
} END_TEST

START_TEST (test_type_decl) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "type foo {a:i32, b:f32 }\n"
            "fn main()->u32{\n"
            "t:foo = foo(24, 0.222)\n"
            "return t.a\n"
            "}")
    };
    ck_end_to_end_run(inputs, 24);
} END_TEST

START_TEST (test_type_member_access) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:u8, b:u16, c:u32, d:u64 }\n"
        "fn main()->u32{\n"
        "t:foo = foo(24, 345, 96, 19234)\n"
        "return t.c\n"
        "}")
    };
    ck_end_to_end_run(inputs, 96);
} END_TEST

START_TEST (test_sum_type_creation) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:i32 | b:string }\n"
        "fn main()->u32{\n"
        "t1:foo = foo(34)\n"
        "t2:foo = foo(\"hello\")\n"
        "return 13\n"
        "}")
    };
    ck_end_to_end_run(inputs, 13);
} END_TEST

START_TEST (test_big_sum_type) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:u64, b:f32 | c:i64, d:u64 | e:i8, f:u64}\n"
        "fn main()->u32{\n"
        "t1:foo = foo(5644246, 3.21)\n"
        "return 13\n"
        "}")
    };
    ck_end_to_end_run(inputs, 13);
} END_TEST

START_TEST (test_function_creation_and_call_noarg) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn foo() { print(\"foo\") }\n"
        "fn main()->u32{\n"
        "foo()\n"
        "return 1\n"
        "}")
    };
    ck_end_to_end_run(inputs, 1, NULL);
} END_TEST

START_TEST (test_function_creation_and_call_1arg) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn foo(a:u32) -> u32 { return a + 13 }\n"
        "fn main()->u32{\n"
        "return foo(7)\n"
        "}")
    };
    ck_end_to_end_run(inputs, 20, NULL);
} END_TEST

START_TEST (test_function_creation_and_call_2arg) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn foo(a:u32, b:u32) -> u32 { return a + b + 10 }\n"
        "fn main()->u32{\n"
        "return foo(3, 7)\n"
        "}")
    };
    ck_end_to_end_run(inputs, 20, NULL);
} END_TEST

START_TEST (test_function_creation_and_call_assigned_return) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn foo(a:u32, b:u32) -> u32 { return a + b + 10 }\n"
        "fn main()->u32{\n"
        "r:u32 = foo(3, 7)\n"
        "print(r)\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("20");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_function_creation_and_call_assigned_return_obj) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type person {name:string, age:u32}\n"
        "fn pcreate(s:string) -> person\n"
        "{\n"
         "    p:person = person(s, 13)\n"
         "    return p\n"
        "}\n"
        "fn main()->u32{\n"
        "    p:person = pcreate(\"Celinka\")\n"
        "    print(p.name)\n"
        "    return 22\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("Celinka");
    ck_end_to_end_run(inputs, 22, &output);
} END_TEST

START_TEST (test_function_with_defined_type_arg) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:u32, b:f32}\n"
        "fn take_foo(f:foo)\n"
        "{\n"
         "    print(f.a + 2)\n"
        "}\n"
        "fn main()->u32{\n"
        "    a:foo = foo(40, 2.124)\n"
        "    take_foo(a)\n"
        "    return 22\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("42");
    ck_end_to_end_run(inputs, 22, &output);
} END_TEST

START_TEST (test_function_with_defined_type_arg2) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:u32, b:f32}\n"
        "fn take_foo(f:foo)\n"
        "{\n"
         "    print(f.a + 2)\n"
        "}\n"
        "fn main()->u32{\n"
        "    take_foo(foo(40, 2.124))\n"
        "    return 22\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("42");
    ck_end_to_end_run(inputs, 22, &output);
} END_TEST

START_TEST (test_simple_if) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "if (2 > 1) {\n"
        "    print(\"yes\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("yes");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_simple_if_else) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:u32\n"
        "a = 15\n"
        "if a > 100 {\n"
        "    print(\"yes\")\n"
        "} else {\n"
        "    print(\"no\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("no");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_simple_if_elif_else) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:u32\n"
        "a = 63\n"
        "if a > 100 {\n"
        "    print(\"in if\")\n"
        "} elif a > 50 {\n"
        "    print(\"in elif\")\n"
        "} else {\n"
        "    print(\"in else\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("in elif");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_no_outer_return) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main() -> u32 {\n"
        "    if true { return 1 } else { return 0 }\n"
        "}")
    };
    ck_end_to_end_run(inputs, 1);
} END_TEST

START_TEST (test_equal) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a == 1342 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a == 5000 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b == 938375 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b == 2938349 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c == 14.532 {\n"
        "    print(\"nope\")\n"
        "}"
        "return 1\n"
        "}")
    };
    // note: not comparing floats for equality in the test result since it's ambiguous
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_not_equal) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a != 5000 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a != 1342 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b != 1421 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b != 938375 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c != 14.532 {\n"
        "    print(\"nope\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    // note: not comparing floats for uneequality in the test result since it's ambiguous
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_greater_than) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a > 100 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a > 5000 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b > 938345 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b > 2938349 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c > 14.532 {\n"
        "    print(\"nope\")\n"
        "} else {\n"
        "    print(\" here\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output here");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_greater_than_or_equal) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a >= 1342 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a >= 5000 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b >= 938375 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b >= 2938349 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c > 14.532 {\n"
        "    print(\"nope\")\n"
        "} elif c >= 3.14 {\n"
        "    print(\" here\")\n"
        "} else {\n"
        "    print(\"not here either\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output here");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_less_than) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a < 100002 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a < 100 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b < 9383450 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b < 23142 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c < 1.22 {\n"
        "    print(\"nope\")\n"
        "} else {\n"
        "    print(\" here\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output here");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_less_than_or_equal) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:i32 = 1342\n"
        "b:u64 = 938375\n"
        "c:f32 = 3.14\n"
        "if a <= 1342 {\n"
        "    print(\"expected\")\n"
        "}\n"
        "if a <= 123 {\n"
        "    print(\"not expected\")\n"
        "}\n"
        "if b <= 938375 {\n"
        "    print(\" output\")\n"
        "}\n"
        "if b <= 93829 {\n"
        "    print(\"won't see me\")\n"
        "}\n"
        "if c <= 2.32 {\n"
        "    print(\"nope\")\n"
        "} else {\n"
        "    print(\" here\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("expected output here");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_if_with_boolean) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "a:bool = false\n"
        "b:bool = true\n"
        "if (a) {\n"
        "    print(\"yin\")"
        "} else {\n"
        "    print(\"yang\")\n"
        "}\n"
        "if (b) {\n"
        "    print(\" yin\")"
        "} else {\n"
        "    print(\" yang\")\n"
        "}\n"
        "return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("yang yin");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_explicit_conversion_to_u8) {
    // TODO: refactor this test when print can output ints
    //       and when missing return in outer block works
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "    a:u64 = 4294967294\n"
            "    b:u8 = u8(a)\n"
            "    if b == 254 { print(\"yes\") } else { print(\"no\") }\n"
            "    return 1\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("yes");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_explicit_conversion_to_u16) {
    // TODO: refactor this test when print can output ints
    //       and when missing return in outer block works
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "    a:u64 = 4294967294\n"
            "    b:u16 = u16(a)\n"
            "    if b == 65534 { print(\"yes\") } else { print(\"no\") }\n"
            "    return 1\n"
            "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("yes");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_explicit_conversion_to_string) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
        "test_input_file.rf",

        "fn main()->u32{\n"
        "    a:string = string(42)\n"
        "    b:string = string(1.61803399)\n"
        "    c:string = string(true)\n"
        "    d:string = string(false)\n"
        "    print(a) print(\" \") print(b)\n"
        "    print(\" \") print(c)\n"
        "    print(\" \") print(d)\n"
        "    return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("42 1.6180 true false");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_explicit_conversion_to_string_from_nonconst_bool) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "    a:string = string(42 == 42)\n"
        "    b:string = string(1 == 3)\n"
        "    print(a) print(\" \") print(b)\n"
        "    return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("true false");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_unaryop_minus) {
    // TODO: refactor this test when print can output ints
    //       and when missing return in outer block works
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "    a:u32 = 64\n"
        "    b:i32 = -a\n"
        "    if b == -64 { print(\"ok\") } else { print(\"no\") }\n"
        "    return 1\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("ok");
    ck_end_to_end_run(inputs, 1, &output);
} END_TEST

START_TEST (test_unaryop_post_inc) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "    a:u32 = 64\n"
        "    b:u32 = ++a\n"
        "    return b\n"
        "}")
    };
    ck_end_to_end_run(inputs, 65);
} END_TEST

START_TEST (test_unaryop_post_dec) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn main()->u32{\n"
        "    a:u32 = 64\n"
        "    b:u32 = --a\n"
        "    return b\n"
        "}")
    };
    ck_end_to_end_run(inputs, 63);
} END_TEST

START_TEST (test_matchexpr_1) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:i32 | b:string }\n"
        "fn main()->u32{\n"
        "t1:foo = foo(29)\n"
        "r:i32 = match t1 {\n"
        " a:i32 => a\n"
        " _ => 0\n"
        "}"
        "return r\n"
        "}")
    };
    ck_end_to_end_run(inputs, 29);
} END_TEST

START_TEST (test_matchexpr_2) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "type foo {a:i32 | b:string }\n"
        "fn main()->u32{\n"
        "t1:foo = foo(29)\n"
        "t2:foo = foo(\"hello\")\n"
        "r:i32 = match t1 {\n"
        " a:i32 => a\n"
        " _ => 0\n"
        "}"
        "s:string = match t2 {\n"
        " b:string => b\n"
        " _ => \"\"\n"
        "}\n"
        "print(s)\n"
        "return r\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("hello");
    ck_end_to_end_run(inputs, 29, &output, "test_input_file.rf");
} END_TEST

START_TEST (test_matchexpr_in_functions) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

        "fn action(a:i32 | b:string)\n"
        // TODO: Change the below to print(string(a)) when conversion of non const ints can happen
        "a:i32    => print(\"change\")\n"
        "b:string => print(b)\n"
        "\n"
        "fn main()->u32{\n"
        "action(3)\n"
        "action(\" friends\")\n"
        "return 0\n"
        "}")
    };
    static const struct RFstring output = RF_STRING_STATIC_INIT("change friends");
    ck_end_to_end_run(inputs, 0, &output);
} END_TEST

START_TEST (test_fixedarr_1) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "a:u64[3] = [1, 73, 3]\n"
            "b:u64 = a[1]\n"
            "return b"
            "}"
        )};
    ck_end_to_end_run(inputs, 73, NULL);
} END_TEST

START_TEST (test_fixedarr_2) {
    struct test_input_pair inputs[] = {
        TEST_DECL_SRC(
            "test_input_file.rf",

            "fn main()->u32{\n"
            "a:string[3] = [\"Berlin\", \"Krosno\", \"Thessaloniki\"]\n"
            "print(a[2])\n"
            "return 3"
            "}"
        )};
    static const struct RFstring output = RF_STRING_STATIC_INIT("Thessaloniki");
    ck_end_to_end_run(inputs, 3, &output);
} END_TEST

Suite *end_to_end_basic_suite_create(void)
{
    Suite *s = suite_create("end_to_end_basic");

    TCase *st_basic = tcase_create("end_to_end_basic");
    tcase_add_checked_fixture(st_basic,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_basic, test_smoke);
    tcase_add_test(st_basic, test_addition);
    tcase_add_test(st_basic, test_multiple_real_arithmetic);
    tcase_add_test(st_basic, test_negative_integer_constants);

    TCase *st_print = tcase_create("end_to_end_print");
    tcase_add_checked_fixture(st_print,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_print, test_print_string);
    tcase_add_test(st_print, test_print_string_literal);
    tcase_add_test(st_print, test_print_integer);
    tcase_add_test(st_print, test_print_member_access);

    TCase *st_basic_types = tcase_create("end_to_end_basic_types");
    tcase_add_checked_fixture(st_basic_types,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_basic_types, test_type_decl);
    tcase_add_test(st_basic_types, test_type_member_access);
    tcase_add_test(st_basic_types, test_sum_type_creation);
    tcase_add_test(st_basic_types, test_big_sum_type);

    TCase *st_functions = tcase_create("end_to_end_functions");
    tcase_add_checked_fixture(st_functions,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_functions, test_function_creation_and_call_noarg);
    tcase_add_test(st_functions, test_function_creation_and_call_1arg);
    tcase_add_test(st_functions, test_function_creation_and_call_2arg);
    tcase_add_test(st_functions, test_function_creation_and_call_assigned_return);
    tcase_add_test(st_functions, test_function_creation_and_call_assigned_return_obj);
    tcase_add_test(st_functions, test_function_with_defined_type_arg);
    tcase_add_test(st_functions, test_function_with_defined_type_arg2);

    TCase *st_control_flow = tcase_create("end_to_end_control_flow");
    tcase_add_checked_fixture(st_control_flow,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_control_flow, test_simple_if);
    tcase_add_test(st_control_flow, test_simple_if_else);
    tcase_add_test(st_control_flow, test_simple_if_elif_else);
    tcase_add_test(st_control_flow, test_no_outer_return);

    TCase *st_comparisons = tcase_create("end_to_end_comparisons");
    tcase_add_checked_fixture(st_comparisons,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_comparisons, test_equal);
    tcase_add_test(st_comparisons, test_not_equal);
    tcase_add_test(st_comparisons, test_greater_than);
    tcase_add_test(st_comparisons, test_greater_than_or_equal);
    tcase_add_test(st_comparisons, test_less_than);
    tcase_add_test(st_comparisons, test_less_than_or_equal);
    tcase_add_test(st_comparisons, test_if_with_boolean);

    TCase *st_explicit_conversions = tcase_create("end_to_end_explicit_conversions");
    tcase_add_checked_fixture(st_explicit_conversions,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_explicit_conversions, test_explicit_conversion_to_u8);
    tcase_add_test(st_explicit_conversions, test_explicit_conversion_to_u16);
    tcase_add_test(st_explicit_conversions, test_explicit_conversion_to_string);
    tcase_add_test(st_explicit_conversions, test_explicit_conversion_to_string_from_nonconst_bool);

    TCase *st_unary_operations = tcase_create("end_to_end_unary_operations");
    tcase_add_checked_fixture(st_unary_operations,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_unary_operations, test_unaryop_minus);
    tcase_add_test(st_unary_operations, test_unaryop_post_inc);
    tcase_add_test(st_unary_operations, test_unaryop_post_dec);

    TCase *st_match_expr = tcase_create("end_to_end_match_expressions");
    tcase_add_checked_fixture(st_match_expr,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_match_expr, test_matchexpr_1);
    tcase_add_test(st_match_expr, test_matchexpr_2);
    tcase_add_test(st_match_expr, test_matchexpr_in_functions);

    TCase *st_arrays = tcase_create("end_to_end_arrays");
    tcase_add_checked_fixture(st_arrays,
                              setup_end_to_end_tests,
                              teardown_end_to_end_tests);
    tcase_add_test(st_arrays, test_fixedarr_1);
    tcase_add_test(st_arrays, test_fixedarr_2);

    suite_add_tcase(s, st_basic);
    suite_add_tcase(s, st_print);
    suite_add_tcase(s, st_basic_types);
    suite_add_tcase(s, st_functions);
    suite_add_tcase(s, st_control_flow);
    suite_add_tcase(s, st_comparisons);
    suite_add_tcase(s, st_explicit_conversions);
    suite_add_tcase(s, st_unary_operations);
    suite_add_tcase(s, st_match_expr);
    suite_add_tcase(s, st_arrays);

    return s;
}
