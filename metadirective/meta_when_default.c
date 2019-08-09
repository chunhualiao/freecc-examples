int main()
{
    int n = 10;
#pragma omp metadirective when(user={condition(n<5)}:) default(parallel for)
    for(int i=0; i<n; i++)
        ;
    return 0;
}
