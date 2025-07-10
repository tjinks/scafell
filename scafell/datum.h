//
//  datum.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef datum_h
#define datum_h

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DT_NONE = 0
    ,DT_CHAR
    ,DT_INT
    ,DT_BOOL
    ,DT_PTR
} scf_datum_type;

typedef struct scf_datum scf_datum;


typedef struct scf_datum {
    scf_datum_type type;
    union {
        char c_value;
        int64_t i_value;
        uint64_t u_value;
        void *p_value;
        bool b_value;
    };
} scf_datum;

scf_datum dt_none(void);
scf_datum dt_int(int64_t i);
scf_datum dt_ptr(void *p);
scf_datum dt_true(void);
scf_datum dt_false(void);

bool dt_int_compare(scf_datum d1, scf_datum d2);


extern const int SCF_DATUM_SIZE;


#endif /* datum_h */
