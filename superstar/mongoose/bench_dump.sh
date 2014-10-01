#!/bin/bash
gr="grep -hv URL latency_analysis/benchmark/copy*"

echo "Statistics:"
# See http://stackoverflow.com/questions/9789806/command-line-utility-to-print-statistics-of-numbers-in-linux
$gr | perl -e 'use List::Util qw(max min sum); 
	@a=();
	while(<>){$sqsum+=$_*$_; push(@a,$_)}; 
	$n=@a;
	$s=sum(@a);
	$a=$s/@a;
	$m=max(@a);
	$mm=min(@a);
	$std=sqrt($sqsum/$n-($s/$n)*($s/$n));
	$mid=int @a/2;
	@srtd=sort @a;
	if(@a%2){$med=($srtd[$mid])*1;}else{$med=($srtd[$mid-1]+$srtd[$mid])/2;};
	print "records:$n\nsum:$s\navg:$a\nstd:$std\nmed:$med\nmax:$m\min:$mm";'

echo
echo "Worst ten timings:"
$gr | sort | tail -10
