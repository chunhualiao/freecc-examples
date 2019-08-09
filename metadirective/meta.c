int main()
{
#pragma omp metadirective
    for(int i=0; i<10; i++)
        ;
    return 0;
}
