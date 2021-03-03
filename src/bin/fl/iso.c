//------------------------------------------------------------------------------
// Copyright 2020 Markus Aronsson
// SPDX-License-Identifier: Apache-2.0
//------------------------------------------------------------------------------

/******************************************************************************/
/*      	                                      							  */
/*     Original author: Markus Aronsson, 2020                                 */
/*                                  									      */
/******************************************************************************/
#include "strings.h"
#include "buf.h"
#include "graph.h"
#include "iso.h"

/******************************************************************************/
/*                               GLOBAL VARIABLES                             */
/******************************************************************************/
// ...

// Global variables referenced -------------------------------------------------
extern str_mgr strings;

/******************************************************************************/
/*                              PRIVATE VARIABLES                             */
/******************************************************************************/
static ustr_mgr	lstrings;
static rec_mgr key_lst_mgr;
static rec_mgr_ptr key_lst_mgr_ptr;
static rec_mgr key_mgr;
static rec_mgr_ptr key_mgr_ptr;
static rec_mgr bkt_mgr;
static rec_mgr_ptr bkt_mgr_ptr;
static rec_mgr vec_mgr;
static rec_mgr rng_mgr;
static rec_mgr mat_mgr;
// ?
static hash_record tbl_in;
static hash_record_ptr tbl_in_ptr;
static hash_record tbl_out;
static hash_record_ptr tbl_out_ptr;
// ?
static typeExp_ptr adj_tp;

// Forward definitions local functions -----------------------------------------
// Matrix mgm.
static void allocate_matrix(mat_ptr m, unint R, unint C);
static void free_matrix(mat_ptr m);
static void read_matrix(mat_ptr m, g_ptr redex);
static void drop_matrix_row(mat_ptr m);
static void drop_matrix_col(mat_ptr m);
// ?
static vec_ptr split_vector(string name);
// Adj.
static void new_adj_mem();
static void rem_adj_mem();
static void record_vector(hash_record_ptr tbl_ptr, key_ptr *tail, unint i, const vec_ptr v);
static bool mk_adj_table(key_lst_ptr *k, unint *c, g_ptr p);
static bool mk_adj_matrix(mat_ptr m, g_ptr p);
static bool mk_adj_needle(g_ptr p, unint size);
static bool mk_adj_haystack(g_ptr p, unint size);
// Isomatch.
static inline bool test_adjacent_needle(int i, int j);
static inline bool test_adjacent_haystack(int i, int j);
static bool test_isomorphism_formatting(mat_ptr iso);
static bool test_isomorphism_match(mat_ptr iso, mat_ptr p, mat_ptr g);

/******************************************************************************/
/*                                LOCAL FUNCTIONS                             */
/******************************************************************************/

// Mem. mgm. -------------------------------------------------------------------

static void
allocate_matrix(mat_ptr m, unint R, unint C)
{
    m->rows   = R;
    m->cols   = C;
    m->mat    = Malloc(R*sizeof(bool*));
    m->mat[0] = Calloc(R*C*sizeof(bool));
    for(unint i=1; i<R; i++) {
		m->mat[i] = m->mat[0]+i*C;
    }
}

static void
free_matrix(mat_ptr m)
{
    Free((void *)m->mat[0]);
    Free((void *)m->mat);
    free_rec(&mat_mgr, m);
}

static void
read_matrix(mat_ptr m, g_ptr redex)
{
    g_ptr lr, lc, row, col;
    int i = 0;
    FOR_CONS(redex, lr, row) {
        int j = 0;
        FOR_CONS(row, lc, col) {
            m->mat[i][j] = GET_BOOL(col);
            j++;
        }
        i++;
    }
}

static void
drop_matrix_row(mat_ptr m)
{
    // Ruins memory, but it's a stupid hack anyway (can't move mat[0]).
    unint R = m->rows;
    m->rows = R-1;
    for(unint i=0; i<R; i++) {
        m->mat[i] = m->mat[i+1];
    }

}

