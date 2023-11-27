.include "../macros_library/common_macros.m"
.include "../macros_library/find_lb.m"
.include "../macros_library/create_result.m"

.data: # Here all the test cases used in the Python test program are described
    text_good: .asciz "Test passed successfully\n"
    text_bad: .asciz "Test not passed\n"

    # Test case 1
    input1: .asciz "ffde42\n2aabf"
    output1_1: .asciz "de4"
    output1_2: .asciz "ab"

    # Test case 2
    input2: .asciz "gurpwei0gw4ug9\n3q0g-ir0awok]f"
    output2_1: .asciz "upe49"
    output2_2: .asciz "3q-aok]f"

    # Test case 3
    input3: .asciz "01\nab"
    output3_1: .asciz "01"
    output3_2: .asciz "ab"

    # Test case 4
    input4: .asciz "q3rpgy2efuhij34it9058hrekg0voq3rpgy2efuhij34it9058hrekg0voq3rpgy2efuhij34it9058hrekg0vo\n2-39rhqfgreahpifno[aein34=902-39rhqfgreahpifno[aein34=902-39rhqfgreahpifno[aein34=90"
    output4_1: .asciz "yujt58kv"
    output4_2: .asciz "-an[="

    # Test case 5
    input5: .asciz "\n"
    output5_1: .asciz ""
    output5_2: .asciz ""

    # Test case 6
    input6: .asciz "20=42-=NO[IAO-FEKWAQN43O-K\n20==04FIJi0g9-u35ri0=w=dwlf-24h56u"
    output6_1: .asciz "NO[AEKWQ"
    output6_2: .asciz "Jig9u5rwdlfh6"


.macro check_case(%input, %output1, %output2) # Macro to check if the test cases passed
    push(s0)
    push(s1)
    push(s2)

    la s0 %input
    la s1 %output1
    la s2 %output2

    find_str_size(s0)
    mv s3 a0

    find_lb(s0, a0)

    mv a1 a0
    mv a0 s0
    mv a2 s3

    create_result(a0, a1, a2)
    mv s4 a0
    mv s5 a2

    compare_str(s1, s4)
    bnez a0 bad_res

    compare_str(s2, s5)
    bnez a0 bad_res
    j good_res

    bad_res:
        la a0 text_bad
        li a7 4
        ecall
        j end

    good_res:
        la a0 text_good
        li a7 4
        ecall
        j end
    end:
        pop(s2)
        pop(s1)
        pop(s0)
.end_macro

.macro compare_str(%x1, %x2) # Macro for comparing two strings
    push(s0)
    push(s1)
    push(s2)
    push(s3)
    mv s0 %x1 # String 1
    mv s1 %x2 # String 2

    mv a0 s0
    find_str_size(%x1)
    mv s2 a0 # Size of String 1

    mv a0 s1
    find_str_size(%x2)
    mv s3 a0 # Size of String 2

    bne s2 s3 not_equal

    li t0 0

    loop:
        lb t1 (s0)
        lb t2 (s1)

        bne t1 t2 not_equal
        addi t0 t0 1

        blt t0 s2 loop

    j equal

    equal:
        li a0 0
        j end

    not_equal:
        li a0 -1
        j end
    end:
        pop(s3)
        pop(s2)
        pop(s1)
        pop(s0)
.end_macro

.macro find_str_size(%x1) # Macro for finding the size of a string
    li t0 0
    mv t1 %x1

    loop:
        lb t2 (t1)
        beqz t2 end_loop

        addi t0 t0 1
        addi t1 t1 1
        b loop

    end_loop:
        mv a0 t0
.end_macro

.text
    check_case(input1, output1_1, output1_2)
    check_case(input2, output2_1, output2_2)
    check_case(input3, output3_1, output3_2)
    check_case(input4, output4_1, output4_2)
    check_case(input5, output5_1, output5_2)
    check_case(input6, output6_1, output6_2)
    li a7 10
    ecall