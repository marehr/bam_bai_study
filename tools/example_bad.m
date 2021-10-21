
% [QNAME, FLAG, RNAME, POS, MAPQ, CIGAR, RNEXT, PNEXT, TLEN, SEQ, QUAL]
[QNAME, POS, CIGAR] = textread("example_bad.sam", "%s %* %* %d %* %dM %* %* %* %* %*", 'Delimiter','\t', "commentstyle", "@")

Xbegin = POS'
Xlength= CIGAR'
label  = QNAME
Y = [0 : size(Xbegin, 2) - 1] * -1

max_x = max(Xbegin + Xlength)
max_y = min(Y)

figure

g_x = [0:2^14:max_x]
g_y = [0 max_y]
for i=1:length(g_x)
   plot([g_x(i) g_x(i)],[g_y(1) g_y(end)],'k:') %y grid lines
   hold on
end

h = plot([Xbegin;Xbegin+Xlength], [Y;Y], 'Marker', '+')
text (Xbegin + Xlength / 2, Y, label, 'horizontalalignment', 'center', 'verticalalignment', 'top');
waitfor(h)
