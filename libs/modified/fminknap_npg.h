/* 29/09/2001, revised 04/02/2003
 Alberto Ceselli: MINKNAP modified version (it handles double profits) */

typedef int           boolean;
typedef long          ntype;   /* number of states/items   */
typedef double        itype;   /* item profits and weights */
typedef double        stype;   /* sum of pofit or weight   */
typedef long double   ptype;   /* product type (sufficient precision) */
typedef unsigned long btype;   /* binary representation of solution */

#define PISINGER_EPSILON 1E-9 /*1E-12*/

/* Returns a dual bound instead of the solution value */
#define RETURN_DUAL_BOUND

/* Skips remotion of items with negative price and
   test for the trivial solutions (all variables to 0 and all variables to 1) */
// #define SKIP_INITIAL_TESTS

/*#define TURN_OFF_REDUCTIONS*/

typedef struct _SolutionList {

	int * x;
	double value;
	struct _SolutionList * next;

} SolutionList;

/* Global variables allocation and deallocation */
void openPisinger(int n);
void closePisinger(void);
void setRCLimit(double val);

stype minknap(SolutionList ** xs, int n, itype *p, itype *w, int *x, itype c);
