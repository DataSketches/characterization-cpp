clf;
k=1024;
d=load('../results/cpc_sketch_lgk10_timing.tsv');
# generate new column with time / min(n, k)
for i=1:size(d, 1)
  d(i,9) = d(i,6) / min(d(i,1), k);
end
# skip the first 9 measurements with high overhead to see more details in the interesting region
semilogx(d(10:end,1) / k, d(10:end,9), 'linewidth', 2);
set(gca, 'fontsize', 16);
title 'Deserialize time of CPC sketch lgK=10'
xlabel 'n / k'
ylabel 'time / min(n, k), nanoseconds'
grid minor on
