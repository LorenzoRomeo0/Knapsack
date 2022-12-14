/* 29/09/2001, revised 04/02/2003
 Alberto Ceselli: MINKNAP modified version (it handles double profits) */


/* ======================================================================
           MINKNAP.C, David Pisinger   march 1993, revised feb. 1998
   ====================================================================== */

/* This is the C-code corresponding to the paper:
 *
 *   D. Pisinger
 *   A minimal algorithm for the 0-1 knapsack problem
 *   Operations Research, 45, 758-767 (1997).
 *
 * Further details on the project can also be found in
 *
 *   D. Pisinger
 *   Algorithms for Knapsack Problems
 *   Report 95/1, DIKU, University of Copenhagen
 *   Universitetsparken 1
 *   DK-2100 Copenhagen
 *
 * The algorithm may be used for academic, non-commercial purposes 
 * only.
 * -------------------------------------------------------------------
 * The present code is a callable routine which solves a 0-1 Knapsack
 * Problem:
 *
 *           maximize   \sum_{j=1}^{n} p_{j} x_{j}
 *           subject to \sum_{j=1}^{n} w_{j} x_{j} \leq c
 *                      x_{j} \in \{0,1\}, j = 1,\ldots,n
 *
 * The minknap algorithm is called as
 *
 *          z = minknap(n, p, w, x, c)
 * 
 * where p[], w[], x[] are arrays of integers. The optimal objective
 * value is returned in z, and x[] gives the solution vector.
 * If you need a different interface for your algorithm, minknap
 * may easily be adapted to your own datastructures since all tables
 * are copied to the internal representation. 
 *
 * Since the minknap algorithm is based on dynamic programming, you
 * must give an upper limit MAXSTATES on the number of states. The 
 * constant is defined below. Different types should be defined as
 * follows:
 * 
 *    itype     should be sufficiently large to hold a profit or weight
 *    stype     should be sufficient to hold sum of profits/weights
 *    ptype     should hold the product of an stype and itype
 * 
 * The code has been tested on a hp9000/735, and conforms with the
 * ANSI-C standard. 
 * 
 * Errors and questions are refered to:
 *
 *   David Pisinger, associate professor
 *   DIKU, University of Copenhagen,
 *   Universitetsparken 1,
 *   DK-2100 Copenhagen.
 *   e-mail: pisinger@diku.dk
 *   fax: +45 35 32 14 01
 */

/* ======================================================================
                                  definitions
   ====================================================================== */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "fminknap_npg.h"


/* ======================================================================
				   macros
   ====================================================================== */

#define MAXSTATES 400000
#define MAXFLOAT 10e12

#define SYNC            5      /* when to switch to linear scan in bins */
#define SORTSTACK     200      /* depth of stack used in qsort */
#define MINMED        100      /* find exact median in qsort if larger size */

#define TRUE  1
#define FALSE 0

#define LEFT  1
#define RIGHT 2

#define PARTIATE 1
#define SORTALL  2

#define MAXV (8*sizeof(btype)) /* number of bits in a long integer */
#define PMAX MAXFLOAT                 /* profit of worlds most efficient item  */
#define WMAX 0.0                 /* weight of worlds most efficient item  */
#define PMIN 0.0                 /* profit of worlds least efficient item */
#define WMIN MAXFLOAT                 /* weight of worlds least efficient item */



#define DET(a1, a2, b1, b2)    ((a1) * (ptype) (b2) - (a2) * (ptype) (b1))
#define SWAP(a, b)   { register item t; t = *(a); *(a) = *(b); *(b) = t; }
#define DIFF(a,b)              ((int) ((b)-(a)+1))
#define NO(a,p)                ((int) ((p) - (a)->fitem + 1))
#define N(a,p)                 ((int) ((p) - (a)->d.set1))
#define L(x)                   ((long) (x))
#define SZ(a)                  (*(((int *) (a)) - 4) - 1)


/* ======================================================================
				 type declarations
   ====================================================================== */

/* item record */
typedef struct irec {
  itype   p;     /* profit */
  itype   w;     /* weight */
  boolean *x;    /* solution variable */
} item;

typedef struct { /* i-stack */
  item  *f;      /* first item in interval */
  item  *l;      /* last item in interval */
} interval;

/* state in dynamic programming */
typedef struct pv {
  stype psum;    /* profit sum */
  stype wsum;    /* weight sum */
  btype vect;    /* solution vector */
} state;

