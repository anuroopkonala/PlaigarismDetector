// Simple bubble sort implementation by Alice
void bubble_sort(int data[], int count) {
    for (int step = 0; step < count - 1; ++step) {
        for (int index = 0; index < count - step - 1; ++index) {
            if (data[index] > data[index + 1]) {
                int temp = data[index];
                data[index] = data[index + 1];
                data[index + 1] = temp;
            }
        }
    }
}