static void
drop_matrix_col(mat_ptr m)
{
    // Ruins memory, but it's a stupid hack anyway (can't move mat[0]).
    unint R = m->rows;
    m->cols = m->cols-1;
    for(unint i=0; i<R; i++) {
        m->mat[i] = m->mat[i]+1;
    }
}

// ? ---------------------------------------------------------------------------

static vec_ptr
split_vector(string name)
{
    vec_ptr vp = Split_vector_name(&lstrings, &vec_mgr, &rng_mgr, name);
    for(vec_ptr p = vp; p != NULL; p = p->next) {
        if(p->type == TXT) {
            p->u.name = wastrsave(&strings, p->u.name);
        }
    }
    return vp;
}

// Adj. matrix -----------------------------------------------------------------

static void
new_adj_mem()
{
    key_lst_mgr_ptr = &key_lst_mgr;
    key_mgr_ptr = &key_mgr;
    bkt_mgr_ptr = &bkt_mgr;
    tbl_in_ptr = &tbl_in;
    tbl_out_ptr = &tbl_out;
    new_mgr(key_lst_mgr_ptr, sizeof(key_lst));
    new_mgr(key_mgr_ptr, sizeof(key_rec));
    new_mgr(bkt_mgr_ptr, sizeof(bkt_rec));
    create_hash(tbl_in_ptr, 100, str_hash, str_equ);
    create_hash(tbl_out_ptr, 100, str_hash, str_equ);
}

static void
rem_adj_mem()
{
    free_mgr(key_lst_mgr_ptr);
    free_mgr(key_mgr_ptr);
    free_mgr(bkt_mgr_ptr);
    dispose_hash(tbl_in_ptr, NULLFCN);
    dispose_hash(tbl_out_ptr, NULLFCN);
    key_lst_mgr_ptr = NULL;
    key_mgr_ptr = NULL;
    bkt_mgr_ptr = NULL;
    tbl_in_ptr = NULL;
    tbl_out_ptr = NULL;
}

static void
record_vector(hash_record_ptr tbl_ptr, key_ptr *tail, unint i, const vec_ptr v)
{
    string key = Get_vector_signature(&lstrings, v);
    // Record in tail.
    key_ptr n = (key_ptr) new_rec(&key_mgr);
    n->lbl  = key;
    n->vec  = v;
    n->next = NULL;
    (*tail) = n;
    // Record in table.
    bkt_ptr bkt = find_hash(tbl_ptr, key);
    if(bkt == NULL) {
        bkt_ptr b = (bkt_ptr) new_rec(&bkt_mgr);
        b->lbl  = i;
        b->vec  = v;
        b->next = NULL;
        insert_hash(tbl_ptr, key, b);
    } else {
        while(bkt->next != NULL) { bkt = bkt->next; }
        bkt_ptr b = (bkt_ptr) new_rec(&bkt_mgr);
        b->lbl  = i;
        b->vec  = v;
        b->next = NULL;
        bkt->next = b;
    }
}