/* set of states */
typedef struct pset {
  ntype size;    /* set size */
  state *fset;   /* first element in set */
  state *lset;   /* last element in set */
  state *set1;   /* first element in array */
  state *setm;   /* last element in array */
} stateset;

typedef struct { /* all problem information */
  ntype    n;               /* number of items         */
  item     *fitem;          /* first item in problem   */
  item     *litem;          /* last item in problem    */
  item     *ftouch;         /* first item considered for reduction */
  item     *ltouch;         /* last item considered for reduction */
  item     *s;              /* current core is [s,t]   */
  item     *t;              /*                         */
  item     *b;              /* break item              */
  item     *fpart;          /* first item returned by partial sort */
  item     *lpart;          /* last item returned by partial sort */
  stype    wfpart;          /* weight sum up to fpart  */
  item     *fsort;          /* first sorted item       */
  item     *lsort;          /* last sorted item        */
  stype    wfsort;          /* weight sum up to fsort  */
  stype    c;               /* current capacity        */
  stype    cstar;           /* origianl capacity       */
  stype    z;               /* current solution        */
  stype    zstar;           /* optimal solution        */
  stype    zwsum;           /* weight sum of zstar     */
  itype    ps, ws, pt, wt;  /* items for deriving bounds */

  btype    vno;             /* current vector number   */
  item *   vitem[MAXV];     /* current last MAXV items */
  item *   ovitem[MAXV];    /* optimal set of items    */
  btype    ovect;           /* optimal solution vector */

  stype    dantzig;         /* dantzig upper bound     */
  stype    ub;              /* global upper bound      */
  stype    psumb;           /* profit sum up to b      */
  stype    wsumb;           /* weight sum up to b      */
  boolean  firsttime;       /* used for restoring x    */
  boolean  welldef;         /* is x welldefined        */
  stateset  d;              /* set of partial vectors  */
  interval *intv1, *intv2;
  interval *intv1b, *intv2b;

	SolutionList * xs;  /* SOLUTIONS STORAGE */
	int * x;
	double rcLimit;

  /* debug */
  long     iterates;        /* counters used to obtain specific */
  long     simpreduced;     /* information about the solution process */
  long     pireduced;
  long     pitested;
  long     maxstates;
  long     coresize;
  long     bzcore;
} allinfo;


/* ======================================================================
				  errorx
   ====================================================================== */

static void errorx(char *str, ...)
{
  va_list args;

  va_start(args, str);
  vprintf(str, args); printf("\n");
  va_end(args);
  printf("Program is terminated !!!\n\n");
  exit(-1);
}


/* ======================================================================
				  palloc
   ====================================================================== */

static void pfree(void *p)
{
  if (p == NULL) errorx("freeing null");
  free(p);
}


static void *palloc(long size)
{
  char *p;

  if (size == 0) size = 1;
  if (size != (size_t) size) errorx("Alloc too big %ld", size);
  p = (char *) malloc(size);
  if (p == NULL) errorx("no memory size %ld", size);
  return p;
}


/* ======================================================================
				  findvect
   ====================================================================== */

static state *findvect(stype ws, state *f, state *l)
{
  /* find vector i, so that i->wsum <= ws < (i+1)->wsum */
  register state *m;

  /* a set should always have at least one vector */
  if (f > l) errorx("findvect: empty set");
  if (f->wsum >  ws) return NULL;
  if (l->wsum <= ws) return l;

  while (l - f > SYNC) {
    m = f + (l - f) / 2;
    if (m->wsum > ws) { l = m-1; } else { f = m; }
  }
  while (l->wsum > ws) l--;
  return l;
}


/* ======================================================================
				push/pop
   ====================================================================== */

static void push(allinfo *a, int side, item *f, item *l)
{
  interval *pos;
  switch (side) {
    case LEFT : pos = a->intv1; (a->intv1)++; break;
    case RIGHT: pos = a->intv2; (a->intv2)--; break;
  }
  if (a->intv1 == a->intv2) errorx("interval stack full");
  pos->f = f; pos->l = l;
}

static void pop(allinfo *a, int side, item **f, item **l)
{
  interval *pos;
  switch (side) {
    case LEFT : if (a->intv1 == a->intv1b) errorx("pop left");
		(a->intv1)--; pos = a->intv1; break;
    case RIGHT: if (a->intv2 == a->intv2b) errorx("pop right");
		(a->intv2)++; pos = a->intv2; break;
  }
  *f = pos->f; *l = pos->l;
}


