/* Simple autoassociative memory to recover names and phone numbers and/or match them *
 * using a Hopfield network.							      *
 */
#include <stdlib.h>
#include <stdio.h>

typedef int           BOOL;
typedef char          CHAR;
typedef int           INT;

#define FALSE         0
#define TRUE          1
#define NOT           !
#define AND           &&
#define OR            ||

#define LO            -1
#define HI            +1

#define BINARY(x)     ((x)==LO ? FALSE : TRUE)
#define BIPOLAR(x)    ((x)==FALSE ? LO : HI)

typedef struct {                     /* A LAYER OF A NET:                     */
        INT           Units;         /* - number of units in this layer       */
        INT*          Output;        /* - output of ith unit                  */
        INT**         Weight;        /* - connection weights to ith unit      */
} LAYER;

typedef struct {                     /* A NET:                                */
        LAYER*        X;             /* - X layer                             */
        LAYER*        Y;             /* - Y layer                             */
} NET;

void InitializeRandoms()
{
  srand(4711);
}

BOOL RandomEqualBOOL()
{
  return rand() % 2;
}      

#define NUM_DATA      3
#define IN_CHARS      5
#define OUT_CHARS     7

#define BITS_PER_CHAR 6
#define FIRST_CHAR    ' '

#define N             (IN_CHARS  * BITS_PER_CHAR)
#define M             (OUT_CHARS * BITS_PER_CHAR)

CHAR                  Names [NUM_DATA][IN_CHARS]  = { "TINA ",
                                                      "ANTJE", 
                                                      "LISA "  };
CHAR                  Names_[NUM_DATA][IN_CHARS]  = { "TINE ",
                                                      "ANNJE", 
                                                      "RITA "  };

CHAR                  Phones[NUM_DATA][OUT_CHARS] = { "6843726",
                                                      "8034673",
                                                      "7260915"  };

INT                   Input [NUM_DATA][N];
INT                   Input_[NUM_DATA][N];
INT                   Output[NUM_DATA][M];

FILE*                 f;

void InitializeApplication(NET* Net)
{
  INT n,i,j,a,a_;

  for (n=0; n<NUM_DATA; n++) {
    for (i=0; i<IN_CHARS; i++) {
      a  = Names [n][i] - FIRST_CHAR;
      a_ = Names_[n][i] - FIRST_CHAR;
      for (j=0; j<BITS_PER_CHAR; j++) {
        Input [n][i*BITS_PER_CHAR+j] = BIPOLAR(a  % 2);
        Input_[n][i*BITS_PER_CHAR+j] = BIPOLAR(a_ % 2);
        a  /= 2;
        a_ /= 2;
      }
    }
    for (i=0; i<OUT_CHARS; i++) {
      a = Phones[n][i] - FIRST_CHAR;
      for (j=0; j<BITS_PER_CHAR; j++) {
        Output[n][i*BITS_PER_CHAR+j] = BIPOLAR(a % 2);
        a /= 2;
      }
    }
  }
  f = fopen("phonebook_processor.txt", "w");
}

void WriteLayer(LAYER* Layer)
{
  INT i,j,a,p;
  for (i=0; i<(Layer->Units / BITS_PER_CHAR); i++) {
    a = 0;
    p = 1;
    for (j=0; j<BITS_PER_CHAR; j++) {
      a += BINARY(Layer->Output[i*BITS_PER_CHAR+j]) * p;
      p *= 2;  
    }
    fprintf(f, "%c", a + FIRST_CHAR);
  }
}

void FinalizeApplication(NET* Net)
{
  fclose(f);
}

void GenerateNetwork(NET* Net)
{
  INT i;

  Net->X = (LAYER*) malloc(sizeof(LAYER));
  Net->Y = (LAYER*) malloc(sizeof(LAYER));
  Net->X->Units  = N;
  Net->X->Output = (INT*)  calloc(N, sizeof(INT));
  Net->X->Weight = (INT**) calloc(N, sizeof(INT*));

  Net->Y->Units  = M;
  Net->Y->Output = (INT*)  calloc(M, sizeof(INT));
  Net->Y->Weight = (INT**) calloc(M, sizeof(INT*));

  for (i=0; i<N; i++) {
    Net->X->Weight[i] = (INT*) calloc(M, sizeof(INT));
  }
  for (i=0; i<M; i++) {
    Net->Y->Weight[i] = (INT*) calloc(N, sizeof(INT));
  }
}

