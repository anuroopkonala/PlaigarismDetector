// Reorganized bubble sort implementation by Bob
void sort_array(int val[], int num) {
    int step = 0;
    while (step < num - 1) {
        int index = 0;
        while (index < num - step - 1) {
            if (val[index] > val[index + 1]) {
                int t = val[index];
                val[index] = val[index + 1];
                val[index + 1] = t;
            }
            index++;
        }
        step++;
    }
}