/* ======================================================================
				improvesolution
   ====================================================================== */

static void improvesolution(allinfo *a, state *v)
{
#ifdef IMPROVE_STORES
	SolutionList * scan;
	register int _i;
  register item *i;
  register stype psum, wsum;
  register btype j, k;
#endif

  if (v->wsum  > a->c) errorx("wrong improvesoluton");
  if (v->psum <= a->z) errorx("not improved solution");

#ifdef IMPROVE_STORES
  psum = v->psum;
  wsum = v->wsum;

  for (j = 0; j < MAXV; j++) {
    k = v->vect & ((btype) 1 << j);
    i = a->vitem[j]; if (i == NULL) continue;
    if (*(i->x) == 1) {
      if (k) { psum += i->p; wsum += i->w; *(i->x) = 0; }
    } else {
      if (k) { psum -= i->p; wsum -= i->w; *(i->x) = 1; }
    }
  }

  if ( (psum == a->psumb) && (wsum == a->wsumb) ) {

	/* Store incumbent */

	if (a->xs == NULL) {
		a->xs = (SolutionList *) malloc (sizeof(SolutionList) );
		scan = a->xs;
	} else {
		scan = a->xs;
    while (scan->next != NULL) scan = scan->next;
		scan->next = (SolutionList *) malloc (sizeof(SolutionList) );
		scan = scan->next;
	}

	scan->next = NULL;
	scan->value = v->psum;
	scan->x = (int *) malloc (a->n * sizeof(int) );
	for (_i = 0; _i<a->n; _i++) scan->x[_i] = a->x[_i];

	}

	/* restore solution */
  for (j = 0; j < MAXV; j++) {
    k = v->vect & ((btype) 1 << j);
    i = a->vitem[j]; if (i == NULL) continue;
    if (*(i->x) == 1) {
      if (k) { *(i->x) = 0; }
    } else {
      if (k) { *(i->x) = 1; }
    }
  }


	/* STORE THE NEW INCUMBENT */
/*	if (a->xs == NULL) {
		a->xs = (ISolutionList *) malloc (sizeof(ISolutionList) );
		scan = a->xs;
	} else {
		scan = a->xs;
    while (scan->next != NULL) scan = scan->next;
		scan->next = (ISolutionList *) malloc (sizeof(ISolutionList) );
		scan = scan->next;
	}

	scan->next = NULL;
	scan->z = v->psum;
	scan->zwsum = v->wsum;
	scan->ovect = v->vect;
  memcpy(scan->ovitem, a->vitem, sizeof(item *) * MAXV);*/
	/* DONE */

#endif

  a->z      = v->psum;
  a->zwsum  = v->wsum;
  a->ovect  = v->vect;
  memcpy(a->ovitem, a->vitem, sizeof(item *) * MAXV);

}

static void storesolution(allinfo *a, state *v)
{
	SolutionList * scan;
	register int _i;
  register item *i;
  register stype psum, wsum;
  register btype j, k;

  psum = v->psum;
  wsum = v->wsum;

  for (j = 0; j < MAXV; j++) {
    k = v->vect & ((btype) 1 << j);
    i = a->vitem[j]; if (i == NULL) continue;
    if (*(i->x) == 1) {
      if (k) { psum += i->p; wsum += i->w;}
    } else {
      if (k) { psum -= i->p; wsum -= i->w;}
    }
  }

  if ( (psum == a->psumb) && (wsum == a->wsumb) ) {

	/* Store incumbent */

	if (a->xs == NULL) {
		a->xs = (SolutionList *) malloc (sizeof(SolutionList) );
		scan = a->xs;
	} else {
		scan = a->xs;
    while (scan->next != NULL) scan = scan->next;
		scan->next = (SolutionList *) malloc (sizeof(SolutionList) );
		scan = scan->next;
	}

	scan->next = NULL;
	scan->value = v->psum;
	scan->x = (int *) malloc (a->n * sizeof(int) );
	for (_i = 0; _i<a->n; _i++) scan->x[_i] = a->x[_i];
  for (j = 0; j < MAXV; j++) {
    k = v->vect & ((btype) 1 << j);
    i = a->vitem[j]; if (i == NULL) continue;
    if (*(i->x) == 1) {
      if (k) { scan->x[i->x - a->x] = 0; }
    } else {
      if (k) { scan->x[i->x - a->x] = 1; }
    }
  }

	}

}