void CalculateWeights(NET* Net)
{
  INT i,j,n;
  INT Weight;

  for (i=0; i<Net->X->Units; i++) {
    for (j=0; j<Net->Y->Units; j++) {
      Weight = 0;
      for (n=0; n<NUM_DATA; n++) {
        Weight += Input[n][i] * Output[n][j];
      }
      Net->X->Weight[i][j] = Weight;
      Net->Y->Weight[j][i] = Weight;
    }
  }
}

void SetInput(LAYER* Layer, INT* Input)
{
  INT i;
  for (i=0; i<Layer->Units; i++) {
    Layer->Output[i] = Input[i];
  }
  WriteLayer(Layer);
}

void SetRandom(LAYER* Layer)
{
  INT i;
  for (i=0; i<Layer->Units; i++) {
    Layer->Output[i] = BIPOLAR(RandomEqualBOOL());
  }
  WriteLayer(Layer);
}

void GetOutput(LAYER* Layer, INT* Output)
{
  INT i;
  for (i=0; i<Layer->Units; i++) {
    Output[i] = Layer->Output[i];
  }
  WriteLayer(Layer);
}

BOOL PropagateLayer(LAYER* From, LAYER* To)
{
  INT  i,j;
  INT  Sum, Out;
  BOOL Stable;

  Stable = TRUE;
  for (i=0; i<To->Units; i++) {
    Sum = 0;
    for (j=0; j<From->Units; j++) {
      Sum += To->Weight[i][j] * From->Output[j];
    }
    if (Sum != 0) {
      if (Sum < 0) Out = LO;
      if (Sum > 0) Out = HI;
      if (Out != To->Output[i]) {
        Stable = FALSE;
        To->Output[i] = Out;
      }
    }
  }
  return Stable;
}

void PropagateNet(LAYER* From, LAYER* To)
{
  BOOL Stable1, Stable2;

  do {
    Stable1 = PropagateLayer(From, To);
    Stable2 = PropagateLayer(To, From);
  } while (NOT (Stable1 AND Stable2));
}

void SimulateNet(LAYER* From, LAYER* To, INT* Pattern, INT* Input, INT* Output)
{
  SetInput(From, Pattern); fprintf(f, " -> ");
  SetRandom(To);           fprintf(f, "  |  ");
  PropagateNet(From, To);
  GetOutput(From, Input);  fprintf(f, " -> ");
  GetOutput(To, Output);   fprintf(f, "\n\n");
}

int main(void)
{
  NET Net;
  INT n;
  INT I[N], O[M];

  InitializeRandoms();
  GenerateNetwork(&Net);
  InitializeApplication(&Net);
  CalculateWeights(&Net);

  fprintf(f, "Hopfield Network for phonebook learning and restoring.\n");
  fprintf(f, "Content-Addressable-Memory behavior. \n");
  fprintf(f,"\n");

  fprintf(f, "Simulate network for distorted number\n") ;
  fprintf(f, "The data is represented as: \n Input | Output \n Name -> Number \n");
  fprintf(f, "\n");
  for (n=0; n<NUM_DATA; n++) {
    SimulateNet(Net.X, Net.Y, Input[n],  I, O);
  }
  fprintf(f, "Simulate network for distorted name\n") ;  
  fprintf(f, "The data is represented as: \n Input | Output \n Number -> Name\n");
  fprintf(f, "\n");
  for (n=0; n<NUM_DATA; n++) {
    SimulateNet(Net.Y, Net.X, Output[n], O, I);
  }
  fprintf(f, "Simulate network for distorted names and numbers\n") ;  
  fprintf(f,"The data is represented as: \n Input | Output \n Name -> Number \n");
  fprintf(f, "\n");
  for (n=0; n<NUM_DATA; n++) {
    SimulateNet(Net.X, Net.Y, Input_[n], I, O);
  }
  FinalizeApplication(&Net);
}

