#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MASTER_PROCESSOR 0

int main(int argc, char *argv[])
{
    MPI_Init(NULL, NULL);
    FILE *inFile, *outFile;
    int processorID, processorSize;

    MPI_Comm_size(MPI_COMM_WORLD, &processorSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &processorID);

    MPI_Status status;
    inFile = fopen("number.txt", "r");
    outFile = fopen("output.txt", "a");
    if (processorID == MASTER_PROCESSOR)
    {
        int *arr, n = 0, a;
        int totalSum = 0, quotaSum = 0, portion = 0;

        // Counting n number from file
        while (fscanf(inFile, "%d", &a) == 1)
            n++;

        fclose(inFile);
        arr = (int *)malloc(n * sizeof(int));

        inFile = fopen("number.txt", "r");
        // Read numbers from file
        for (int i = 0; i < n; i++)
            fscanf(inFile, "%d,", &arr[i]);

        int quota = n / processorSize;
        int remaining_num = (n % processorSize);

        // Add remaining numbers to the master process
        if (processorID == MASTER_PROCESSOR)
            remaining_num += quota;

        portion = remaining_num;

        // distributing data to processors
        for (int i = 1; i < processorSize; i++)
        {
            MPI_Send(&quota, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send((arr + portion), quota, MPI_INT, i, 0, MPI_COMM_WORLD);
            portion += quota;
        }

        // Addition at master processor
        for (int i = 0; i < remaining_num; i++)
            totalSum += arr[i];

        fprintf(outFile, "\nAT PROCESSOR %d SUM IS:%d", processorID, totalSum);

        // Recieving sum from other processors
        for (int i = 1; i < processorSize; i++)
        {
            MPI_Recv(&quotaSum, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            totalSum += quotaSum;
        }
        fprintf(outFile, "\nSUM OF n NUMBERS IS : %d", totalSum);
    }
    else
    {
        int quota_recieved, sum = 0;
        int *temp;

        // Recieved number of quota from master processors
        MPI_Recv(&quota_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        temp = (int *)malloc(quota_recieved * sizeof(int));

        // Recieved quota array from master processors
        MPI_Recv(temp, quota_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // printf("%d", part_recived);
        for (int i = 0; i < quota_recieved; i++)
            sum += temp[i];

        fprintf(outFile, "\nAT PROCESSOR %d SUM IS:%d", processorID, sum);

        // Sending sums to the master processors
        MPI_Send(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    fclose(inFile);
    fclose(outFile);
    MPI_Finalize();
    return 0;
}