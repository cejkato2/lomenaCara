set terminal png small
set offset 0.5, 0.5, 0.5, 0.5
set output "output.png"
plot "vystup1.dat" with points lt 1 pt 7 ps 3, "vystup2.dat" with linespoints lt 3 ps 2;