/* ======================================================================
				definesolution
   ====================================================================== */

static void definesolution(allinfo *a)
{
  register item *f, *l, *i;
  register stype psum, wsum;
  register btype j, k;

  if (a->firsttime) {
    a->zstar = a->z;
    a->firsttime = FALSE;
  }

  psum = a->z;
  wsum = a->zwsum;
  f    = a->fsort - 1;
  l    = a->lsort + 1;

  for (j = 0; j < MAXV; j++) {
    k = a->ovect & ((btype) 1 << j);
    i = a->ovitem[j]; if (i == NULL) continue;
    if (*(i->x) == 1) {
      if (i > f) f = i;
      if (k) { psum += i->p; wsum += i->w; *(i->x) = 0; }
    } else {
      if (i < l) l = i;
      if (k) { psum -= i->p; wsum -= i->w; *(i->x) = 1; }
    }
  }

  a->welldef = (psum == a->psumb) && (wsum == a->wsumb);

  /* prepare for next round */
  if (!a->welldef) {
    a->fsort = f + 1;
    a->lsort = l - 1;
    a->intv1 = a->intv1b;
    a->intv2 = a->intv2b;
    a->c     = wsum;
    /* A gap from optimum (psum) must be ensured */
    a->z     = psum - 1.0; /*& DOUBLE - 1E-3; */
    a->ub    = psum;
    a->maxstates = 0;
  }

}


/* ======================================================================
				median
   ====================================================================== */

static item *median(item *f1, item *l1, ntype s)
{
  /* Find median r of items [f1, f1+s, f1+2s, ... l1], */
  /* and ensure the ordering f1 >= r >= l1.            */
  register ptype mp, mw;
  register item *i, *j;
  register item *f, *l, *k, *m, *q;
  ntype n, d;
  static item r;

  n = (l1 - f1) / s;              /* number of values      */
  f = f1;                         /* calculated first item */
  l = f1 + s * n;                 /* calculated last item  */
  k = l;                          /* saved last item       */
  q = f + s * (n / 2);            /* middle value          */

  for (;;) {
    d = (l - f + s) / s;
    m = f + s * (d / 2);

    if (d > 1) {
      if (DET(f->p, f->w, m->p, m->w) < 0) SWAP(f, m);
      if (d > 2) {
	if (DET(m->p, m->w, l->p, l->w) < 0) {
	  SWAP(m, l);
	  if (DET(f->p, f->w, m->p, m->w) < 0) SWAP(f, m);
	}
      }
    }
    if (d <= 3) { r = *q; break; }

    r.p = mp = m->p; r.w = mw = m->w; i = f; j = l;
    for (;;) {
      do { i += s; } while (DET(i->p, i->w, mp, mw) > 0);
      do { j -= s; } while (DET(j->p, j->w, mp, mw) < 0);
      if (i > j) break;
      SWAP(i, j);
    }

    if ((j <= q) && (q <= i)) break;
    if (i > q) l = j; else f = i;    
  }
  SWAP(k, l1);
  return &r;
}


/* ======================================================================
				partsort
   ====================================================================== */

static void partsort(allinfo *a, item *f, item *l, stype ws, int what)
{
  register ptype mp, mw;
  register item *i, *j, *m;
  register stype wi;
  register int d;

  d = l - f + 1;
  if (d < 1) errorx("negative interval in partsort");
  if (d > MINMED) {
    m = median(f, l, (int) sqrt(d) );
  } else {
    if (d > 1) {
      m = f + d / 2;
      if (DET(f->p, f->w, m->p, m->w) < 0) SWAP(f, m);
      if (d > 2) {
        if (DET(m->p, m->w, l->p, l->w) < 0) {
          SWAP(m, l);
          if (DET(f->p, f->w, m->p, m->w) < 0) SWAP(f, m);
        }
      }
    }
  }

  if (d > 3) {
    mp = m->p; mw = m->w; i = f; j = l; wi = ws;
    for (;;) {
      do { wi += i->w; i++; } while (DET(i->p, i->w, mp, mw) > 0);
      do {             j--; } while (DET(j->p, j->w, mp, mw) < 0);
      if (i > j) break;
      SWAP(i, j);
    }

    if (wi <= a->cstar) {
      if (what ==  SORTALL) partsort(a, f, i-1, ws, what);
      if (what == PARTIATE) push(a, LEFT, f, i-1);
      partsort(a, i, l, wi, what);
    } else {
      if (what ==  SORTALL) partsort(a, i, l, wi, what);
      if (what == PARTIATE) push(a, RIGHT, i,  l);
      partsort(a, f, i-1, ws, what);
    }
  }

  if ((d <= 3) || (what == SORTALL)) {
    a->fpart = f; a->lpart = l; a->wfpart = ws;
  }
}


