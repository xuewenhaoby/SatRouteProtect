cat nobackRoute |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3+30}' >gnuplotDelayv1.log
cat backRoute |awk -F '=' 'NR>1{print $2, $4}' |awk  '{print $1, $3+30}' >gnuplotDelayv2.log

gnuplot -persist <<EOF
set multiplot layout 3,1
set title "有无备份链路时延结果对比图" 
set xlabel'仿真时间(ms)'
set ylabel'到特定网段时延(ms)'
set xrange [0:100]
set yrange [0:50]
set key center at 83,8
plot 'gnuplotDelayv1.log' using 1:2 title '无备份链路时延' with points ls 7 pt 13 pointsize 1.5
unset title
plot 'gnuplotDelayv2.log' using 1:2 title '有备份链路时延' with points ls 6 pt 6 pointsize 1.5
unset multiplot
set output

EOF