static bool
mk_adj_table(key_lst_ptr *keys, unint *count, g_ptr p)
{
    g_ptr attrs, fa_inps, fa_outs, inter, cont, children, fns;
    string name;
    bool leaf;
    if(!is_PINST(p, &name, &attrs, &leaf, &fa_inps, &fa_outs, &inter, &cont)) {
	    Fail_pr("Adj. mat. expects a pexlif.");
        return FALSE;
    }    
    if(is_P_LEAF(cont, &fns)) {
        Fail_pr("Adj. mat. expects a hierarchical pexlif.");
        return FALSE;
    }
    if(is_P_HIER(cont, &children)) {
        vec_ptr vec;
        key_ptr key = NULL, *key_tail = &key;
        // Record vector names for parent's formals.
        FOREACH_FORMAL(vec, fa_inps) {
            record_vector(tbl_in_ptr, key_tail, 0, vec);
            key_tail = &(*key_tail)->next;
        }
        FOREACH_FORMAL(vec, fa_outs) {
            record_vector(tbl_out_ptr, key_tail, 0, vec);
            key_tail = &(*key_tail)->next;
        }
        key_lst_ptr key_lst = (key_lst_ptr) new_rec(&key_lst_mgr), *key_lst_tail = &key_lst;
        key_lst->key  = key;
        key_lst->next = NULL;
        *keys         = key_lst;
        *count        = 1;
        key_lst_tail  = &key_lst->next;
        // Record vector names for each child's actuals.
        g_ptr child, tmp;
        FOR_CONS(children, tmp, child) {
            if(!is_PINST(child, &name, &attrs, &leaf, &fa_inps, &fa_outs, &inter, &cont)) {
                Fail_pr("Adj. mat. expects all children to be pexlifs.");
                return FALSE;
            }
            key_ptr key = NULL, *key_tail = &key;
            FOREACH_ACTUAL(vec, fa_inps) {
                record_vector(tbl_in_ptr, key_tail, *count, vec);
                key_tail = &(*key_tail)->next;
            }
            FOREACH_ACTUAL(vec, fa_outs) {
                record_vector(tbl_out_ptr, key_tail, *count, vec);
                key_tail = &(*key_tail)->next;
            }
            key_lst_ptr key_lst = (key_lst_ptr) new_rec(&key_lst_mgr);
            key_lst->key  = key;
            key_lst->next = NULL;
            *key_lst_tail = key_lst;
            key_lst_tail  = &key_lst->next;
            *count        = *count + 1;
        }
        return TRUE;
    }
    Fail_pr("What are you then?!");
    return FALSE;
}

static bool
mk_adj_matrix(mat_ptr m, g_ptr p)
{
    new_adj_mem();
    // Mark adjacencies, if any.
    key_lst_ptr keys = NULL;
    unint length;
    if(mk_adj_table(&keys, &length, p)) {
        // All nodes accounted for.
        ASSERT(m->cols == length);
        // Square matrix.
        ASSERT(m->cols == m->rows);
        //
        for(unint i = 0; keys != NULL; i++, keys = keys->next) {
            for(key_ptr key = keys->key; key != NULL; key = key->next) {
                string name = key->lbl;
                vec_ptr vec = key->vec;
                // Search inputs.
                bkt_ptr bkt = (bkt_ptr) find_hash(tbl_in_ptr, name);
                while(bkt != NULL) {
                    if(bkt->lbl != i && Check_vector_overlap(vec, bkt->vec)) {
                        m->mat[i][bkt->lbl] = TRUE;
                    }
                    bkt = bkt->next;
                }
                // Search outputs.
                bkt = (bkt_ptr) find_hash(tbl_out_ptr, name);
                while(bkt != NULL) {
                    if(bkt->lbl != i && Check_vector_overlap(vec, bkt->vec)) {
                        m->mat[i][bkt->lbl] = TRUE;
                    }
                    bkt = bkt->next;
                }
            }
        }
    }
    // Done.
    rem_adj_mem();
    return TRUE;
}

static bool
mk_adj_needle(g_ptr p, unint size)
{
    P = (mat_ptr) new_rec(&mat_mgr);
    allocate_matrix(P, size, size);
    return mk_adj_matrix(P, p);
}

static bool
mk_adj_haystack(g_ptr p, unint size)
{
    G = (mat_ptr) new_rec(&mat_mgr);
    allocate_matrix(G, size, size);
    return mk_adj_matrix(G, p);
}

// Solution check --------------------------------------------------------------

static inline bool
test_adjacent_needle(int i, int j)
{
    ASSERT((unint) i < P->rows);
    ASSERT((unint) j < P->cols);
    return P->mat[i][j];
}

static inline bool
test_adjacent_haystack(int i, int j)
{
    ASSERT((unint) i < G->rows);
    ASSERT((unint) j < G->cols);
    return G->mat[i][j];
}

static bool
test_isomorphism_formatting(mat_ptr iso)
{
    bool row;
    // ToDo: maybe 'used' is small enough for the stack?
    bool *used = Calloc(iso->cols*sizeof(bool));
    // Only one 'TRUE' allowed per row and column.
    for(unint i=0; i<iso->rows; i++) {
		row = FALSE;
		for(unint j=0; j<iso->cols; j++) {
            if(iso->mat[i][j]) {
				if(row || used[j]) {
					Free((void *)used);
					return FALSE;
                }
                row = TRUE;
                used[j] = TRUE;
            }
        }
    }
    Free((void *)used);
    return TRUE;
}