/* ======================================================================
				  haschance
   ====================================================================== */

static boolean haschance(allinfo *a, item *i, int side)
{
  register state *j, *m;
  register ptype p, w, r;
  stype pp, ww;

  if (a->d.size == 0) return FALSE;
 
  if (side == RIGHT) {
    if (a->d.fset->wsum <= a->c - i->w) return TRUE;
    p = a->ps; w = a->ws; a->pitested++;
    pp = i->p - a->z-PISINGER_EPSILON/*DOUBLE floor(a->z)*//*a->z - 1*/; ww = i->w - a->c;
    r = -DET(pp, ww, p, w);
    for (j = a->d.fset, m = a->d.lset + 1; j != m; j++) {
      if (DET(j->psum, j->wsum, p, w) >= r) return TRUE;
    }
  } else {
    if (a->d.lset->wsum > a->c + i->w) return TRUE;
    p = a->pt; w = a->wt; a->pitested++;
    pp = -i->p - a->z-PISINGER_EPSILON/*DOUBLE floor(a->z)*//*a->z - 1*/; ww = -i->w - a->c;
    r = -DET(pp, ww, p, w);
    for (j = a->d.lset, m = a->d.fset - 1; j != m; j--) {
      if (DET(j->psum, j->wsum, p, w) >= r) return TRUE;
    }
  }
  a->pireduced++;
  return FALSE;
}


/* ======================================================================
				  multiply
   ====================================================================== */

static void multiply(allinfo *a, item *h, int side)
{
  register state *i, *j, *k, *m;
  register itype p, w;
  register btype mask0, mask1;
  state *r1, *rm;

  if (a->d.size == 0) return;
  if (side == RIGHT) { p = h->p; w = h->w; } else { p = -h->p; w = -h->w; }
  if (2*a->d.size + 2 > MAXSTATES) errorx("no space in multiply");

  /* keep track on solution vector */
  a->vno++;
  if (a->vno == MAXV) a->vno = 0;
  mask1 = ((btype) 1 << a->vno);
  mask0 = ~mask1;
  a->vitem[a->vno] = h;

  /* initialize limits */
  r1 = a->d.fset; rm = a->d.lset; k = a->d.set1; m = rm + 1;
  k->psum = -1;
  k->wsum = r1->wsum + fabs(p) + 1;
  m->wsum = rm->wsum + fabs(w) + 1;

  for (i = r1, j = r1; (i != m) || (j != m); ) {
    if (i->wsum <= j->wsum + w) {
      if (i->psum > k->psum) {
				if (i->wsum > k->wsum) k++; /* ALL OPTIMAL ??? */
				k->psum = i->psum; k->wsum = i->wsum;
				k->vect = i->vect & mask0;
	    }
      i++; 
    } else {
      if (j->psum + p > k->psum) {
				if (j->wsum + w > k->wsum) k++; /* ALL OPTIMAL ??? */
				k->psum = j->psum + p; k->wsum = j->wsum + w;
        k->vect = j->vect | mask1;
				/* MULTIPLE PRICING */
				#ifdef PISINGER_MULTIPLE_PRICING
				if (k->psum >= a->rcLimit && k->wsum <= a->c) storesolution(a, k);
				#endif
      }
      j++; 
    }
  }

  a->d.fset = a->d.set1;
  a->d.lset = k;
  a->d.size  = a->d.lset - a->d.fset + 1;
  a->coresize++;
  if (a->d.size > a->maxstates) a->maxstates = a->d.size;
}


/* =========================================================================
				   simpreduce
   ========================================================================= */

