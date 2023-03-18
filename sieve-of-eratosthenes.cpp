// TODO: implement Sieve of Eratosthenes


// There is no synchronization between threads, they all work independently on the Prime array to mark the primes.

// The distribution logic runs on the lines 81 and 82. it takes into account the number of threads used by the program,
// and splits the maxNum interval so that every thread has it's own load.

// There is no need for communication between the threads, as they are working on their individual intervals, only thing is,
//Main application must wait for all child threads to print the prime array correctly.

// As the number of cores increase, on small maxNum values, there is no visual difference,
// but more in the millions there is a few milliseconds of improvement, as we go larger into the numbers, the efficiency will increase.

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <string.h>
#include <cmath>

using namespace std;

//The upper limit to counting primes
const int maxNum = 3000000;
const int numOfThreads = 1;

//The array where we keep track of primes
bool prime[maxNum + 1];

struct thread_data
{
    int startOfInterval;
    int endOfInterval;
    int sizeOfPrimes;
    int *seedPrimeNumbers;
};

void SieveOfEratosthenes(int b, int n)
{
    memset(prime, true, sizeof(prime));

    for (int p = b; p * p <= n; p++)
    {
        if (prime[p] == true)
        {
            for (int i = p * p; i <= n; i += p)
                prime[i] = false;
        }
    }
}

void *sieveOfParalelism(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *)threadarg;

    my_data->endOfInterval = min(maxNum, my_data->endOfInterval);

    for (int i = 1; i <= my_data->sizeOfPrimes; i++)
    {
        int counter = ceil((float)((float)my_data->startOfInterval / (float)my_data->seedPrimeNumbers[i])) * my_data->seedPrimeNumbers[i];

        for (int j = counter; j <= my_data->endOfInterval; j += my_data->seedPrimeNumbers[i])
        {
            prime[j] = false;
        }
    }

    pthread_exit(NULL);
}

int main()
{
    int sequentallyCalculatedPrimeNumbers[(int)sqrt(maxNum)];
    int numOfSeeds = 0;

    // Sequentially computed primes up to SQRT(max)
    SieveOfEratosthenes(2, sqrt(maxNum));

    for (int p = 2; p <= sqrt(maxNum); p++)
    {
        if (prime[p])
        {
            numOfSeeds++;
            sequentallyCalculatedPrimeNumbers[numOfSeeds] = p;
        }
    }

    int lowerBound = sqrt(maxNum) + 1;
    int higherBound = lowerBound + ceil((maxNum - (sqrt(maxNum))) / numOfThreads);

    pthread_t threads[numOfThreads];
    struct thread_data td[numOfThreads];
    int rc;

    for (int i = 0; i < numOfThreads; i++)
    {
        td[i].startOfInterval = lowerBound;
        td[i].endOfInterval = higherBound;
        td[i].sizeOfPrimes = numOfSeeds;
        td[i].seedPrimeNumbers = sequentallyCalculatedPrimeNumbers;

        rc = pthread_create(&threads[i], NULL, sieveOfParalelism, (void *)&td[i]);

        lowerBound = higherBound + 1;
        higherBound = lowerBound + (maxNum - ceil(sqrt(maxNum))) / numOfThreads;

        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    //Wait for all threads to finish before printing the prime array
    for (int i = 0; i < numOfThreads; i++)
        pthread_join(threads[i], NULL);

    for (int p = 2; p <= maxNum; p++)
        if (prime[p])
            cout << p << " ";

    pthread_exit(NULL);
}
