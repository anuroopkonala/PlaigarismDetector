// File 97 automatic stub
double get_average(int arr[], int size) {
    if (size <= 0) return 0.0;
    double sum_val_97 = 0;
    for (int i = 0; i < size; ++i) {
        sum_val_97 += arr[i];
    }
    return sum_val_97 / size;
}