static void simpreduce(int side, item **f, item **l, allinfo *a)
{
  register item *i, *j, *k;
  register ptype pb, wb;
  register ptype q, r;
  register int redu;

  if (a->d.size == 0) { *f = *l+1; return; }
  if (*l < *f) return;

  pb = a->b->p; wb = a->b->w;
  q = DET(a->z+PISINGER_EPSILON/* DOUBLE a->z+1*/-a->psumb, a->c - a->wsumb, pb, wb);
  r = -DET(a->z+PISINGER_EPSILON/* DOUBLE a->z+1*/-a->psumb, a->c - a->wsumb, pb, wb);
  i = *f; j = *l;
  redu = 0;
  if (side == LEFT) {
    k = a->fsort - 1;
    while (i <= j) {
      if (DET(j->p, j->w, pb, wb) > r) {
	SWAP(i, j); i++; redu++;       /* not feasible */
      } else {
        SWAP(j, k); j--; k--;  /* feasible */
      }
    }
    *l = a->fsort - 1; *f = k + 1;
  } else {
    k = a->lsort + 1;
    while (i <= j) {
      if (DET(i->p, i->w, pb, wb) < q) {
        SWAP(i, j); j--; redu++;      /* not feasible */
      } else {
        SWAP(i, k); i++; k++;  /* feasible */
      }
    }
    *f = a->lsort + 1; *l = k - 1;
  }
  a->simpreduced += redu;
}


/* ======================================================================
				  reduceset
   ====================================================================== */

static void logicreduce(allinfo *a) {
/*state *r1, *rm, *v;
char lrSet

	  r1 = a->d.fset; rm = a->d.lset;

		for (v = r1; v<=rm; v++) {

			for (k = 0; k<a->logicCutsNumber; k++) {

				count = 0;
				for (i = 0; i<a->cutLength[k]; i++) {

	  	    lrSet = v->ovitem & ((btype) 1 << j);
		    	i = v->ovitem[a->cut[k][i]]; if (i == NULL) continue;

					if ( (*(i->x) == 0 && lrSet) || (*(i->x) == 1 && !lrSet) ) count++;

				}
				if (count > a->cutLength[k]-1) {
					// fathom state
					SWAP(v,rm);	rm--;	v--;
					a->d.size--;
					break;
				}

			}

		}
  */
	
}

static void reduceset(allinfo *a)
{
  register state *i, *m, *k;
  register ptype ps, ws, pt, wt, r;
  stype z, c;
  state *r1, *rm, *v;
  item *f, *l;

  if (a->d.size == 0) return;

	/* logic cuts reduction */
	/*logicreduce(a);*/

  /* initialize limits */
  r1 = a->d.fset; rm = a->d.lset;
  v  = findvect(a->c, r1, rm);
  if (v == NULL) v = r1 - 1; /* all states infeasible */ 
  else {
		if (v->psum > a->z) improvesolution(a, v);
	}

  c = a->c; z = a->z + PISINGER_EPSILON/*DOUBLE a->z + 1*/; k = a->d.setm;

  /* expand core, and choose ps, ws */
  if (a->s < a->fsort) {
    if (a->intv1 == a->intv1b) {
      ps = PMAX; ws = WMAX;
    } else {
      pop(a, LEFT, &f, &l);
      if (f < a->ftouch) a->ftouch = f;
      ps = f->p; ws = f->w; /* default: pick first item */
			#ifndef TURN_OFF_REDUCTIONS
      simpreduce(LEFT, &f, &l, a);
			#endif
      if (f <= l) {
	partsort(a, f, l, 0, SORTALL); a->fsort = f;
	ps = a->s->p; ws = a->s->w;
      }
    }
  } else {
    ps = a->s->p; ws = a->s->w;
  }

  /* expand core, and choose pt, wt */
  if (a->t > a->lsort) {
    if (a->intv2 == a->intv2b) {
      pt = PMIN; wt = WMIN;
    } else {
      pop(a, RIGHT, &f, &l);
      if (l > a->ltouch) a->ltouch = l;
      pt = l->p; wt = l->w; /* default: pick first item */
			#ifndef TURN_OFF_REDUCTIONS
      simpreduce(RIGHT, &f, &l, a);
			#endif
      if (f <= l) {
	partsort(a, f, l, 0, SORTALL); a->lsort = l;
	pt = a->t->p; wt = a->t->w;
      }
    }
  } else {
    pt = a->t->p; wt = a->t->w;
  }

  /* now do the reduction */
	#ifndef TURN_OFF_REDUCTIONS
  r = DET(z, c, ps, ws);
  for (i = rm, m = v; i != m; i--) {
    if (DET(i->psum, i->wsum, ps, ws) > r) { /*DOUBLE >=*/
      k--; *k = *i;
    }
  }

  r = DET(z, c, pt, wt);
  for (i = v, m = r1 - 1; i != m; i--) {
    if (DET(i->psum, i->wsum, pt, wt) > r) { /* DOUBLE >= */
      k--; *k = *i;
    }
  }
	#endif

  a->ps = ps; a->ws = ws;
  a->pt = pt; a->wt = wt;
  a->d.fset = k;
  a->d.lset = a->d.setm - 1; /* reserve one record for multiplication */
  a->d.size = a->d.lset - a->d.fset + 1;
}


