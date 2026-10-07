.global process_text_asm
.text

process_text_asm:

    li t1, 65         #Capital A in ASCII
    li t2, 90         #Capital Z in ASCII

    li t3, 0          #word_cnt = 0  #word count counter
    li t4, 0          #in_word = 0   #track if currently in a word
    
    li t5, 32         # ' '
    li t6, 10         # '\n'
    li a2, 9          # '\t'
    li a3, 13         # '\r'

loop:
    blez a1, end      #check if number of characters (a1) reached 0

    lbu t0, 0(a0)     #load the current character to t0

    #jump to "is_space" if whitespace
    beq t0, t5, is_space
    beq t0, t6, is_space
    beq t0, a2, is_space
    beq t0, a3, is_space

    #jump to "check_lowercase" if previous char was already in_word=1
    bnez t4, check_lowercase 
    
    li t4, 1          #set in_word from to 1
    addi t3, t3, 1   #add 1 to word_cnt

check_lowercase:
    #check if character is a capital letter through ASCII
    blt t0, t1, next  #if char < 'A' skip
    bgt t0, t2, next  #if char > 'Z' skip

    addi t0, t0, 32   #change capital to small letter

    sb t0, 0(a0)      #store back in array
    j next

is_space:
    li t4, 0          #set in_word from to 0

next:
    addi a1, a1, -1   #decrement total num of characters
    addi a0, a0, 1    #go to next character
    j loop

end:
    addi a0, t3, 0    #move word_cnt to a0
    ret 