static bool
test_isomorphism_match(mat_ptr iso, mat_ptr p, mat_ptr g)
{
    // P : AxA, G : BxB, ISO : AxB
    ASSERT(p->rows   == p->cols);
    ASSERT(g->rows   == g->cols);
    ASSERT(iso->rows == p->rows);
    ASSERT(iso->cols == g->cols);
    // Short-hands.
    unint GC = g->cols, IR = iso->rows, IC = iso->cols;
    bool *P = p->mat[0], *G = g->mat[0], *I = iso->mat[0];
    // P == ISO x (ISO x G)^T
	bool t1,t2;
	for(unint i=0; i<IR; i++) {
		for(unint j=0; j<IR; j++) {
			t1 = FALSE;
            for(unint k=0; k<IC; k++) {
				t2 = FALSE;
                for(unint l=0; l<IC; l++) {
					if(I[l + (IC * j)] && G[k + (GC * l)]) {
						t2 = TRUE;
                        break;
                    }
                }
                if(I[k + (IC * i)] && t2) {
					t1 = TRUE;
                    break;
                }
            }
            if(P[i * IR + j] != t1) {
				return FALSE;
            }
        }
    }
    return TRUE;
}

/******************************************************************************/
/*                               PUBLIC FUNCTIONS                             */
/******************************************************************************/

static void
Make_needle(g_ptr redex)
{
    g_ptr l = GET_APPLY_LEFT(redex);
    g_ptr r = GET_APPLY_RIGHT(redex);
    //
    g_ptr g_pex, g_size;
    EXTRACT_2_ARGS(redex, g_pex, g_size);
    if(mk_adj_needle(g_pex, (unint) GET_INT(g_size))) {
        MAKE_REDEX_BOOL(redex, B_One());
    } else {
        MAKE_REDEX_BOOL(redex, B_Zero());
    }
    //
    DEC_REF_CNT(l);
    DEC_REF_CNT(r);
}

static void
Make_haystack(g_ptr redex)
{
    g_ptr l = GET_APPLY_LEFT(redex);
    g_ptr r = GET_APPLY_RIGHT(redex);
    //
    g_ptr g_pex, g_size;
    EXTRACT_2_ARGS(redex, g_pex, g_size);
    if(mk_adj_haystack(g_pex, (unint) GET_INT(g_size))) {
        MAKE_REDEX_BOOL(redex, B_One());
    } else {
        MAKE_REDEX_BOOL(redex, B_Zero());
    }
    //
    DEC_REF_CNT(l);
    DEC_REF_CNT(r);
}

static void
Is_adjacent_needle(g_ptr redex)
{
    g_ptr l = GET_APPLY_LEFT(redex);
    g_ptr r = GET_APPLY_RIGHT(redex);
    //
    g_ptr g_i, g_j;
    EXTRACT_2_ARGS(redex, g_i, g_j);
    if(test_adjacent_needle(GET_INT(g_i), GET_INT(g_j))) {
        MAKE_REDEX_BOOL(redex, B_One());
    } else {
        MAKE_REDEX_BOOL(redex, B_Zero());
    }
    //
    DEC_REF_CNT(l);
    DEC_REF_CNT(r);
}

static void
Is_adjacent_haystack(g_ptr redex)
{
    g_ptr l = GET_APPLY_LEFT(redex);
    g_ptr r = GET_APPLY_RIGHT(redex);
    //
    g_ptr g_i, g_j;
    EXTRACT_2_ARGS(redex, g_i, g_j);
    if(test_adjacent_haystack(GET_INT(g_i), GET_INT(g_j))) {
        MAKE_REDEX_BOOL(redex, B_One());
    } else {
        MAKE_REDEX_BOOL(redex, B_Zero());
    }
    //
    DEC_REF_CNT(l);
    DEC_REF_CNT(r);    
}