/* ======================================================================
				  initfirst
   ====================================================================== */

static void initfirst(allinfo *a, stype ps, stype ws)
{
  register state *k;

  a->d.size  = 1;
/*OPEN*/
	/* OPEN a->d.set1  = (state *) palloc(MAXSTATES * sizeof(state)); */
  a->d.setm  = a->d.set1 + MAXSTATES - 1;
  a->d.fset  = a->d.set1;
  a->d.lset  = a->d.set1;
/**/

  k = a->d.fset;
  k->psum   = ps;
  k->wsum   = ws;
  k->vect   = 0;
}


/* ======================================================================
				  initvect
   ====================================================================== */

static void initvect(allinfo *a)
{
  register btype i;
  for (i = 0; i < MAXV; i++) a->vitem[i] = NULL;
  a->vno = MAXV-1;
}


/* ======================================================================
				  copyproblem
   ====================================================================== */

static void copyproblem(item *f, item *l, itype *p, itype *w, int *x)
{
  register item *i, *m;
  register itype *pp, *ww;
  register int *xx;

  for (i = f, m = l+1, pp = p, ww = w, xx = x; i != m; i++, pp++, ww++, xx++) {
    i->p = *pp; i->w = *ww; i->x = xx; 
  }
}

/* ======================================================================
				findbreak
   ====================================================================== */

static void findbreak(allinfo *a)
{
  register item *i, *m;
  register stype psum, wsum, c, r;

  psum = 0.0; wsum = 0.0; c = a->cstar;

	
  for (i = a->fitem; wsum <= c; i++) {
    *(i->x) = 1; psum += i->p; wsum += i->w; 
  }
  i--; psum -= i->p; wsum -= i->w; /* we went one item too far */

  a->fsort   = a->fpart;
  a->lsort   = a->lpart;
  a->ftouch  = a->fpart;
  a->ltouch  = a->lpart;
  a->b       = i;
  a->psumb   = psum;
  a->wsumb   = wsum;
  a->dantzig = psum + ((c - wsum) * (ptype) i->p) / i->w;
 
  /* find greedy solution */ 
  r = c - wsum;
  for (i = a->b, m = a->litem; i <= m; i++) {
    *(i->x) = 0; if (i->w <= r) { psum += i->p; r -= i->w; }
  }

  a->z       = psum - 1;
  a->zstar   = 0;
  a->c       = a->cstar;
}

/* ======================================================================
		ALLOCATION / DEALLOCATION "Global" variables
 ====================================================================== */
 
  static allinfo a;
  static item *tab;
  static interval *inttab;

  static int * subIndex;	/* Items with positive weights */
  static int subItems;		/* ... their number */

	void openPisinger(int n) {
	static int alloc = 0;

		if (alloc == 0) {

		a.d.set1  = (state *) palloc(MAXSTATES * sizeof(state));
		tab = (item *) palloc(sizeof(item) * n);
		inttab  = (interval *) palloc(sizeof(interval) * SORTSTACK);

		#ifndef SKIP_INITIAL_TESTS
		subIndex = (int *) palloc (sizeof(int)  * n);
		#endif
		alloc = 1;
		}
		
	}

	void closePisinger(void) {
	static int dealloc = 0;

		if (dealloc == 0) {

		pfree(a.d.set1);
		pfree(tab);
		pfree(inttab);
		
		#ifndef SKIP_INITIAL_TESTS
		pfree(subIndex);
		#endif
		dealloc = 1;
		}

	}

	void setRCLimit(double val) {	a.rcLimit = val; }


/* ======================================================================
		PREPROCESSING: removing items with negative price	
 ====================================================================== */

static void preprocess(int n, itype * p, itype *w, int *x, itype c) {
register int i;
	subItems = 0;	
  puts("test1");
  printf("%d \n", subIndex[0]); // segfaults
  puts("test2");
	for (i = 0; i<n; i++)
		if (p[i] > 0) subIndex[subItems++] = i;
		else x[i] = 0;
		
/*	printf("SubItems: %d of %d\n", subItems, n); */

}

