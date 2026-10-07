#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define ITERATIONS 100000

extern int process_text_asm(char* str, int len);
extern int process_text_asm_opt(char* str, int len);

//normalizes capital letters to small & performs word count
int process_text_c(char* str, int len) {

    bool in_word = false; //track if currently in a word
    int word_cnt = 0;     //word count counter

    for (int i = 0; i < len; i++) {

        char c = str[i];

        //whitespace means cursor is not inside a word
        if (c=='\n' || c=='\t' || c=='\r' || c==' ')
            in_word = false;
        else {
            //if current char is not a whitespace, check if the preceeding char is a whitespace
            //checks if new word has started
            if (!in_word){
                in_word = true;
                word_cnt++;
            }

            //normalize capital letters to small
            if (c >= 65 && c <= 90)
                str[i] = c + 32;
        }
    }

    return word_cnt;
}

int main() {
    bool match = true;
    bool match_opt = true;
    
    char original[] = "wE'Re NO sTrAnGeRs tO LoVe\nyOU kNOw THe rULes ANd sO dO i\nA FuLL cOmMiTmENt'S wHaT I'm ThInKiNg oF\nyOU wOUldN't gEt tHiS fROm aNy oThEr gUy";
    
    char text[sizeof(original)];
    char text2[sizeof(original)];
    char text3[sizeof(original)];
    
    int len = strlen(original);
    int word_cnt_c = 0;
    int word_cnt_asm = 0;
    int word_cnt_asm_opt = 0;

    printf("Original Text: \n\n%s\n\n", original);

    //strcpy baseline (reset cost only), subtracted from the results below
    struct timespec start_base, end_base;

    clock_gettime(CLOCK_MONOTONIC, &start_base);
    for (int i = 0; i < ITERATIONS; i++) {
        strcpy(text, original); // reset string each run
        __asm__ volatile("" ::: "memory"); // prevents the compiler from removing the copy
    }
    clock_gettime(CLOCK_MONOTONIC, &end_base);

    double baseline_ms = ((end_base.tv_sec - start_base.tv_sec) * 1000.0) + 
                         ((end_base.tv_nsec - start_base.tv_nsec) / 1000000.0);

    printf("strcpy Baseline (%d iterations): %.4f ms\n\n", ITERATIONS, baseline_ms);

    //C implementation
    //1000 iterations to get a larger measurable time
    struct timespec start_c, end_c;
    
    clock_gettime(CLOCK_MONOTONIC, &start_c);
    for (int i = 0; i < ITERATIONS; i++) {
        strcpy(text, original); // reset string each run
        process_text_c(text, len);

        if (i==ITERATIONS-1)
            word_cnt_c = process_text_c(text, len);
    }
   
    clock_gettime(CLOCK_MONOTONIC, &end_c);

    double time_c_ms = ((end_c.tv_sec - start_c.tv_sec) * 1000.0) + 
                       ((end_c.tv_nsec - start_c.tv_nsec) / 1000000.0);

    printf("Normalize and Word Count (C): \n\n%s\n", text);
    printf("\nWord Count (C): %d\n", word_cnt_c);
    printf("C Execution Time (%d iterations): %.4f ms\n", ITERATIONS, time_c_ms);
    printf("C Net Time (baseline subtracted): %.4f ms\n\n", time_c_ms - baseline_ms);


    //Assembly implementation
    struct timespec start_asm, end_asm;

    clock_gettime(CLOCK_MONOTONIC, &start_asm);
    for (int i = 0; i < ITERATIONS; i++) {
        strcpy(text2, original); // reset string each run
        process_text_asm(text2, len);

        if (i==ITERATIONS-1)
            word_cnt_asm = process_text_asm(text2, len);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_asm);

    double time_asm_ms = ((end_asm.tv_sec - start_asm.tv_sec) * 1000.0) + 
                         ((end_asm.tv_nsec - start_asm.tv_nsec) / 1000000.0);

    printf("Normalized Text (ASM): \n\n%s\n", text2);
    printf("\nWord Count (ASM): %d\n", word_cnt_asm);
    printf("ASM Execution Time (%d iterations): %.4f ms\n", ITERATIONS, time_asm_ms);
    printf("ASM Net Time (baseline subtracted): %.4f ms\n\n", time_asm_ms - baseline_ms);


    //Optimized (branchless) assembly implementation
    struct timespec start_asm_opt, end_asm_opt;

    clock_gettime(CLOCK_MONOTONIC, &start_asm_opt);
    for (int i = 0; i < ITERATIONS; i++) {
        strcpy(text3, original); // reset string each run
        process_text_asm_opt(text3, len);

        if (i==ITERATIONS-1)
            word_cnt_asm_opt = process_text_asm_opt(text3, len);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_asm_opt);

    double time_asm_opt_ms = ((end_asm_opt.tv_sec - start_asm_opt.tv_sec) * 1000.0) + 
                             ((end_asm_opt.tv_nsec - start_asm_opt.tv_nsec) / 1000000.0);

    printf("Normalized Text (ASM Optimized): \n\n%s\n", text3);
    printf("\nWord Count (ASM Optimized): %d\n", word_cnt_asm_opt);
    printf("ASM Optimized Execution Time (%d iterations): %.4f ms\n", ITERATIONS, time_asm_opt_ms);
    printf("ASM Optimized Net Time (baseline subtracted): %.4f ms\n\n", time_asm_opt_ms - baseline_ms);

    //Check for mismatch
    for (int i = 0; i < len; i++) {
        if (text[i] != text2[i]) {
            printf("Mismatch at index %d:\n C: %c  ASM: %c\n\n", i, text[i], text2[i]);
            if (match)
                match = false;
        }
    }

    if (match) {
        printf("No mismatch found! Normalization successful.\n");
    }

    if (word_cnt_asm == word_cnt_c) {
        printf("No mismatch found! Word Counts are equal.\n");
    }
    else
        printf("Mismatch found! Word Counts are NOT equal.\n");

    //Check for mismatch (optimized)
    for (int i = 0; i < len; i++) {
        if (text[i] != text3[i]) {
            printf("Mismatch at index %d:\n C: %c  ASM Optimized: %c\n\n", i, text[i], text3[i]);
            if (match_opt)
                match_opt = false;
        }
    }

    if (match_opt) {
        printf("No mismatch found! Normalization successful (ASM Optimized).\n");
    }

    if (word_cnt_asm_opt == word_cnt_c) {
        printf("No mismatch found! Word Counts are equal (ASM Optimized).\n");
    }
    else
        printf("Mismatch found! Word Counts are NOT equal (ASM Optimized).\n");
    

    return 0;
}