static void
Is_valid_isomorphism(g_ptr redex)
{
    g_ptr l = GET_APPLY_LEFT(redex);
    g_ptr r = GET_APPLY_RIGHT(redex);
    //
    g_ptr g_iso, g_rows, g_cols;
    EXTRACT_3_ARGS(redex, g_iso, g_rows, g_cols);
    mat_ptr iso = (mat_ptr) new_rec(&mat_mgr);
    allocate_matrix(iso, GET_INT(g_rows), GET_INT(g_cols));
    read_matrix(iso, g_iso);
    if( test_isomorphism_formatting(iso) &&
        test_isomorphism_match(iso, P, G))
    {
        MAKE_REDEX_BOOL(redex, B_One());
    } else {
        MAKE_REDEX_BOOL(redex, B_Zero());
    }
    free_matrix(iso);
    free_rec(&mat_mgr, (pointer) iso);
    //
    DEC_REF_CNT(l);
    DEC_REF_CNT(r);
}

static void
Trim_needle()
{
    drop_matrix_row(P);
    drop_matrix_col(P);
}

static void
Trim_haystack()
{
    drop_matrix_row(G);
    drop_matrix_col(G);
}

//------------------------------------------------------------------------------

void
Iso_Init()
{
    Add_ExtAPI_Object(
        "adj"
      , NULL //mark_adj_fn
      , NULL //sweep_adj_fn
      , NULL //save_adj_fn
      , NULL //load_adj_fn
      , NULL //adj2string_fn
      , NULL //adj_eq_fn
      , NULL
      , NULL
      , NULL //adj2sha256_fn
    );
    adj_tp = Get_Type("adj", NULL, TP_INSERT_FULL_TYPE);

    new_ustrmgr(&lstrings);
    new_mgr(&vec_mgr, sizeof(vec_rec));
    new_mgr(&rng_mgr, sizeof(range_rec));
    new_mgr(&mat_mgr, sizeof(mat_rec));
}

void
Iso_Install_Functions()
{
    typeExp_ptr	pexlif_tp = Get_Type("pexlif", NULL, TP_INSERT_PLACE_HOLDER);

    Add_ExtAPI_Function(
          "pex2adj"
        , "11"
        , TRUE
        , GLmake_arrow(
              pexlif_tp
            , GLmake_arrow(
                GLmake_int()
              , adj_tp))
        , Make_needle
    );

    Add_ExtAPI_Function(
          "trim_needle"
        , ""
        , TRUE
        , GLmake_void()
        , Trim_needle
    );

    Add_ExtAPI_Function(
          "trim_haystack"
        , ""
        , TRUE
        , GLmake_void()
        , Trim_haystack
    );

    Add_ExtAPI_Function(
          "adj_needle"
        , "11"
        , FALSE
        , GLmake_arrow(
              GLmake_int()
            , GLmake_arrow(
                GLmake_int()
              , GLmake_bool()))
        , Is_adjacent_needle
    );
    
    Add_ExtAPI_Function(
          "adj_haystack"
        , "11"
        , FALSE
        , GLmake_arrow(
              GLmake_int()
            , GLmake_arrow(
                GLmake_int()
              , GLmake_bool()))
        , Is_adjacent_haystack
    );

    Add_ExtAPI_Function(
          "is_valid_iso"
        , "111"
        , FALSE
        , GLmake_arrow(
              GLmake_list(GLmake_list(GLmake_bool()))
            , GLmake_arrow(
                GLmake_int()
              , GLmake_arrow(
                  GLmake_int()
                , GLmake_bool())))
        , Is_valid_isomorphism
    );
}

/******************************************************************************/
/*                               DUMMY FUNCTIONS                              */
/******************************************************************************/

void
_DuMMy_iso()
{
    free_matrix(NULL);
    mk_adj_needle(NULL,0);
    mk_adj_haystack(NULL,0);
    test_isomorphism_formatting(NULL);
    test_isomorphism_match(NULL,NULL,NULL);
    M = M;
}

/******************************************************************************/