/* Same format as copyproblem() */
static void copyproblemPP(item *f, item *l, itype *p, itype *w, int *x)
{
  register item *i;
  register int j;

  i = f;
  for (j = 0; j< subItems; i++, j++) {
  	i->p = p[subIndex[j]];
  	i->w = w[subIndex[j]];
  	i->x = &x[subIndex[j]];
  }

}
	
/* ======================================================================
				minknap
   ====================================================================== */

stype minknap(SolutionList ** xs, int n, itype *p, itype *w, int *x, itype c)
{
/*OPEN
  allinfo a;
  item *tab;
  interval *inttab;
 */

 /* allocate space for internal representation */
/*OPEN
  tab = (item *) palloc(sizeof(item) * n);
 */

	SolutionList * scan;

#ifndef SKIP_INITIAL_TESTS

int i;
double tw = 0.0, tp = 0.0;

/* Preprocessing:
 1) removing items with negative price */

  preprocess(n, p, w, x, c);

/* 2) checking trivial solutions */

	for (i = 0; i<subItems; i++) if (w[subIndex[i]] <= c) break;
	if (i == subItems) {
		/* printf("Trivial: All 0\n"); */
		for (i = 0; i<subItems; i++) x[subIndex[i]] = 0;
		return 0.0;
	}

	for (i = 0; i<subItems; i++) {
		tw += w[subIndex[i]];
		if (tw > c) break;
	}
	
	if (i == subItems) {
		/* printf("Trivial: All 1\n"); */
		for (i = 0; i<subItems; i++) {
			x[subIndex[i]] = 1;
			tp += p[subIndex[i]];
		}
		return tp;
	}

  a.fitem = &tab[0]; a.litem = &tab[subItems-1];
  copyproblemPP(a.fitem, a.litem, p, w, x);
  a.n           = subItems;
  a.cstar       = c;
#else
  a.fitem = &tab[0]; a.litem = &tab[n-1];
  copyproblem(a.fitem, a.litem, p, w, x);
  a.n           = n;
  a.cstar       = c;
#endif

  a.iterates    = 0;
  a.simpreduced = 0;
  a.pireduced   = 0;
  a.pitested    = 0;
  a.maxstates   = 0;
  a.coresize    = 0;

/* OPEN  inttab  = (interval *) palloc(sizeof(interval) * SORTSTACK); */
  a.intv1 = a.intv1b = &inttab[0];
  a.intv2 = a.intv2b = &inttab[SORTSTACK - 1];
  a.fsort = a.litem; a.lsort = a.fitem;
  partsort(&a, a.fitem, a.litem, 0, PARTIATE);
  findbreak(&a);

  a.ub        = a.dantzig;
  a.firsttime = TRUE;

	a.xs = NULL;
	a.x = x;
	a.rcLimit = 0;


  for (;;) {

		while (a.xs != NULL) {
			scan = a.xs;
			a.xs = a.xs->next;
			free(scan);
		}

    a.iterates++;

    a.s = a.b-1;
    a.t = a.b;
    initfirst(&a, a.psumb, a.wsumb);
    initvect(&a);
    reduceset(&a);

    while ((a.d.size > 0) && (a.z < a.ub)) {

      if (a.t <= a.lsort) {
			if (haschance(&a, a.t, RIGHT)) multiply(&a, a.t, RIGHT);
			(a.t)++;
      }
      reduceset(&a);
      if (a.s >= a.fsort) {
			if (haschance(&a, a.s, LEFT)) multiply(&a, a.s, LEFT);
			(a.s)--;
      }
      reduceset(&a);
    }
    /* OPEN pfree(a.d.set1); */

    definesolution(&a);
    if (a.welldef) break;
  }
/*OPEN  pfree(tab);
  OPEN  pfree(inttab); */

	*xs = a.xs;

#ifdef RETURN_DUAL_BOUND
	
	#ifndef SKIP_INITIAL_TESTS
  return (a.zstar + (subItems  - a.coresize ) * PISINGER_EPSILON);/* DOUBLE a.zstar; */
	#else
  return (a.zstar + (n  - a.coresize ) * PISINGER_EPSILON);/* DOUBLE a.zstar; */
	#endif
	
#else

  return (a.zstar);

#endif

}
