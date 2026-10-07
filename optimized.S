.global process_text_asm_opt
.text
 
process_text_asm_opt:
    li t3, 0              # word_cnt = 0
    li t4, 0              # in_word = 0
    blez a1, end_opt      # handle len <= 0 once, before the loop
 
loop_opt:
    lbu t0, 0(a0)         # load current char
 
    # word count w/o branching
    sltiu t1, t0, 33      # t1 = 1 if c <= 32 (whitespace)
    xori t1, t1, 1        # t1 = 1 if non-whitespace
    xori t2, t4, 1        # t2 = !in_word
    and t2, t2, t1        # t2 = 1 only when a new word starts
    add t3, t3, t2        # word_cnt += t2
    mv t4, t1             # in_word = non-whitespace
 
    # lowercase w/o branching
    addi t5, t0, -65      # c - 'A'
    sltiu t5, t5, 26      # 1 if 'A' <= c <= 'Z' (unsigned compare)
    slli t5, t5, 5        # 32 if uppercase, else 0
    add t0, t0, t5        # add 32 only for uppercase
    sb t0, 0(a0)          # always store
 
    addi a0, a0, 1
    addi a1, a1, -1
    bgtz a1, loop_opt     # single branch per character
 
end_opt:
    mv a0, t3
    ret
 
