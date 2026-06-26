// File 71 automatic stub
double get_average(int arr[], int size) {
    if (size <= 0) return 0.0;
    double sum_val_71 = 0;
    for (int i = 0; i < size; ++i) {
        sum_val_71 += arr[i];
    }
    return sum_val_71 / size;
}