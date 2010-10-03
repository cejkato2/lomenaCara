set terminal png small
set pointsize 1
set output "output.png"
plot "vystup1.dat" with points, "vystup2.dat" with linespoints;
