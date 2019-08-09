int main()
{
#pragma omp metadirective when
    for(int i=0; i<10; i++)
        ;
    return 0;
}
