#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <gmp.h>

static int const SIZE = 512;
static unsigned long int const PUBLIC_KEY = 789528681;
static unsigned long int const MESSAGE = 123456;

int main()
{

	mpz_t p, q, pMinusOne, qMinusOne, n, phiN, publicKey, secretKey,
	originalMessage, secretMessage, decryptedMessage;

	int i = 1;
	int isPrime;
	srand ( time(NULL) );

	mpz_init(p);
	mpz_init(q);
	mpz_init(pMinusOne);
	mpz_init(qMinusOne);
	mpz_init(n);
	mpz_init(phiN);
	mpz_init(publicKey);
	mpz_init(secretKey);
	mpz_init(originalMessage);
	mpz_init(secretMessage);
	mpz_init(decryptedMessage);

	mpz_set_ui(publicKey, PUBLIC_KEY);


	/**************************************************************************************
	 * ******************************* Generate p,q primes ********************************
	 **************************************************************************************/

	char pString[SIZE+1];
	char qString[SIZE+1];

	pString[0] = '1';
	qString[0] = '1';

	do
	{
		for(i;i<SIZE;i++)
		{
			pString[i] = (int)(rand()%2) + 48;
		}
		pString[SIZE] = '\0';
		mpz_set_str(p,pString,2);
		mpz_nextprime(p,p);
		mpz_get_str(pString,10,p);
		printf("Random Prime 'p' = %s\n",pString);
		isPrime = millerRabin(p);
	} while (!isPrime);


	i=1;
	do
	{
		for(i;i<SIZE;i++)
		{
			qString[i] = (int)(rand()%2) + 48;
		}
		qString[SIZE] = '\0';
		mpz_set_str(q,qString,2);
		mpz_nextprime(q,q);
		mpz_get_str(qString,10,q);
		printf("Random Prime 'q' = %s\n",qString);
		isPrime = millerRabin(p);
	} while (!isPrime);


	/**************************************************************************************
	 ******************************** Calculate n, phiN ***********************************
	 **************************************************************************************/
	mpz_mul(n, p, q);
	mpz_sub_ui(pMinusOne, p, 1);
	mpz_sub_ui(qMinusOne, q, 1);
	mpz_mul(phiN, pMinusOne, qMinusOne);

	gmp_printf ("n is %Zd\n", n);
	gmp_printf ("phin is %Zd\n", phiN);


	/**************************************************************************************
	 *********************** Calculate secret key with euclidean **************************
	 **************************************************************************************/
	mpz_t divident, divisor, quotient, yNull, yOne, tempY, tempDivident, minusOne;

	mpz_init(divident);
	mpz_init(divisor);
	mpz_init(quotient);
	mpz_init(yNull);
	mpz_init(yOne);
	mpz_init(tempY);
	mpz_init(tempDivident);
	mpz_init(minusOne);

	mpz_set(divident, phiN);
	mpz_set(divisor, publicKey);
	mpz_set_ui(quotient, 0);
	mpz_set_ui(yNull, 0);
	mpz_set_ui(yOne, 1);

	int counter;
	for (counter = 1; mpz_cmp_ui(divisor, 0) != 0; counter++)
	{
		if (counter != 1)
		{
			mpz_set(tempY, yOne);
			mpz_mul(yOne, yOne, quotient);
			mpz_add(yOne, yOne, yNull);
			mpz_set(yNull, tempY);
		}
		mpz_div(quotient, divident, divisor);
		mpz_set(tempDivident, divisor);
		mpz_mod(divisor, divident, divisor);
		mpz_set(divident, tempDivident);
	}

	mpz_set_si(minusOne, -1);
	mpz_pow_ui(secretKey, minusOne, counter);
	mpz_mul(secretKey, secretKey, yOne);

	if (mpz_sgn(secretKey) < 0)
	{
		mpz_add(secretKey, secretKey, phiN);
	}

	gmp_printf ("secretKey is %Zd\n", secretKey);


	/**************************************************************************************
	******************** Create secret message, original message **************************
	**************************************************************************************/

	mpz_set_ui(originalMessage, MESSAGE);
	gmp_printf ("message is %Zd\n", originalMessage);

	mpz_powm(secretMessage, originalMessage, publicKey, n);
	gmp_printf ("secretMessage is %Zd\n", secretMessage);

	mpz_powm(decryptedMessage, secretMessage, secretKey, n);
	gmp_printf ("decryptedMessage is %Zd\n", decryptedMessage);

	return 0;
}


int millerRabin(mpz_t n)
{
    int i, k;
    mpz_t a, b, d, n_minus_one;

    // a = 7
    mpz_init(a);
    mpz_set_str(a, "7", 10);

    mpz_init(n_minus_one);
    mpz_sub_ui(n_minus_one, n, 1);

    k = 0;
    mpz_init_set(d, n_minus_one);

    //amig d páros
    while (mpz_even_p(d))
    {
    	//d-t eggyel jobbra
        mpz_fdiv_q_2exp(d, d, 1);
        k++;
    }

    mpz_init(b);
    //gyorshatvanyozas
    //mpz_powm(result, base, exponent, modulus)
    mpz_powm(b, a, d, n);
    //compare to unsigned int
    if (mpz_cmp_ui(b, 1) == 0)
    {
        return 1;
    }
    for(i=0; i < k-1; i++)
    {
    	//compare
        if (mpz_cmp(b, n_minus_one) == 0)
        {
        	return 1;
        }
        //gyorshatvanyozas (unsigned int)
        //mpz_powm(result, base, exponent, modulus)
        mpz_powm_ui (b, b, 2, n);
    }
    //compare
    if (mpz_cmp(b, n_minus_one) == 0)
    {
    	return 1;
    }
    return 0;
}

void squareAndMultiply (mpz_t result, mpz_t number, mpz_t square, mpz_t modulus)
{
	char binarySquare[SIZE+1];
	mpz_get_str(binarySquare, 2, square); //hatvány bináris formában

	mpz_t tempNumber;
	mpz_init(tempNumber); //temporary változó a hatványtól (0 v 1) függõen kap értéket

	mpz_set_ui(result,1); //a result-t 1re állítjuk

	int i;
	for (i = 0;binarySquare[i] != '\0';i++) //for ciklus a bináris hatványon ('\0'-ig)
	{
		if (binarySquare[i] == '0') //ha az adott érték 0
		{
			mpz_set_ui(tempNumber, 1); //a temporary változó értéke 1
		}
		else // egyébként (vagyis ha az érték 1)
		{
			mpz_set(tempNumber, number); //a temporary változó értéke a paraméterként kapott szám
		}

		mpz_pow_ui(result, result, 2); // a pillanatnyi result-t a 2-ik hatványra emeljük
		mpz_mul(result, result, tempNumber); //majd megszorozzuk a temporary változóval
		mpz_mod(result, result, modulus); // és végül maradékos osztás
	}
	gmp_printf ("square and multiply result is %Zd\n", result);
}
