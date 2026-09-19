//
//  ucdb.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "ucdb.h"
#include "unicode_data.h"
#include "str.h"

static const int UC_CHAR_COUNT = sizeof(uc_database) / sizeof(uc_database[0]);

static const int LAST_ASCII = 127;

static const scf_char_info INVALID_UC_CHAR_DATA = {-1, UC_NONE, -1, -1, -1, -1};

static int binary_search(scf_codepoint cpToFind) {
    int start = LAST_ASCII + 1, end = UC_CHAR_COUNT;
    
    for (;;) {
        int nextTry = (start + end) / 2;
        int currentCp = uc_database[nextTry].codepoint;
        if (currentCp == cpToFind) {
            return nextTry;
        } else if (currentCp < cpToFind) {
            start = nextTry + 1;
        } else {
            end = nextTry;
        }
        
        if (start == end) {
            return -1;
        }
    }
}

scf_char_info scf_get_char_info(scf_codepoint cpToFind) {
    int index;
    if (cpToFind <= LAST_ASCII) {
        index = cpToFind;
    } else {
        index = binary_search(cpToFind);
    }
    
    if (index >= 0) {
        return uc_database[index];
    } else {
        return INVALID_UC_CHAR_DATA;
    }
}

scf_codepoint scf_last_uc_codepoint(void) {
    return uc_database[UC_CHAR_COUNT - 1].codepoint;
}


