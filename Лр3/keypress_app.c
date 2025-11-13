#include <stdio.h>
#include <sys/time.h>

int main()
{
    struct timeval start, end;
    char str[100];

    printf("Введите строку: ");
    fflush(stdout);  
    gettimeofday(&start, NULL);
    fgets(str, sizeof(str), stdin);
    gettimeofday(&end, NULL);

    long delta = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Вы ввели: %s", str);  
    printf("Время между нажатиями: %ld мкс\n", delta);

    // Запись в ядро (для простоты — в файл)
    FILE *f = fopen("/tmp/entropy", "w");
    if (f) {
        fprintf(f, "%ld\n", delta); 
        fclose(f);
 
        printf("Entropy value %ld written to /tmp/entropy\n", delta);
    }
    return 0;
}