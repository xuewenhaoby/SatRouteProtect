cat dropHelloNormal |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3+30}' >gnuplotDelayv1.log
cat dropHelloAttack |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3+30}' >gnuplotDelayv2.log
cat dropHelloProtect |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3+30}' >gnuplotDelayv3.log

gnuplot -persist <<EOF
set multiplot layout 3,1
set title "丢弃Hello包攻击状态下时延结果对比图" 
set xlabel'仿真时间(s)'
set ylabel'到特定网段时延(ms)'
set xrange [0:100]
set yrange [0:150]
set key center at 83,8

plot 'gnuplotDelayv1.log' using 1:2 title '未攻击状态时延' with points ls 7 pt 13 pointsize 1.5
unset title
plot 'gnuplotDelayv2.log' using 1:2 title '攻击状态时延' with points ls 6 pt 6 pointsize 1.5
plot 'gnuplotDelayv3.log' using 1:2 title '加入认证机制时延' with points ls 9 pt 8 pointsize 1.5
unset multiplot
set output


