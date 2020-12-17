cat modifyLsuNormal |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3}' >gnuplotDelayv1.log
cat modifyLsuAttack |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3}' >gnuplotDelayv2.log
cat modifyLsuNormal |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3}' >gnuplotDelayv3.log

gnuplot -persist <<EOF
set multiplot layout 3,1
set title "篡改LSU包攻击状态下时延结果图" 
set xlabel'仿真时间(ms)'
set ylabel'到特定网段时延(ms)'
set xrange [0:130]
set yrange [0:100]

set key center at 110,8
plot 'gnuplotDelayv1.log' using 1:2 title '未攻击状态时延' with points ls 7 pt 13 pointsize 1.5
unset title
plot 'gnuplotDelayv2.log' using 1:2 title '攻击状态时延' with points ls 6 pt 6 pointsize 1.5
plot 'gnuplotDelayv3.log' using 1:2 title '加入认证机制时延' with points ls 9 pt 8 pointsize 1.5

unset multiplot
set output

EOF
