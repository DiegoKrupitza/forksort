# Forksort
Forksort is a program written in the programming language c that takes a given number of lines and returns them in sorted order.
The forksort algorithm is based on [mergesort](https://en.wikipedia.org/wiki/Merge_sort)

After receiving the number of lines through stdin. The lines get split into two parts (left and right).
Each part gets recursivly pasted into a new instance of forksort. When there is only one line left the recursiv base case is reached and the results are put together.
