int main()
{
    int *A;
#pragma omp allocate(A)
    return 0;
}
