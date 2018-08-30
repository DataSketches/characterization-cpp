clf;
k=1024;
d=load('../results/cpc_sketch_lgk10_timing.tsv');
# skip the first 9 measurements with high overhead to see more details in the interesting region
semilogx(d(10:end,1) / k, d(10:end,4), 'linewidth', 2);
set(gca, 'fontsize', 16);
title 'Update time of CPC sketch lgK=10'
xlabel 'n / k'
ylabel 'update time, nanoseconds'
grid minor on
