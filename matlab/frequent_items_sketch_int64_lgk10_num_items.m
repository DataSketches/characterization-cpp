clf;
k=1024;
d1=load('../results/frequent_items_sketch_int64_lgk10_zipf07.tsv');
d2=load('../results/frequent_items_sketch_int64_lgk10_zipf1.tsv');
d3=load('../results/frequent_items_sketch_int64_lgk10_zipf11.tsv');
d4=load('../results/frequent_items_sketch_int64_lgk10_geom0005.tsv');
semilogx([d1(:,1) / k, d2(:,1) / k, d3(:,1) / k, d4(:,1) / k], [d1(:,10), d2(:,10), d3(:,10), d4(:,10)], 'linewidth', 2);
set(gca, 'fontsize', 16);
title 'Number of retiained items in frequent items sketch<long long>(10)'
xlabel 'n / k'
ylabel 'number of items'
legend('zipf 0.7', 'zipf 1.0', 'zipf 1.1', 'geom 0.005');
grid minor on
